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

void MainWindow::setup_shortcuts(){
	static struct Pair{
		const char *command;
		const char *slot;
	} pairs[] = {
#define SETUP_SHORTCUT(command, slot) { command, SLOT(slot)},
		SETUP_SHORTCUT(quit_command, quit_slot())
		SETUP_SHORTCUT(quit2_command, quit2_slot())
		SETUP_SHORTCUT(next_command, next_slot())
		SETUP_SHORTCUT(back_command, back_slot())
		SETUP_SHORTCUT(background_swap_command, background_swap_slot())
		SETUP_SHORTCUT(close_command, close_slot())
		SETUP_SHORTCUT(zoom_in_command, zoom_in_slot())
		SETUP_SHORTCUT(zoom_out_command, zoom_out_slot())
		SETUP_SHORTCUT(reset_zoom_command, reset_zoom_slot())
		SETUP_SHORTCUT(up_command, up_slot())
		SETUP_SHORTCUT(down_command, down_slot())
		SETUP_SHORTCUT(left_command, left_slot())
		SETUP_SHORTCUT(right_command, right_slot())
		SETUP_SHORTCUT(up_big_command, up_big_slot())
		SETUP_SHORTCUT(down_big_command, down_big_slot())
		SETUP_SHORTCUT(left_big_command, left_big_slot())
		SETUP_SHORTCUT(right_big_command, right_big_slot())
		SETUP_SHORTCUT(cycle_zoom_mode_command, cycle_zoom_mode_slot())
		SETUP_SHORTCUT(toggle_lock_zoom_command, toggle_lock_zoom_slot())
		SETUP_SHORTCUT(go_to_start_command, go_to_start())
		SETUP_SHORTCUT(go_to_end_command, go_to_end())
		SETUP_SHORTCUT(toggle_fullscreen_command, toggle_fullscreen())
		SETUP_SHORTCUT(rotate_left_command, rotate_left())
		SETUP_SHORTCUT(rotate_right_command, rotate_right())
		SETUP_SHORTCUT(rotate_left_fine_command, rotate_left_fine())
		SETUP_SHORTCUT(rotate_right_fine_command, rotate_right_fine())
		SETUP_SHORTCUT(flip_h_command, flip_h())
		SETUP_SHORTCUT(flip_v_command, flip_v())
		SETUP_SHORTCUT(minimize_command, minimize_slot())
		SETUP_SHORTCUT(minimize_all_command, minimize_all_slot())
		SETUP_SHORTCUT(show_options_command, show_options_dialog())
	};

	for (auto &c : this->connections)
		this->disconnect(c);
	
	this->connections.clear();
	this->shortcuts.clear();

	auto &shortcuts = this->app->get_shortcuts();
	for (auto &p : pairs){
		auto setting = shortcuts.get_shortcut_setting(p.command);
		if (!setting)
			continue;
		for (auto &seq : setting->sequences)
			this->setup_shortcut(seq, p.slot);
	}
}

void MainWindow::setup_shortcut(const QKeySequence &sequence, const char *slot){
	std::shared_ptr<QShortcut> shortcut(new QShortcut(sequence, this));
	this->shortcuts.push_back(shortcut);
	auto connection = connect(shortcut.get(), SIGNAL(activated()), this, slot);
	this->connections.push_back(connection);
}

void MainWindow::quit_slot(){
	this->app->quit();
}

void MainWindow::quit2_slot(){
	this->app->quit_and_discard_state();
}

void MainWindow::next_slot(){
	this->move_in_direction(true);
}

void MainWindow::back_slot(){
	this->move_in_direction(false);
}

void MainWindow::background_swap_slot(){
	this->use_checkerboard_pattern = !this->use_checkerboard_pattern;
	this->set_background();
}

void MainWindow::close_slot(){
	this->close();
}

void MainWindow::zoom_in_slot(){
	this->change_zoom(true);
}

void MainWindow::zoom_out_slot(){
	this->change_zoom(false);
}

void MainWindow::up_slot(){
	this->offset_image(QPoint(0, this->movement_size));
}

void MainWindow::down_slot(){
	this->offset_image(QPoint(0, -this->movement_size));
}

void MainWindow::left_slot(){
	this->offset_image(QPoint(this->movement_size, 0));
}

