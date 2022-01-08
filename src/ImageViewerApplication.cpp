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
#include "GenericException.h"
#include "ProtocolModule.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <random>
#include <QJsonDocument>
#include <memory>

template <typename T>
class AutoSetter{
	T *dst;
	T old_value;
public:
	AutoSetter(): dst(nullptr){}
	AutoSetter(T &dst, T new_value): dst(&dst), old_value(dst){
		*this->dst = new_value;
	}
	AutoSetter(const AutoSetter &) = delete;
	AutoSetter &operator=(const AutoSetter &) = delete;
	AutoSetter(AutoSetter &&other){
		*this = std::move(other);
	}
	AutoSetter &operator=(AutoSetter &&other){
		this->dst = other.dst;
		other.dst = nullptr;
		this->old_value = std::move(other.old_value);
		return *this;
	}
	~AutoSetter(){
		if (this->dst)
			*this->dst = this->old_value;
	}
};

template <typename T>
AutoSetter<T> autoset(T &dst, T value){
	return AutoSetter<T>(dst, value);
}

ImageViewerApplication::ImageViewerApplication(int &argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false),
		tray_icon(QIcon(":/icon16.png"), this){
	QDir::setCurrent(this->applicationDirPath());
	this->load_custom_file_protocols();
	this->restore_settings_only();
	this->reset_tray_menu();
	this->conditional_tray_show();
	this->setQuitOnLastWindowClosed(!this->settings->get_keep_application_in_background());
	ImageViewerApplication::new_instance(this->args);
	if (!this->windows.size() && !this->settings->get_keep_application_in_background())
		throw NoWindowsException();
	
	this->setup_slots();
}

ImageViewerApplication::~ImageViewerApplication(){}

void ImageViewerApplication::new_instance(const QStringList &args){
	if (args.size() < 2 && !this->app_state)
		this->restore_state_only();
	else if (this->get_state_is_empty()){
		this->app_state = std::make_shared<ApplicationState>();
		this->state_is_empty = false;
	}
	
	auto p = std::make_shared<MainWindow>(*this, args);
	if (!p->is_null())
		this->add_window(p);
	this->save_settings();
}

void ImageViewerApplication::add_window(sharedp_t p){
	if (!p->is_loaded()){
		p->close();
		return;
	}
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

void ImageViewerApplication::save_current_state(ApplicationState &state){
	this->save_current_windows(state.get_windows());
}

void ImageViewerApplication::save_current_windows(std::vector<std::shared_ptr<WindowState>> &windows){
	windows.clear();
	windows.reserve(this->windows.size());
	for (auto &w : this->windows)
		windows.push_back(w.second->save_state());
}

class SettingsException : public GenericException{
public:
	SettingsException(const char *what) : GenericException(what){}
};

QByteArray hash_file(const QByteArray &contents){
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(contents);
	return hash.result();
}

void ImageViewerApplication::conditionally_save_file(const QByteArray &contents, const QString &path, QByteArray &last_digest){
	auto new_digest = hash_file(contents);
	if (!last_digest.isNull() && new_digest == last_digest)
		return;

	QFile file(path);
	file.open(QFile::WriteOnly | QFile::Truncate);
	if (!file.isOpen())
		return;
	file.write(contents);

	last_digest = new_digest;
}

void ImageViewerApplication::save_settings_only(){
	if (this->restoring_settings)
		return;
	QString path = this->get_settings_filename();
	if (path.isNull())
		return;
	Settings settings;
	settings.main = this->settings;
	settings.shortcuts = this->shortcuts.save_settings();

	QJsonDocument doc;
	doc.setObject(settings.serialize().toObject());
	this->conditionally_save_file(doc.toJson(QJsonDocument::Indented), path, this->last_saved_settings_digest);
}

void ImageViewerApplication::save_state_only(){
	if (this->restoring_state)
		return;
	if (!this->app_state)
		return;
	auto path = this->get_state_filename();
	if (path.isNull())
		return;
	this->save_current_state(*this->app_state);
	QJsonDocument doc;
	StateFile sf;
	sf.state = this->app_state;
	doc.setObject(sf.serialize().toObject());
	this->conditionally_save_file(doc.toJson(QJsonDocument::Indented), path, this->last_saved_state_digest);
}

void ImageViewerApplication::save_settings(bool with_state){
	if (this->do_not_save)
		return;
	this->save_settings_only();
	if (with_state && this->settings->get_save_state_on_exit())
		this->save_state_only();
}

void ImageViewerApplication::restore_current_state(const ApplicationState &windows_state){
	this->restore_current_windows(windows_state.get_windows());
}

void ImageViewerApplication::restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &window_states){
	for (auto &state : window_states)
		this->add_window(std::make_shared<MainWindow>(*this, state));
}

