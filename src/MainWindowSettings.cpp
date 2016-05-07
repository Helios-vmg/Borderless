/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDir>

void MainWindow::restore_state(const std::shared_ptr<WindowState> &state){
	this->window_state = state;
	this->window_state->set_using_checkerboard_pattern_updated(true);
	auto path = QString::fromStdWString(this->window_state->get_current_directory());
	path += QDir::separator();
	path += QString::fromStdWString(this->window_state->get_current_filename());
	auto temp_zoom_mode = this->window_state->get_zoom_mode();
	this->window_state->set_zoom_mode(ZoomMode::Locked);
	this->display_image(path);
	this->ui->label->load_state(*state);
	this->window_state->set_zoom_mode(temp_zoom_mode);

	this->ui->label->move(this->window_state->get_label_pos().to_QPoint());
	this->move(this->window_state->get_pos().to_QPoint());
	this->resize(this->window_state->get_size().to_QSize());
	this->fix_positions_and_zoom();
}

std::shared_ptr<WindowState> MainWindow::save_state() const{
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
