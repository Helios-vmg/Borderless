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

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include "RotateDialog.h"
#include "Misc.h"
#include "OptionsDialog.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>

const ZoomMode ImageViewerApplication::default_zoom_mode_for_new_windows = ZoomMode::Normal;
const ZoomMode ImageViewerApplication::default_fullscreen_zoom_mode_for_new_windows = ZoomMode::AutoFit;

ImageViewerApplication::ImageViewerApplication(int argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		do_not_save(false){
	this->reset_settings();
	this->restore_settings();
	this->setQuitOnLastWindowClosed(!this->keep_application_in_background);
	this->new_instance(this->args);
	if (!this->windows.size() && !this->keep_application_in_background)
		throw NoWindowsException();

	connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(resolution_change(int)));
	connect(this->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(work_area_change(int)));
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

void ImageViewerApplication::reset_settings(){
	RESET_SETTING(use_checkerboard_pattern);
	RESET_SETTING(clamp_strength);
	RESET_SETTING(clamp_to_edges);
	RESET_SETTING(center_when_displayed);
	RESET_SETTING(keep_application_in_background);
	RESET_SETTING(zoom_mode_for_new_windows);
	RESET_SETTING(fullscreen_zoom_mode_for_new_windows);
	this->shortcuts.reset_settings();
}

DEFINE_SETTING_STRING(clamp_strength);
DEFINE_SETTING_STRING(clamp_to_edges);
DEFINE_SETTING_STRING(use_checkerboard_pattern);
DEFINE_SETTING_STRING(last_state);
DEFINE_SETTING_STRING(windows);
DEFINE_SETTING_STRING(window);
DEFINE_SETTING_STRING(center_when_displayed);
DEFINE_SETTING_STRING(zoom_mode_for_new_windows);
DEFINE_SETTING_STRING(fullscreen_zoom_mode_for_new_windows);
DEFINE_SETTING_STRING(shortcuts);
DEFINE_SETTING_STRING(keep_application_in_background);

#define SAVE_SETTING(x) tree.set_value(#x, this->x)

void ImageViewerApplication::save_current_state(SettingsTree &tree){
	auto last_state = tree.create_tree();
	this->save_current_windows(*last_state);
	tree.add_tree(last_state_setting, last_state);
}

void ImageViewerApplication::save_current_windows(SettingsTree &tree){
	auto windows = tree.create_array();
	int i = 0;
	for (auto &w : this->windows)
		windows->push_back(w.second->save_state());
	tree.add_tree(windows_setting, windows);
}

void ImageViewerApplication::save_settings(bool with_state){
	if (this->do_not_save)
		return;
	SettingsTree tree;
	SAVE_SETTING(clamp_strength);
	SAVE_SETTING(clamp_to_edges);
	SAVE_SETTING(use_checkerboard_pattern);
	SAVE_SETTING(center_when_displayed);
	SAVE_SETTING(zoom_mode_for_new_windows);
	SAVE_SETTING(fullscreen_zoom_mode_for_new_windows);
	SAVE_SETTING(keep_application_in_background);
	if (with_state)
		this->save_current_state(tree);
	tree.add_tree(shortcuts_setting, this->shortcuts.save_settings());
	this->settings.write(tree);
	this->do_not_save = true;
}

void ImageViewerApplication::restore_current_state(const SettingsTree &tree){
	auto subtree = tree.get_array(windows_setting);
	if (subtree)
		this->restore_current_windows(*subtree);
}

void ImageViewerApplication::restore_current_windows(const SettingsArray &tree){
	this->windows.clear();
	tree.iterate([this](int, const SettingsItem &item){
		if (!item.is_value() && !item.is_array())
			this->add_window(sharedp_t(new MainWindow(*this, static_cast<const SettingsTree &>(item))));
	});
}

std::shared_ptr<QMenu> ImageViewerApplication::build_context_menu(MainWindow *caller){
	this->context_menu_last_requester = caller;
	std::shared_ptr<QMenu> ret(new QMenu);
	auto initial = ret->actions().size();
	caller->build_context_menu(*ret);
	if (ret->actions().size() != initial)
		ret->addSeparator();
	ret->addAction("Options...", this, SLOT(show_options()), this->shortcuts.get_current_sequence(show_options_command));
	ret->addAction("Quit", this, SLOT(quit_slot()), this->shortcuts.get_current_sequence(quit_command));
	return ret;
}

OptionsPack ImageViewerApplication::get_option_values() const{
	OptionsPack ret;
	ret.center_images = this->center_when_displayed;
	ret.use_checkerboard = this->use_checkerboard_pattern;
	ret.clamp_to_edges = this->clamp_to_edges;
	ret.keep_in_background = this->keep_application_in_background;
	ret.clamp_strength = this->clamp_strength;
	ret.windowed_zoom_mode = this->zoom_mode_for_new_windows;
	ret.fullscreen_zoom_mode = this->fullscreen_zoom_mode_for_new_windows;
	return ret;
}

void ImageViewerApplication::set_option_values(const OptionsPack &options){
	this->center_when_displayed = options.center_images;
	this->use_checkerboard_pattern = options.use_checkerboard;
	this->clamp_to_edges = options.clamp_to_edges;
	this->keep_application_in_background = options.keep_in_background;
	this->clamp_strength = options.clamp_strength;
	this->zoom_mode_for_new_windows = options.windowed_zoom_mode;
	this->fullscreen_zoom_mode_for_new_windows = options.fullscreen_zoom_mode;

	this->setQuitOnLastWindowClosed(!this->keep_application_in_background);
}

void ImageViewerApplication::show_options(){
	OptionsDialog dialog(*this);
	dialog.exec();
}

void ImageViewerApplication::quit_and_discard_state(){
	this->save_settings(false);
	this->quit();
}

#define RESTORE_SETTING(key) tree->get_value(this->key, key##_setting)

void ImageViewerApplication::restore_settings(){
	auto tree = this->settings.read();
	if (!tree)
		return;
	RESTORE_SETTING(clamp_strength);
	RESTORE_SETTING(clamp_to_edges);
	RESTORE_SETTING(use_checkerboard_pattern);
	RESTORE_SETTING(keep_application_in_background);
	RESTORE_SETTING(center_when_displayed);
	RESTORE_SETTING(zoom_mode_for_new_windows);
	RESTORE_SETTING(fullscreen_zoom_mode_for_new_windows);

	std::shared_ptr<SettingsTree> subtree;

	subtree = tree->get_tree(shortcuts_setting);
	if (subtree)
		this->shortcuts.restore_settings(*subtree);

	subtree = tree->get_tree(last_state_setting);
	if (subtree)
		this->restore_current_state(*subtree);

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