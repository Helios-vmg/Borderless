/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWERAPPLICATION_H
#define IMAGEVIEWERAPPLICATION_H

#include "SingleInstanceApplication.h"
#include "serialization/settings.generated.h"
#include "Shortcuts.h"
#include "Enums.h"
#include <QMenu>
#include <memory>
#include <exception>

class QAction;
struct lua_State;

class NoWindowsException : public std::exception{};

class ImageViewerApplication : public SingleInstanceApplication
{
	Q_OBJECT

	typedef std::shared_ptr<MainWindow> sharedp_t;
	std::map<uintptr_t, sharedp_t> windows;
	std::vector<std::pair<DirectoryListing *, unsigned> > listings;
	bool do_not_save;
	std::vector<std::shared_ptr<QAction> > actions;
	ApplicationShortcuts shortcuts;
	MainWindow *context_menu_last_requester;
	QString config_location,
		config_filename,
		user_filters_location;

	std::shared_ptr<MainSettings> settings;

	QMenu lua_submenu;

	void save_current_state(std::shared_ptr<ApplicationState> &);
	void save_current_windows(std::vector<std::shared_ptr<WindowState>> &);
	void restore_current_state(const ApplicationState &);
	void restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &);
	void propagate_shortcuts();
	QMenu &get_lua_submenu(MainWindow *caller);
	QString get_config_location();
	QString get_config_filename();
	QString get_user_filters_location();
	QStringList get_user_filter_list();

protected:
	void new_instance(const QStringList &args) override;
	void add_window(sharedp_t window);

public:
	ImageViewerApplication(int argc, char **argv, const QString &unique_name);
	~ImageViewerApplication();
	std::shared_ptr<DirectoryIterator> request_directory(const QString &path);
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
	bool restore_settings();
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
	std::shared_ptr<QMenu> build_context_menu(MainWindow *caller);
	std::shared_ptr<MainSettings> get_option_values() const{
		return this->settings;
	}
	void set_option_values();

public slots:
	void window_closing(MainWindow *);
	void show_options();
	void resolution_change(int screen);
	void work_area_change(int screen);
	void lua_script_activated(QAction *action);
};

#endif // IMAGEVIEWERAPPLICATION_H
