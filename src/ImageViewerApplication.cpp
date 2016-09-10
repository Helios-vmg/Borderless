/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "plugin-core/PluginCoreState.h"
#include "MainWindow.h"
#include "RotateDialog.h"
#include "Misc.h"
#include "OptionsDialog.h"
#include "GenericException.h"
#include "ProtocolModule.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>
#include <QDir>
#include <QStandardPaths>

ImageViewerApplication::ImageViewerApplication(int &argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false),
		tray_icon(QIcon(":/icon16.png"), this){
	this->load_custom_file_protocols();
	if (!this->restore_settings())
		this->settings = std::make_shared<MainSettings>();
	this->reset_tray_menu();
	this->conditional_tray_show();
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
	ImageViewerApplication::new_instance(this->args);
	if (!this->windows.size() && !this->settings->get_keep_application_in_background())
		throw NoWindowsException();
	
	this->setup_slots();
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

template <typename ListingT, typename ListT>
std::shared_ptr<DirectoryIterator> generic_get_dir(ListT &list, const QString &path, bool local, CustomProtocolHandler &handler){
	for (auto &p : list){
		if (p.first->is_local() == local && *p.first == path){
			p.second++;
			return std::make_shared<DirectoryIterator>(*p.first);
		}
	}
	auto listing = std::make_shared<ListingT>(path, handler);
	if (!*listing)
		return std::shared_ptr<DirectoryIterator>();
	list.push_back(std::make_pair(listing, 1));
	return std::make_shared<DirectoryIterator>(*listing);
}

std::shared_ptr<DirectoryIterator> ImageViewerApplication::request_local_directory_iterator(const QString &path){
	auto clean = path;
	std::shared_ptr<DirectoryIterator> ret;
	if (!check_and_clean_path(clean))
		return ret;
	return generic_get_dir<LocalDirectoryListing>(this->listings, clean, true, *this->protocol_handler);
}

std::shared_ptr<DirectoryIterator> ImageViewerApplication::request_directory_iterator_by_url(const QString &url){
	if (!this->protocol_handler->is_url(url))
		return std::shared_ptr<DirectoryIterator>();
	return generic_get_dir<ProtocolDirectoryListing>(this->listings, url, false, *this->protocol_handler);
}

void ImageViewerApplication::release_directory(std::shared_ptr<DirectoryIterator> it){
	if (!it)
		return;
	bool found = false;
	size_t i = 0;
	for (auto &p : this->listings){
		if (p.first.get() == it->get_listing()){
			found = true;
			break;
		}
		i++;
	}
	if (!found)
		return;
	auto *pair = &this->listings[i];
	if (!--pair->second)
		this->listings.erase(this->listings.begin() + i);
}

void ImageViewerApplication::save_current_state(std::shared_ptr<ApplicationState> &state){
	state = std::make_shared<ApplicationState>();
	this->save_current_windows(state->get_windows());
}

void ImageViewerApplication::save_current_windows(std::vector<std::shared_ptr<WindowState>> &windows){
	for (auto &w : this->windows)
		windows.push_back(w.second->save_state());
}

class SettingsException : public GenericException{
public:
	SettingsException(const char *what) : GenericException(what){}
};

class DeserializationException : public std::exception {
protected:
	const char *message;
public:
	DeserializationException(DeserializerStream::ErrorType type){
		switch (type){
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
		case DeserializerStream::ErrorType::AllocateObjectOfUnknownType:
			this->message = "The stream contains an object of an unknown type. Did you try to import a configuration created by a newer version of the program?";
			break;
		default:
			this->message = "Unknown error.";
			break;
		}
	}
	virtual const char *what() const NOEXCEPT override {
		return this->message;
	}
};

class ImplementedDeserializerStream : public DeserializerStream {
protected:
	void report_error(ErrorType type, const char *) override {
		throw DeserializationException(type);
	}
public:
	ImplementedDeserializerStream(std::istream &stream) : DeserializerStream(stream){}
};

void ImageViewerApplication::save_settings(bool with_state){
	if (this->do_not_save)
		return;
	QString path = this->get_config_filename();
	if (path.isNull())
		return;
	Settings settings;
	settings.main = this->settings;
	if (with_state && this->settings->get_save_state_on_exit())
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
	if (caller){
		caller->build_context_menu(*ret, this->get_lua_submenu(caller));
		if (ret->actions().size() != initial)
			ret->addSeparator();
	}
	auto receiver = caller ? (QObject *)caller : (QObject *)this;
	ret->addAction("Options...", this, SLOT(show_options()), this->shortcuts.get_current_sequence(show_options_command));
	ret->addAction("Quit", receiver, SLOT(quit_slot()), this->shortcuts.get_current_sequence(quit_command));
	return ret;
}

void ImageViewerApplication::set_option_values(MainSettings &settings){
	*this->settings = settings;
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
}

void ImageViewerApplication::show_options(){
	this->tray_icon.setContextMenu(nullptr);
	OptionsDialog dialog(*this);
	dialog.exec();
	this->reset_tray_menu();
	this->conditional_tray_show();
}

void ImageViewerApplication::quit_and_discard_state(){
	this->save_settings(false);
	this->do_not_save = true;
	this->quit();
}

bool ImageViewerApplication::restore_settings(){
	QString path = this->get_config_filename();
	if (path.isNull())
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
		msgbox.setWindowTitle("Error reading configuration");
		msgbox.setText(ex.what());
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

QString ImageViewerApplication::get_config_location(){
	auto &ret = this->config_location;
	if (ret.isNull()){
		auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
		if (!list.size())
			return QString::null;
		ret = list[0];
		int index = ret.lastIndexOf('/');
		if (index < 0)
			index = ret.lastIndexOf('\\');
		ret = ret.mid(0, index);
		auto c = QDir::separator();
		ret += c;
		ret += "BorderlessImageViewer";
		ret += c;
		QDir dir(ret);
		if (!dir.mkpath(ret))
			return ret = QString::null;
	}
	return ret;
}

QString ImageViewerApplication::get_config_filename(){
	auto &ret = this->config_filename;
	if (ret.isNull()){
		ret = this->get_config_location();
		if (ret.isNull())
			return ret;
		ret += "settings.dat";
	}
	return ret;
}

QString ImageViewerApplication::get_user_filters_location(){
	auto &ret = this->user_filters_location;
	if (ret.isNull()){
		ret = this->get_config_location();
		if (ret.isNull())
			return ret;
		ret += "filters";
		ret += QDir::separator();
		QDir dir(ret);
		if (!dir.mkpath(ret))
			return ret = QString::null;
	}
	return ret;
}

QStringList ImageViewerApplication::get_user_filter_list(){
	QStringList ret;

	auto user_filters_location = this->get_user_filters_location();
	if (user_filters_location.isNull())
		return ret;

	QDir directory(user_filters_location);
	directory.setFilter(QDir::Files | QDir::Hidden);
	directory.setSorting(QDir::Name);
	QStringList filters;
	filters << "*.lua";
	for (auto i = accepted_cpp_extensions_size; i--;)
		filters << QString("*.") + accepted_cpp_extensions[i];
	directory.setNameFilters(filters);
	return directory.entryList();
}

QMenu &ImageViewerApplication::get_lua_submenu(MainWindow *){
	this->lua_submenu.clear();
	this->lua_submenu.setTitle("User filters");
	auto list = this->get_user_filter_list();
	if (list.isEmpty()){
		this->lua_submenu.addAction("(None)");
		this->lua_submenu.actions()[0]->setEnabled(false);
	}else
		for (auto &i : list){
			//this->lua_submenu.addAction("Quit", caller, SLOT(user_script_slot()));
			this->lua_submenu.addAction(i);
		}
	return this->lua_submenu;
}

void ImageViewerApplication::lua_script_activated(QAction *action){
	try{
		this->context_menu_last_requester->process_user_script(this->get_user_filters_location() + action->text());
	}catch (std::exception &e){
		QMessageBox msgbox;
		msgbox.setWindowTitle("Error");
		QString text = "Error executing user script \"";
		text += action->text();
		text += "\": ";
		text += e.what();
		msgbox.setText(text);
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.exec();
	}
}

PluginCoreState &ImageViewerApplication::get_plugin_core_state(){
	if (!this->plugin_core_state)
		this->plugin_core_state.reset(new PluginCoreState);
	return *this->plugin_core_state;
}

void ImageViewerApplication::setup_slots(){
	connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(resolution_change(int)));
	connect(this->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(work_area_change(int)));
	connect(&this->lua_submenu, SIGNAL(triggered(QAction *)), this, SLOT(lua_script_activated(QAction *)));
}

void ImageViewerApplication::reset_tray_menu(){
	this->last_tray_context_menu = this->build_context_menu();
	this->tray_icon.setContextMenu(this->last_tray_context_menu.get());
	this->tray_context_menu.swap(this->last_tray_context_menu);
}

void ImageViewerApplication::conditional_tray_show(){
	if (this->settings->get_keep_application_in_background())
		this->tray_icon.show();
	else
		this->tray_icon.hide();
}

void ImageViewerApplication::load_custom_file_protocols(){
	this->protocol_handler.reset(new CustomProtocolHandler(this->get_config_location()));
}

QImage ImageViewerApplication::load_image(const QString &path){
	auto dev = this->protocol_handler->open(path);
	if (!dev)
		return QImage(path);
	QImage ret;
	auto filename = this->protocol_handler->get_filename(path);
	auto extension = QFileInfo(filename).suffix().toStdString();
	ret.load(dev.get(), extension.c_str());
	return ret;
}

std::pair<std::unique_ptr<QMovie>, std::unique_ptr<QIODevice>> ImageViewerApplication::load_animation(const QString &path){
	auto dev = this->protocol_handler->open(path);
	std::unique_ptr<QMovie> mov;
	if (!dev)
		mov = std::make_unique<QMovie>(path);
	else
		mov = std::make_unique<QMovie>(dev.get());
	return std::make_pair(std::move(mov), std::move(dev));
}

bool ImageViewerApplication::is_animation(const QString &path){
	auto testString = this->protocol_handler->get_filename(path);
	if (testString.isNull())
		testString = path;
	return testString.endsWith(".gif", Qt::CaseInsensitive);
}

QString ImageViewerApplication::get_filename_from_url(const QString &url){
	return this->protocol_handler->get_filename(url);
}
