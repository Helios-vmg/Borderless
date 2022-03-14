/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWERAPPLICATION_H
#define IMAGEVIEWERAPPLICATION_H

#include "SingleInstanceApplication.h"
#include "Settings.h"
#include "Shortcuts.h"
#include "Streams.h"
#include "Enums.h"
#include <QMenu>
#include <memory>
#include <exception>
#include <QSystemTrayIcon>
#include <QWindow>
#include <optional>

class QAction;
class CustomProtocolHandler;
struct lua_State;
class ImageViewerApplication;

class NoWindowsException : public std::exception{};

class ResolutionChangeCallback : public QObject{
	Q_OBJECT

	ImageViewerApplication *app;
	QScreen *screen;
public:
	ResolutionChangeCallback(ImageViewerApplication &, QScreen &);
public slots:
	void resolution_change(const QRect &geometry);
	void work_area_change(const QRect &geometry);
};

class ImageViewerApplication : public SingleInstanceApplication{
	Q_OBJECT

	typedef std::shared_ptr<MainWindow> sharedp_t;
	std::map<uintptr_t, sharedp_t> windows;
	std::vector<std::pair<std::shared_ptr<DirectoryListing>, unsigned>> listings;
	bool do_not_save;
	std::vector<std::shared_ptr<QAction> > actions;
	ApplicationShortcuts shortcuts;
	MainWindow *context_menu_last_requester;
	QString config_location,
		config_filename,
		state_filename;
	bool restoring_settings = false;
	bool restoring_state = false;
	std::optional<bool> state_is_empty;

	std::shared_ptr<MainSettings> settings;
	std::shared_ptr<ApplicationState> app_state;

	QSystemTrayIcon tray_icon;
	std::shared_ptr<QMenu> tray_context_menu,
		last_tray_context_menu;
	QByteArray last_saved_settings_digest;
	QByteArray last_saved_state_digest;
	std::map<QString, std::unique_ptr<ResolutionChangeCallback>> rccbs;

	void save_current_state(ApplicationState &);
	void save_current_windows(std::vector<std::shared_ptr<WindowState>> &);
	void restore_current_state(const ApplicationState &);
	void restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &);
	void propagate_shortcuts();
	QString get_config_location();
	QString get_config_subpath(QString &dst, const char *sub);
	QString get_settings_filename();
	QString get_state_filename();
	void setup_slots();
	void reset_tray_menu();
	void conditional_tray_show();
	std::unique_ptr<CustomProtocolHandler> protocol_handler;
	bool get_state_is_empty();

protected:
	void new_instance(const QStringList &args) override;
	void add_window(sharedp_t window);
	static QJsonDocument load_json(const QString &, QByteArray &digest);
	void restore_settings_only();
	void restore_state_only();
	void save_settings_only();
	void save_state_only();
	static void conditionally_save_file(const QByteArray &contents, const QString &path, QByteArray &last_digest);

public:
	ImageViewerApplication(int &argc, char **argv, const QString &unique_name);
	~ImageViewerApplication();
	std::shared_ptr<DirectoryIterator> request_local_directory_iterator(const QString &path);
	std::shared_ptr<DirectoryIterator> request_directory_iterator_by_url(const QString &url);
	void release_directory(std::shared_ptr<DirectoryIterator>);
	bool get_clamp_to_edges() const{
		return this->settings->get_clamp_to_edges();
	}
	int get_clamp_strength() const{
		return this->settings->get_clamp_strength();
	}
	bool get_use_checkerboard_pattern() const{
		return this->settings->get_use_checkerboard_pattern();
	}
	void save_settings(bool with_state = true);
	void quit_and_discard_state();
	bool toggle_center_when_displayed(){
		this->settings->set_center_when_displayed(!this->settings->get_center_when_displayed());
		return this->settings->get_center_when_displayed();
	}
	bool get_center_when_displayed() const{
		return this->settings->get_center_when_displayed();
	}
	ZoomMode get_zoom_mode_for_new_windows() const{
		return this->settings->get_zoom_mode_for_new_windows();
	}
	ZoomMode get_fullscreen_zoom_mode_for_new_windows() const{
		return this->settings->get_fullscreen_zoom_mode_for_new_windows();
	}
	void minimize_all();
	const ApplicationShortcuts &get_shortcuts() const{
		return this->shortcuts;
	}
	void options_changed(const std::vector<ShortcutTriple> &new_shortcuts);
	std::shared_ptr<QMenu> build_context_menu(MainWindow *caller = nullptr);
	std::shared_ptr<MainSettings> get_option_values() const{
		return this->settings;
	}
	void set_option_values(MainSettings &settings);
	void load_custom_file_protocols();
	QImage load_image(const QString &);
	std::pair<std::unique_ptr<QMovie>, std::unique_ptr<QIODevice>> load_animation(const QString &);
	bool is_animation(const QString &);
	QString get_filename_from_url(const QString &);
	QString get_unique_filename_from_url(const QString &);
	void turn_transparent(MainWindow &window, bool yes);
	void about_to_quit();

public slots:
	void window_closing(MainWindow *);
	void show_options();
	void screen_added(QScreen *);
	void screen_removed(QScreen *);
	void resolution_change(QScreen &, const QRect &);
	void work_area_change(QScreen &, const QRect &);
	void quit_slot(){
		this->about_to_quit();
		this->quit();
	}
};

QString get_per_user_unique_id();
std::string unique_identifier(QScreen &);

#endif // IMAGEVIEWERAPPLICATION_H
