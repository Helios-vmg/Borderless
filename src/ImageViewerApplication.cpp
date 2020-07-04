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
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <random>
#include <QJsonDocument>

ImageViewerApplication::ImageViewerApplication(int &argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false),
		tray_icon(QIcon(":/icon16.png"), this){
	QDir::setCurrent(this->applicationDirPath());
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
	this->save_settings();
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

class SettingsException : public GenericException{
public:
	SettingsException(const char *what) : GenericException(what){}
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

	QJsonDocument doc;
	doc.setObject(settings.serialize().toObject());
	auto contents = doc.toJson(QJsonDocument::Indented);
	
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(contents);
	auto digest = hash.result();
	if (!this->last_saved_settings_digest.isNull() && digest == this->last_saved_settings_digest)
		return;

	QFile file(path);
	file.open(QFile::WriteOnly | QFile::Truncate);
	if (!file.isOpen())
		return;
	file.write(contents);

	this->last_saved_settings_digest = digest;
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
	auto doc = QJsonDocument::fromJson(file.readAll());

	Settings settings;
	if (!doc.isNull())
		settings = Settings(doc.object());

	this->settings = settings.main;

	if (settings.shortcuts)
		this->shortcuts.restore_settings(*settings.shortcuts);

	if (settings.state)
		this->restore_current_state(*settings.state);

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

QString get_config_location(bool strip_last_item = true){
	QString ret;
	auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	if (!list.size())
		return QString::null;
	ret = list[0];
	if (strip_last_item){
		int index = ret.lastIndexOf('/');
		if (index < 0)
			index = ret.lastIndexOf('\\');
		ret = ret.mid(0, index);
	}
	auto c = QDir::separator();
	ret += c;
	ret += "BorderlessImageViewer";
	ret += c;
	QDir dir(ret);
	if (!dir.mkpath(ret))
		return QString::null;
	return ret;
}

QString ImageViewerApplication::get_config_location(){
	if (this->config_location.isNull())
		this->config_location = ::get_config_location();
	return this->config_location;
}

QString ImageViewerApplication::get_config_filename(){
	auto &ret = this->config_filename;
	if (ret.isNull()){
		ret = this->get_config_location();
		if (ret.isNull())
			return ret;
		ret += "settings.json";
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

QString generate_random_string(){
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> dist1('a', 'z');
	std::uniform_int_distribution<int> dist2(0, 1);
	QString ret;
	for (int i = 16; i--;){
		int c = dist1(rng);
		if (dist2(rng))
			c = toupper(c);
		ret += (char)c;
	}
	return ret;
}

QString get_per_user_unique_id(){
	auto location = get_config_location(false);
	if (location == QString::null)
		return "";
	location += "user_id.txt";
	QFile file(location);
	if (!file.exists()){
		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
			return "";
		auto ret = generate_random_string();
		auto bytes = ret.toUtf8();
		file.write(bytes.data(), bytes.size());
		return ret;
	}
	if (!file.open(QIODevice::ReadOnly))
		return "";
	auto bytes = file.readAll();
	if (bytes.isNull())
		return "";
	return QString::fromUtf8(bytes);
}
