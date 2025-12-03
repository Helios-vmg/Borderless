/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDir>

void MainWindow::restore_state(const std::shared_ptr<WindowState> &state, OptionalFuture<LoadedGraphics::create_result> *future){
	this->window_state = state;
	this->window_state->set_using_checkerboard_pattern_updated(true);
	this->window_state->set_zoom_mode(ZoomMode::Locked);
	auto will_need_hash = this->window_state->get_loaded_preferred_position() && !!this->app->get_persistent_settings();
	auto pos = this->window_state->get_pos();
	auto size = this->window_state->get_size();
	auto result = this->open_path_and_display_image(this->window_state->get_path(), will_need_hash, future);
	this->ui->label->load_state(*this->window_state);
	this->window_state->set_zoom_mode(this->window_state->get_zoom_mode());

	this->ui->label->move(this->window_state->get_label_pos());
	if (result.preferred_position)
		pos = result.preferred_position->get_pos();
	this->move(pos);
	this->window_rect.moveTopLeft(pos);
	this->current_desktop = unique_identifier(*this->screen());
	switch (result.status){
		case OpenResult::Status::Success:
			if (!result.preferred_position){
				this->resize(size);
				this->fix_positions_and_zoom(true);
			}else{
				this->resize(result.preferred_position->get_size());
				this->ensure_border_sizes_are_reasonable();
			}
			break;
		case OpenResult::Status::TemporaryFail:
			this->app->report_temporary_failure(state);
		case OpenResult::Status::PermanentFail:
			break;
	}
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
