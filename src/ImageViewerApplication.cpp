/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include "RotateDialog.h"
#include "Misc.h"
#include "OptionsDialog.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>
#include <QDir>
#include <QStandardPaths>
#include <boost/iostreams/stream.hpp>
#include "plugin-core/lua.h"

class QFileInputStream {
	QFile *file;
public:
	typedef char char_type;
	typedef boost::iostreams::source_tag category;
	QFileInputStream(QFile *file) : file(file) {}
	std::streamsize read(char *s, std::streamsize n){
		std::streamsize ret = 0;
		bool bad = false;
		while (n) {
			auto count = this->file->read(s, n);
			ret += count;
			s += count;
			n -= count;
			if (this->file->error()!= QFileDevice::NoError || this->file->atEnd()) {
				bad = true;
				break;
			}
		}
		return !ret && bad ? -1 : ret;
	}
};

class QFileOutputStream {
	QFile *file;
public:
	typedef char char_type;
	typedef boost::iostreams::sink_tag category;
	QFileOutputStream(QFile *file) : file(file) {}
	std::streamsize write(const char *s, std::streamsize n){
		return this->file->write(s, n);
	}
};

ImageViewerApplication::ImageViewerApplication(int argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false){
	if (!this->restore_settings())
		this->settings = std::make_shared<MainSettings>();
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
	this->new_instance(this->args);
	if (!this->windows.size() && !this->settings->get_keep_application_in_background())
		throw NoWindowsException();

	connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(resolution_change(int)));
	connect(this->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(work_area_change(int)));

	this->lua_state.reset(init_lua_state(), [](lua_State *state){ lua_close(state); });
}

ImageViewerApplication::~ImageViewerApplication(){
	this->save_settings();
	this->windows.clear();
}

void ImageViewerApplication::new_instance(const QStringList &args){
	sharedp_t p(new MainWindow(*this, args));
	if (!p->is_null())
		this->add_window(p);
}

void ImageViewerApplication::add_window(sharedp_t p){
	connect(p.get(), SIGNAL(closing(MainWindow *)), this, SLOT(window_closing(MainWindow *)));
	p->show();
	p->raise();
	p->activateWindow();
	p->setFocus();
	this->windows[(uintptr_t)p.get()] = p;
}

void ImageViewerApplication::window_closing(MainWindow *window){
	auto it = this->windows.find((uintptr_t)window);
	if (it == this->windows.end())
		return;
	this->windows.erase(it);
}

std::shared_ptr<DirectoryIterator> ImageViewerApplication::request_directory(const QString &path){
	auto clean = path;
	std::shared_ptr<DirectoryIterator> ret;
	if (!check_and_clean_path(clean))
		return ret;
	for (auto &p : this->listings){
		if (*p.first == clean){
			ret.reset(new DirectoryIterator(*p.first));
			p.second++;
			return ret;
		}
	}
	auto list = new DirectoryListing(clean);
	if (!*list){
		delete list;
		return ret;
	}
	this->listings.push_back(std::make_pair(list, 1));
	ret.reset(new DirectoryIterator(*list));
	return ret;
}

void ImageViewerApplication::release_directory(std::shared_ptr<DirectoryIterator> it){
	if (!it)
		return;
	bool found = false;
	size_t i = 0;
	for (auto &p : this->listings){
		if (p.first == it->get_listing()){
			found = true;
			break;
		}
		i++;
	}
	if (!found)
		return;
	auto *pair = &this->listings[i];
	if (!--pair->second){
		delete pair->first;
		this->listings.erase(this->listings.begin() + i);
	}
}

void ImageViewerApplication::save_current_state(std::shared_ptr<ApplicationState> &state){
	state = std::make_shared<ApplicationState>();
	this->save_current_windows(state->get_windows());
}

void ImageViewerApplication::save_current_windows(std::vector<std::shared_ptr<WindowState>> &windows){
	for (auto &w : this->windows)
		windows.push_back(w.second->save_state());
}

class SettingsException : public std::exception {
public:
	SettingsException(const char *what) : std::exception(what) {}
};

class DeserializationException : public std::exception {
protected:
	const char *message;
public:
	DeserializationException(DeserializerStream::ErrorType type) {
		switch (type) {
		case DeserializerStream::ErrorType::UnexpectedEndOfFile:
			this->message = "Unexpected end of file.";
			break;
		case DeserializerStream::ErrorType::InconsistentSmartPointers:
			this->message = "Serialized stream uses smart pointers inconsistently.";
			break;
		case DeserializerStream::ErrorType::UnknownObjectId:
			this->message = "Serialized stream contains a reference to an unknown object.";
			break;
		case DeserializerStream::ErrorType::InvalidProgramState:
			this->message = "The program is in an unknown state.";
			break;
		case DeserializerStream::ErrorType::MainObjectNotSerializable:
			this->message = "The root object is not an instance of a Serializable subclass.";
			break;
		case DeserializerStream::ErrorType::AllocateAbstractObject:
			this->message = "The stream contains a concrete object with an abstract class type ID.";
			break;
		default:
			this->message = "Unknown.";
			break;
		}
	}
	virtual const char *what() const override {
		return this->message;
	}
};

