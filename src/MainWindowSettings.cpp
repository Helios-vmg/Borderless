/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDir>

void MainWindow::restore_state(const std::shared_ptr<WindowState> &state){
	this->window_state = state;
	this->window_state->set_using_checkerboard_pattern_updated(true);
	this->last_set_by_user = state->get_last_set_by_user();
	QString path;
	if (!this->window_state->get_file_is_url()){
		path = this->window_state->get_current_directory();
		auto c = QDir::separator().toLatin1();
		path += c;
		path += this->window_state->get_current_filename();
	}else
		path = this->window_state->get_current_url();

	auto temp_zoom_mode = this->window_state->get_zoom_mode();
	this->window_state->set_zoom_mode(ZoomMode::Locked);
	bool success = this->open_path_and_display_image(path);
	this->ui->label->load_state(*this->window_state);
	this->window_state->set_zoom_mode(temp_zoom_mode);

	auto pos = this->window_state->get_pos_u();
	if (!(this->last_set_by_user = !!this->screen()->virtualSiblingAt(pos)))
		pos = this->window_state->get_pos();
	else
		this->window_state->override_computed();
	this->ui->label->move(this->window_state->get_label_pos());
	this->move(pos);
	this->current_desktop = unique_identifier(*this->screen());
	this->window_rect.moveTopLeft(pos);
	if (!success)
		return;
	this->resize(this->window_state->get_size());
	this->fix_positions_and_zoom(true);
}

std::shared_ptr<WindowState> MainWindow::save_state() const{
	this->window_state->set_last_set_by_user(this->last_set_by_user);
	this->window_state->set_pos(this->pos());
	this->window_state->set_size(this->size());
	this->window_state->set_label_pos(this->ui->label->pos());
	this->ui->label->save_state(*this->window_state);
	return this->window_state;
}

void MainWindow::reset_settings(){
	auto &state = this->window_state;
	if (!state)
		state = std::make_shared<WindowState>();
	WindowState defaults;
	state->set_using_checkerboard_pattern(this->app->get_use_checkerboard_pattern());
	state->set_movement_size(defaults.get_movement_size());
	state->set_zoom_mode(this->app->get_zoom_mode_for_new_windows());
	state->set_fullscreen_zoom_mode(this->app->get_fullscreen_zoom_mode_for_new_windows());
}