void MainWindow::right_slot(){
	this->offset_image(QPoint(-this->movement_size, 0));
}

void MainWindow::up_big_slot(){
	auto size = this->size().height() / 2;
	this->offset_image(QPoint(0, size));
}

void MainWindow::down_big_slot(){
	auto size = this->size().height() / 2;
	this->offset_image(QPoint(0, -size));
}

void MainWindow::left_big_slot(){
	auto size = this->size().width() / 2;
	this->offset_image(QPoint(size, 0));
}

void MainWindow::right_big_slot(){
	auto size = this->size().width() / 2;
	this->offset_image(QPoint(-size, 0));
}

void MainWindow::offset_image(const QPoint &offset){
	this->move_image(this->ui->label->pos() + offset);
}

void MainWindow::reset_zoom_slot(){
	int zoom = this->get_current_zoom();
	this->get_current_zoom_mode() = ZoomMode::Normal;
	this->ui->label->reset_transform();
	this->set_zoom();
	this->apply_zoom(zoom);
}

void cycle_zoom_mode(ZoomMode &mode){
	switch (mode){
		case ZoomMode::Normal:
		case ZoomMode::Locked:
			mode = ZoomMode::AutoFit;
			break;
		case ZoomMode::AutoFit:
			mode = ZoomMode::AutoFill;
			break;
		case ZoomMode::AutoFill:
			mode = ZoomMode::Normal;
			break;
	}
}

void MainWindow::cycle_zoom_mode_slot(){
	cycle_zoom_mode(this->get_current_zoom_mode());
	auto zoom = this->get_current_zoom();
	this->set_zoom();
	this->apply_zoom(zoom);
	this->set_background_sizes();
}

void toggle_lock_zoom(ZoomMode &mode){
	mode = mode == ZoomMode::Locked ? ZoomMode::Normal : ZoomMode::Locked;
}

void MainWindow::toggle_lock_zoom_slot(){
	toggle_lock_zoom(this->zoom_mode);
}

void MainWindow::go_to_start(){
	if (!this->directory_iterator)
		return;
	this->set_iterator();
	size_t i = this->directory_iterator->pos();
	this->directory_iterator->to_start();
	if (this->directory_iterator->pos() == i)
		return;
	this->moving_forward = true;
	this->display_image(**this->directory_iterator);
}

void MainWindow::go_to_end(){
	if (!this->directory_iterator)
		return;
	this->set_iterator();
	size_t i = this->directory_iterator->pos();
	this->directory_iterator->to_end();
	if (this->directory_iterator->pos() == i)
		return;
	this->moving_forward = false;
	this->display_image(**this->directory_iterator);
}

void MainWindow::toggle_fullscreen(){
	auto zoom = this->get_current_zoom();
	this->fullscreen = !this->fullscreen;
	if (!this->fullscreen){
		this->apply_zoom(zoom);
		this->setGeometry(this->window_rect);
		this->set_image_pos(this->image_pos);
	}else{
		this->image_pos = this->get_image_pos();
		this->set_zoom();
		this->apply_zoom(zoom);
		this->resolution_to_window_size();
		this->reposition_image();
		//this->move_image(QPoint(0, 0));
	}
	this->set_background_sizes();
}

void MainWindow::rotate(bool right, bool fine){
	this->ui->label->rotate((right ? 1 : -1) * (fine ? 1 : 90));
	this->fix_positions_and_zoom();
	//this->reposition_image();
	//this->ui->label->repaint();
}

void MainWindow::rotate_left(){
	this->rotate(false);
}

void MainWindow::rotate_right(){
	this->rotate(true);
}

void MainWindow::rotate_left_fine(){
	this->rotate(false, true);
}

void MainWindow::rotate_right_fine(){
	this->rotate(true, true);
}

void MainWindow::minimize_slot(){
	this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::minimize_all_slot(){
	this->app->minimize_all();
}

void MainWindow::flip_h(){
	this->ui->label->flip(true);
	this->ui->label->repaint();
}

void MainWindow::flip_v(){
	this->ui->label->flip(false);
	this->ui->label->repaint();
}

void MainWindow::show_options_dialog(){
	this->app->show_options();
}
