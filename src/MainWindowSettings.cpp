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

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDir>

DEFINE_SETTING_STRING(using_checkerboard_pattern);
DEFINE_SETTING_STRING(current_directory);
DEFINE_SETTING_STRING(current_filename);
DEFINE_SETTING_STRING(movement_size);
DEFINE_SETTING_STRING(zoom_mode);
DEFINE_SETTING_STRING(pos);
DEFINE_SETTING_STRING(label_pos);
DEFINE_SETTING_STRING(size);
DEFINE_SETTING_STRING(zoom);
DEFINE_SETTING_STRING(fullscreen_zoom_mode);
DEFINE_SETTING_STRING(fullscreen);

void MainWindow::restore_state(const SettingsTree &tree){
	QString current_directory,
		current_filename;
	QPoint pos, label_pos;
	QSize size;
	auto zoom = this->zoom;
	auto zoom_mode = this->zoom_mode;
	auto fullscreen_zoom_mode = this->fullscreen_zoom_mode;
	int movement_size = this->movement_size;
	bool using_checkerboard_pattern = this->using_checkerboard_pattern;
	bool fullscreen = false;
#define RESTORE_SETTING(key) tree.get_value(key, key##_setting)
	RESTORE_SETTING(using_checkerboard_pattern);
	RESTORE_SETTING(current_directory);
	RESTORE_SETTING(current_filename);
	RESTORE_SETTING(movement_size);
	RESTORE_SETTING(pos);
	RESTORE_SETTING(label_pos);
	RESTORE_SETTING(size);
	RESTORE_SETTING(zoom);
	RESTORE_SETTING(zoom_mode);
	RESTORE_SETTING(fullscreen_zoom_mode);
	RESTORE_SETTING(fullscreen);

	auto path = current_directory;
	path += QDir::separator();
	path += current_filename;
	this->use_checkerboard_pattern = using_checkerboard_pattern;
	this->using_checkerboard_pattern = !using_checkerboard_pattern;

	this->zoom = zoom;
	this->zoom_mode = ZoomMode::Locked;
	this->fullscreen = fullscreen;
	this->display_image(path);
	this->ui->label->load_state(tree);
	this->zoom_mode = zoom_mode;
	this->fullscreen_zoom_mode = fullscreen_zoom_mode;

	this->ui->label->move(label_pos);
	this->move(pos);
	this->resize(size);
	this->movement_size = movement_size;
	this->fix_positions_and_zoom();
}

#define SAVE_SETTING(x) ret->set_value(#x, this->x)

std::shared_ptr<SettingsTree> MainWindow::save_state() const{
	auto ret = SettingsTree::create_tree();
	ret->set_value(pos_setting, this->pos());
	ret->set_value(size_setting, this->size());
	ret->set_value(label_pos_setting, this->ui->label->pos());
	SAVE_SETTING(using_checkerboard_pattern);
	SAVE_SETTING(current_directory);
	SAVE_SETTING(current_filename);
	SAVE_SETTING(zoom);
	SAVE_SETTING(fullscreen);
	ret->set_value(zoom_mode_setting, (int)this->zoom_mode);
	ret->set_value(fullscreen_zoom_mode_setting, (int)this->fullscreen_zoom_mode);
	SAVE_SETTING(movement_size);
	this->ui->label->save_state(*ret);
	return ret;
}

void MainWindow::reset_settings(){
	this->use_checkerboard_pattern = this->app->get_use_checkerboard_pattern();
	this->using_checkerboard_pattern = !this->use_checkerboard_pattern;
	this->movement_size = this->default_movement_size;
	this->zoom_mode = this->app->get_zoom_mode_for_new_windows();
	this->fullscreen_zoom_mode = this->app->get_fullscreen_zoom_mode_for_new_windows();
}
