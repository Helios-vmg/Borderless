/*

Copyright (c) 2015, Helios
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef IMAGEVIEWERAPPLICATION_H
#define IMAGEVIEWERAPPLICATION_H

#include "SingleInstanceApplication.h"
#include "Settings.h"
#include "Shortcuts.h"
#include <QMenu>
#include <memory>
#include <exception>

class QAction;

class NoWindowsException : public std::exception{};

enum class ZoomMode{
	Normal = 0,
	Locked = 1,
	Automatic = 2,
	AutoFit = Automatic | 0,
	AutoFill = Automatic | 1,
};

struct OptionsPack{
	bool center_images,
		use_checkerboard,
		clamp_to_edges,
		keep_in_background;
	int clamp_strength;
	ZoomMode windowed_zoom_mode,
		fullscreen_zoom_mode;
	bool operator==(const OptionsPack &b) const{
		if (this->center_images != b.center_images)
			return false;
		if (this->use_checkerboard != b.use_checkerboard)
			return false;
		if (this->clamp_to_edges != b.clamp_to_edges)
			return false;
		if (this->clamp_strength != b.clamp_strength)
			return false;
		if (this->windowed_zoom_mode != b.windowed_zoom_mode)
			return false;
		if (this->fullscreen_zoom_mode != b.fullscreen_zoom_mode)
			return false;
		if (this->keep_in_background != b.keep_in_background)
			return false;
		return true;
	}
	bool operator!=(const OptionsPack &b) const{
		return !(*this == b);
	}
};

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

	Settings settings;
	DEFINE_SETTING(int, clamp_strength, 25);
	DEFINE_SETTING(bool, clamp_to_edges, true);
	DEFINE_SETTING(bool, use_checkerboard_pattern, true);
    DEFINE_SETTING(bool, center_when_displayed, true);
    DEFINE_SETTING(bool, keep_application_in_background, true);
	DEFINE_SETTING2(ZoomMode, zoom_mode_for_new_windows);
	DEFINE_SETTING2(ZoomMode, fullscreen_zoom_mode_for_new_windows);

	void save_current_state(SettingsTree &tree);
	void save_current_windows(SettingsTree &tree);
	void restore_current_state(const SettingsTree &tree);
	void restore_current_windows(const SettingsArray &tree);
	void propagate_shortcuts();

protected:
	void new_instance(const QStringList &args) override;
	void add_window(sharedp_t window);

public:
	ImageViewerApplication(int argc, char **argv, const QString &unique_name);
	~ImageViewerApplication();
	std::shared_ptr<DirectoryIterator> request_directory(const QString &path);
	void release_directory(std::shared_ptr<DirectoryIterator>);
	bool get_clamp_to_edges() const{
		return this->clamp_to_edges;
	}
	int get_clamp_strength() const{
		return this->clamp_strength;
	}
	bool get_use_checkerboard_pattern() const{
		return this->use_checkerboard_pattern;
	}
	void reset_settings();
	void save_settings(bool with_state = true);
	void restore_settings();
	void quit_and_discard_state();
	bool toggle_center_when_displayed(){
		this->center_when_displayed = !this->center_when_displayed;
		return this->center_when_displayed;
	}
	bool get_center_when_displayed() const{
		return this->center_when_displayed;
	}
	ZoomMode get_zoom_mode_for_new_windows() const{
		return this->zoom_mode_for_new_windows;
	}
	ZoomMode get_fullscreen_zoom_mode_for_new_windows() const{
		return this->fullscreen_zoom_mode_for_new_windows;
	}
	void minimize_all();
	const ApplicationShortcuts &get_shortcuts() const{
		return this->shortcuts;
	}
	void options_changed(const std::vector<ShortcutTriple> &new_shortcuts);
	std::shared_ptr<QMenu> build_context_menu(MainWindow *caller);
	OptionsPack get_option_values() const;
	void set_option_values(const OptionsPack &);

public slots:
	void window_closing(MainWindow *);
	void show_options();
	void resolution_change(int screen);
	void work_area_change(int screen);
};

#endif // IMAGEVIEWERAPPLICATION_H