class ImplementedDeserializerStream : public DeserializerStream {
protected:
	void report_error(ErrorType type, const char *info) override {
		throw DeserializationException(type);
	}
public:
	ImplementedDeserializerStream(std::istream &stream) : DeserializerStream(stream) {}
};

bool get_config_location(QString &dst) {
	auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	if (!list.size())
		return false;
	dst = list[0];
	int index = dst.lastIndexOf('/');
	if (index < 0)
		index = dst.lastIndexOf('\\');
	dst = dst.mid(0, index);
	auto c = QDir::separator();
	dst += c;
	dst += "BorderlessImageViewer";
	dst += c;
	return true;
}

bool get_config_filename(QString &path) {
	if (!get_config_location(path))
		//throw SettingsException("Can't determine settings path.");
		return false;
	QDir dir(path);
	if (!dir.mkpath(path))
		//throw SettingsException("Can't create settings path.");
		return false;
	path += "settings.dat";
	return true;
}

void ImageViewerApplication::save_settings(bool with_state){
	if (this->do_not_save)
		return;
	QString path;
	if (!get_config_filename(path))
		return;
	Settings settings;
	settings.main = this->settings;
	if (with_state)
		this->save_current_state(settings.state);
	settings.shortcuts = this->shortcuts.save_settings();

	QFile file(path);
	file.open(QFile::WriteOnly | QFile::Truncate);
	if (!file.isOpen())
		return;
	boost::iostreams::stream<QFileOutputStream> stream(&file);
	SerializerStream ss(stream);
	ss.serialize(settings, true);
}

void ImageViewerApplication::restore_current_state(const ApplicationState &windows_state){
	this->restore_current_windows(windows_state.get_windows());
}

void ImageViewerApplication::restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &window_states){
	this->windows.clear();
	for (auto &state : window_states)
		this->add_window(std::make_shared<MainWindow>(*this, state));
}

std::shared_ptr<QMenu> ImageViewerApplication::build_context_menu(MainWindow *caller){
	this->context_menu_last_requester = caller;
	std::shared_ptr<QMenu> ret(new QMenu);
	auto initial = ret->actions().size();
	caller->build_context_menu(*ret, this->get_lua_submenu());
	if (ret->actions().size() != initial)
		ret->addSeparator();
	ret->addAction("Options...", this, SLOT(show_options()), this->shortcuts.get_current_sequence(show_options_command));
	ret->addAction("Quit", this, SLOT(quit_slot()), this->shortcuts.get_current_sequence(quit_command));
	return ret;
}

void ImageViewerApplication::set_option_values(){
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
}

void ImageViewerApplication::show_options(){
	OptionsDialog dialog(*this);
	dialog.exec();
}

void ImageViewerApplication::quit_and_discard_state(){
	this->save_settings(false);
	this->do_not_save = true;
	this->quit();
}

bool ImageViewerApplication::restore_settings(){
	QString path;
	if (!get_config_filename(path))
		return false;
	QFile file(path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		return false;
	boost::iostreams::stream<QFileInputStream> stream(&file);
	ImplementedDeserializerStream ds(stream);
	std::shared_ptr<Settings> settings;
	try{
		settings.reset(ds.deserialize<Settings>(true));
	}catch (std::bad_cast &){
		return false;
	}catch (DeserializationException &ex){
		QMessageBox msgbox;
		msgbox.setText("Error reading configuration: " + QString(ex.what()));
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.exec();
		return false;
	}
	this->settings = settings->main;

	if (settings->shortcuts)
		this->shortcuts.restore_settings(*settings->shortcuts);

	if (settings->state)
		this->restore_current_state(*settings->state);

	return true;
}

void ImageViewerApplication::resolution_change(int screen){
	for (auto &i : this->windows)
		i.second->resolution_change(screen);
}

void ImageViewerApplication::work_area_change(int screen){
	for (auto &i : this->windows)
		i.second->work_area_change(screen);
}

void ImageViewerApplication::minimize_all(){
	for (auto &p : this->windows)
		p.second->minimize_slot();
}

void ImageViewerApplication::options_changed(const std::vector<ShortcutTriple> &new_shortcuts){
	this->shortcuts.update(new_shortcuts);
	this->propagate_shortcuts();
}

void ImageViewerApplication::propagate_shortcuts(){
	for (auto &w : this->windows)
		w.second->setup_shortcuts();
}

QMenu &ImageViewerApplication::get_lua_submenu(){
	this->lua_submenu.clear();
	this->lua_submenu.addAction("(None)");
	this->lua_submenu.actions()[0]->setEnabled(false);
	this->lua_submenu.setTitle("User filters");
	return this->lua_submenu;
}