std::shared_ptr<QMenu> ImageViewerApplication::build_context_menu(MainWindow *caller){
	this->context_menu_last_requester = caller;
	std::shared_ptr<QMenu> ret(new QMenu);
	auto initial = ret->actions().size();
	if (caller){
		caller->build_context_menu(*ret);
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
	this->about_to_quit();
	this->quit();
}

QJsonDocument ImageViewerApplication::load_json(const QString &path, QByteArray &digest){
	if (path.isNull())
		return {};
	QFile file(path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		return {};
	auto contents = file.readAll();
	digest = hash_file(contents);
	return QJsonDocument::fromJson(contents);
}

void ImageViewerApplication::restore_settings_only(){
	auto as = autoset(this->restoring_settings, true);
	auto json = this->load_json(this->get_settings_filename(), this->last_saved_settings_digest);
	if (json.isNull()){
		this->settings = std::make_shared<MainSettings>();
		return;
	}
	
	Settings settings(json.object());
	this->settings = settings.main;
	if (settings.shortcuts)
		this->shortcuts.restore_settings(*settings.shortcuts);
}

void ImageViewerApplication::restore_state_only(){
	auto as = autoset(this->restoring_state, true);
	auto json = this->load_json(this->get_state_filename(), this->last_saved_state_digest);
	if (json.isNull()){
		this->app_state = std::make_shared<ApplicationState>();
		return;
	}
	
	StateFile state(json.object());
	this->app_state = std::move(state.state);
	this->restore_current_state(*this->app_state);
}

void ImageViewerApplication::resolution_change(QScreen &screen, const QRect &resolution){
	for (auto &i : this->windows)
		i.second->resolution_change(screen, resolution);
}

void ImageViewerApplication::work_area_change(QScreen &screen, const QRect &resolution){
	for (auto &i : this->windows)
		i.second->work_area_change(screen, resolution);
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
		return {};
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
		return {};
	return ret;
}

QString ImageViewerApplication::get_config_location(){
	if (this->config_location.isNull())
		this->config_location = ::get_config_location();
	return this->config_location;
}

QString ImageViewerApplication::get_config_subpath(QString &dst, const char *sub){
	auto &ret = dst;
	if (ret.isNull()){
		ret = this->get_config_location();
		if (ret.isNull())
			return ret;
		ret += sub;
	}
	return ret;
}

QString ImageViewerApplication::get_settings_filename(){
	return this->get_config_subpath(this->config_filename, "settings.json");
}

QString ImageViewerApplication::get_state_filename(){
	return this->get_config_subpath(this->state_filename, "state.json");
}

void ImageViewerApplication::setup_slots(){
	connect(this, SIGNAL(screenAdded(QScreen *)), this, SLOT(screen_added(QScreen *)));
	connect(this, SIGNAL(screenRemoved(QScreen *)), this, SLOT(screen_removed(QScreen *)));
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
	auto t0 = clock();
	ret.load(dev.get(), extension.c_str());
	if (ret.isNull()){
		//Workaround. QImage refuses to read certain files correctly.
		auto n = dev->size();
		std::unique_ptr<uchar[]> temp(new uchar[n]);
		dev->seek(0);
		dev->read((char *)temp.get(), n);
		ret.loadFromData(temp.get(), n, extension.c_str());
	}
	auto t1 = clock();
	qDebug() << "Load " << path << " took " << (t1 - t0) / (double)CLOCKS_PER_SEC;
	return ret;
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

QString get_per_user_unique_id(){
	auto location = get_config_location(false);
	if (location.isNull())
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

void ImageViewerApplication::turn_transparent(MainWindow &window, bool yes){
	auto state = window.save_state();
	std::shared_ptr<MainWindow> new_window;
	if (yes)
		new_window = std::make_shared<TransparentMainWindow>(*this, state);
	else
		new_window = std::make_shared<MainWindow>(*this, state);
	this->add_window(new_window);
}

bool ImageViewerApplication::get_state_is_empty(){
	if (this->state_is_empty)
		return *this->state_is_empty;
	auto as = autoset(this->restoring_state, true);
	auto json = this->load_json(this->get_state_filename(), this->last_saved_state_digest);
	if (json.isNull()){
		this->state_is_empty = true;
		return true;
	}
	StateFile state(json.object());
	if (!state.state->get_windows().size()){
		this->state_is_empty = true;
		return true;
	}
	this->state_is_empty = false;
	return false;
}

void ImageViewerApplication::screen_added(QScreen *screen){
	auto s = screen->serialNumber();
	auto cb = std::make_unique<ResolutionChangeCallback>(*this, *screen);
	connect(screen, SIGNAL(geometryChanged(const QRect &)), cb.get(), SLOT(resolution_change(const QRect &)));
	connect(screen, SIGNAL(availableGeometryChanged(const QRect &)), cb.get(), SLOT(work_area_change(const QRect &)));
	this->rccbs[s] = std::move(cb);
}

void ImageViewerApplication::screen_removed(QScreen *screen){
	this->rccbs.erase(screen->serialNumber());
}

ResolutionChangeCallback::ResolutionChangeCallback(ImageViewerApplication &app, QScreen &screen)
	: app(&app)
	, screen(&screen){}

void ResolutionChangeCallback::resolution_change(const QRect &geometry){
	this->app->resolution_change(*this->screen, geometry);
}

void ResolutionChangeCallback::work_area_change(const QRect &geometry){
	this->app->work_area_change(*this->screen, geometry);
}

std::string unique_identifier(QScreen &screen){
	auto s = screen.manufacturer() + "/" + screen.model() + "/" + screen.name() + "/" + screen.serialNumber();
	return s.toStdString();
}

void ImageViewerApplication::about_to_quit(){
	this->save_settings();
	this->windows.clear();
}
