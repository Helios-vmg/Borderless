/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"

void set_flags(bool &left, bool &right, bool &middle, const QMouseEvent &ev){
	left = check_flag(ev.buttons(), Qt::LeftButton);
	right = check_flag(ev.buttons(), Qt::RightButton);
	middle = check_flag(ev.buttons(), Qt::MiddleButton);
}

void MainWindow::mousePressEvent(QMouseEvent *ev){
	MouseEvent mme(*ev);
	this->not_moved = false;

	if (mme.button_sum > 1)
		return;

	this->first_label_pos = this->ui->label->pos();
	if (mme.left){
		this->reset_left_mouse(mme);
	}else if (mme.right){
		this->first_mouse_pos = mme.absolute;
		this->not_moved = true;
	}
}

void MainWindow::reset_left_mouse(const MouseEvent &ev){
	this->first_mouse_pos = ev.absolute;
	this->first_window_pos = this->pos();
	this->first_window_size = this->size();
	this->set_resize_mode(ev.relative);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev){
	if (this->not_moved)
		this->show_context_menu(ev);
	else
		this->app->save_settings();
	this->not_moved = false;
}

MouseEvent::MouseEvent(const QMouseEvent &ev){
	set_flags(this->left, this->right, this->middle, ev);
	this->button_sum = (int)this->left + (int)this->right + (int)this->middle;
	this->absolute = ev.globalPos();
	this->relative = ev.pos();
}

bool MainWindow::set_cursor_flags(const MouseEvent &ev){
	this->not_moved = false;

	if (ev.button_sum > 1)
		return false;

	ResizeMode rm;
	if (!ev.button_sum)
		rm = this->get_resize_mode(ev.relative);
	else{
		if (!ev.left)
			this->resize_mode = ResizeMode::None;
		rm = this->resize_mode;
	}
	switch (rm){
		case ResizeMode::None:
			this->setCursor(Qt::ArrowCursor);
			break;
		case ResizeMode::Top:
		case ResizeMode::Bottom:
			this->setCursor(Qt::SizeVerCursor);
			break;
		case ResizeMode::Right:
		case ResizeMode::Left:
			this->setCursor(Qt::SizeHorCursor);
			break;
		case ResizeMode::TopLeft:
		case ResizeMode::BottomRight:
			this->setCursor(Qt::SizeFDiagCursor);
			break;
		case ResizeMode::TopRight:
		case ResizeMode::BottomLeft:
			this->setCursor(Qt::SizeBDiagCursor);
			break;
	}

	return ev.button_sum >= 1;
}

#define FTEMP(a, A, b, B) \
	if (this->first_label_pos.a() != this->ui->label->pos().a() || this->first_window_pos.a() != this->pos().a() || this->first_window_size.b() != this->size().b()){ \
		this->first_label_pos.set##A(this->ui->label->pos().a()); \
		this->first_mouse_pos.set##A(mouse_pos.a()); \
		this->first_window_pos.set##A(this->pos().a()); \
		this->first_window_size.set##B(this->size().b()); \
	}

void MainWindow::mouseMoveEvent(QMouseEvent *ev){
	MouseEvent mme(*ev);
	if (!this->set_cursor_flags(mme))
		return;

	const auto &mouse_pos = mme.absolute;

	if (mme.left){
		if (this->window_state->get_fullscreen())
			return;
		if (this->resize_mode == ResizeMode::None)
			this->move_window(this->first_window_pos + mouse_pos - this->first_mouse_pos, mouse_pos);
		else{
			QPoint pos;
			QRect rect;
			if (!this->compute_resize(pos, rect, mouse_pos - this->first_mouse_pos, mouse_pos)){
				this->reset_zoom_slot();
				
				auto copy = mme;
				copy.relative = mme.absolute - this->pos();
				this->reset_left_mouse(copy);
				this->set_cursor_flags(copy);
			}else{
				this->set_window_rect(rect);
				this->ui->label->move(pos);
				FTEMP(x, X, width, Width);
				FTEMP(y, Y, height, Height);
			}
		}
	}else if (mme.right){
		auto new_position = this->first_label_pos + mouse_pos - this->first_mouse_pos;
		if (this->move_image(new_position)){
			this->first_mouse_pos = mouse_pos;
			this->first_label_pos = this->ui->label->pos();
		}
	}
}

#undef FTEMP

bool MainWindow::compute_resize(QPoint &out_label_pos, QRect &out_window_rect, QPoint mouse_offset, const QPoint &mouse_position){
	auto ds = this->app->desktop()->availableGeometry(mouse_position);
	int left = 0,
		top = 0,
		right = 0,
		bottom = 0;
	auto mode = this->resize_mode;
	bool moving_left = check_flag(mode, ResizeMode::Left);
	bool moving_top = check_flag(mode, ResizeMode::Top);
	bool moving_right = check_flag(mode, ResizeMode::Right);
	bool moving_bottom = check_flag(mode, ResizeMode::Bottom);
	if (moving_left)
		left = mouse_offset.x();
	if (moving_top)
		top = mouse_offset.y();
	if (moving_right)
		right = mouse_offset.x();
	if (moving_bottom)
		bottom = mouse_offset.y();

	QRect rect;
	rect.setTopLeft(this->first_window_pos);
	rect.setSize(this->first_window_size);
	//auto first_rect = rect;
	rect.setX(rect.x() + left);
	rect.setY(rect.y() + top);
	rect.setRight(rect.right() + right);
	rect.setBottom(rect.bottom() + bottom);

	int strength = this->app->get_clamp_strength();
	if (this->perform_clamping()){
		if (moving_left && intabs(rect.left() - ds.left()) < strength)
			rect.setLeft(ds.left());

		if (moving_top && intabs(rect.top() - ds.top()) < strength)
			rect.setTop(ds.top());

		if (moving_right && intabs(rect.right() - ds.right()) < strength)
			rect.setRight(ds.right());

		if (moving_bottom && intabs(rect.bottom() - ds.bottom()) < strength)
			rect.setBottom(ds.bottom());
	}

	auto label_rect = this->ui->label->rect();

	auto border_size = this->window_state->get_border_size();
	if ((std::uint32_t)rect.width() < border_size){
		if (right)
			rect.setWidth(border_size);
		else
			rect.setLeft(rect.right() - border_size + 1);
	}
	if ((std::uint32_t)rect.height() < border_size){
		if (bottom)
			rect.setHeight(border_size);
		else
			rect.setTop(rect.bottom() - border_size + 1);
	}

	if (rect.width() > label_rect.width()){
		if (moving_left)
			rect.setX(rect.x() + rect.width() - label_rect.width());
		if (moving_right)
			rect.setWidth(label_rect.width());
	}
	if (rect.height() > label_rect.height()){
		if (moving_top)
			rect.setY(rect.y() + rect.height() - label_rect.height());
		if (moving_bottom)
			rect.setHeight(label_rect.height());
	}

	auto pos = this->first_label_pos;
	pos.setX(pos.x() - (rect.left() - this->first_window_pos.x()));
	pos.setY(pos.y() - (rect.top() - this->first_window_pos.y()));

	if (pos.x() > 0)
		pos.setX(0);
	if (pos.y() > 0)
		pos.setY(0);
	if (pos.x() + label_rect.width() < rect.width())
		pos.setX(rect.width() - label_rect.width());
	if (pos.y() + label_rect.height() < rect.height())
		pos.setY(rect.height() - label_rect.height());

	if (rect.height() <= 0 || rect.width() <= 0)
		return false;

	out_label_pos = pos;
	out_window_rect = rect;
	return true;
}

void MainWindow::move_window(const QPoint &requested_position, const QPoint &mouse_position){
	auto computed_position = this->compute_movement(requested_position, mouse_position);
	this->move_window_rect(computed_position);
}

bool MainWindow::move_image(const QPoint &_new_position){
	auto new_position = _new_position;
	auto label_size = this->ui->label->size();
	auto window_size = this->size();
	bool refresh = false;

	QRect new_label_rect(new_position, label_size);
	QRect window_rect(QPoint(0, 0), window_size);
	QRect allowed_region = new_label_rect;

	if (new_label_rect.width() >= window_rect.width()){
		auto diff = window_rect.width() - new_label_rect.width();
		allowed_region.setLeft(diff);
		allowed_region.setWidth(new_label_rect.width() - diff);
	}else
		allowed_region.moveLeft((window_rect.width() - new_label_rect.width()) / 2);

	if (new_label_rect.height() >= window_rect.height()){
		auto diff = window_rect.height() - new_label_rect.height();
		allowed_region.setTop(diff);
		allowed_region.setHeight(new_label_rect.height() - diff);
	}else
		allowed_region.moveTop((window_rect.height() - new_label_rect.height()) / 2);

	if (new_label_rect.left() < allowed_region.left())
		new_label_rect.moveLeft(allowed_region.left());
	if (new_label_rect.right() > allowed_region.right())
		new_label_rect.moveRight(allowed_region.right());
	if (new_label_rect.top() < allowed_region.top())
		new_label_rect.moveTop(allowed_region.top());
	if (new_label_rect.bottom() > allowed_region.bottom())
		new_label_rect.moveBottom(allowed_region.bottom());

	this->ui->label->move(new_label_rect.topLeft());
	return refresh;
}

bool MainWindow::perform_clamping(){
	return this->app->get_clamp_to_edges() && !check_flag(this->app->keyboardModifiers(), Qt::ControlModifier);
}

bool MainWindow::force_keep_window_in_desktop(){
	return this->perform_clamping();
}

QPoint MainWindow::compute_movement(const QPoint &_new_position, const QPoint &mouse_position){
	auto new_position = _new_position;
	if (this->perform_clamping()){
		auto ds = this->app->desktop()->availableGeometry(mouse_position);
		int x[] = {
			ds.x(),
			ds.x() + ds.width() - this->size().width(),
		};
		int y[] = {
			ds.y(),
			ds.y() + ds.height() - this->size().height(),
		};

		auto c = closest(x, new_position.x());
		auto strength = this->app->get_clamp_strength();
		if (intabs(new_position.x() - c) < strength)
			new_position.setX(c);

		c = closest(y, new_position.y());
		if (intabs(new_position.y() - c) < strength)
			new_position.setY(c);

		if (this->force_keep_window_in_desktop()){
#define FTEMP(a, A) \
	if (new_position.a() < a[0]) \
		new_position.set##A(a[0]); \
	else if (new_position.a() > a[1]) \
		new_position.set##A(a[1])

			FTEMP(x, X);
			FTEMP(y, Y);
#undef FTEMP
		}
	}
	return new_position;
}

void MainWindow::reposition_window(bool do_not_enlarge){
	this->resize_to_max(do_not_enlarge);
	if (this->window_state->get_fullscreen())
		this->resolution_to_window_size();
	this->reposition_image();
}

void MainWindow::reposition_image(){
	this->move_image(this->ui->label->pos());
}

MainWindow::ResizeMode MainWindow::get_resize_mode(const QPoint &pos){
	if (this->window_state->get_fullscreen() || this->current_zoom_mode_is_auto())
		return ResizeMode::None;

	for (int border = this->window_state->get_border_size(); border >= 0; border -= 5){
		bool left = false,
			top = false,
			right = false,
			bottom = false;
		if (pos.x() <= border)
			left = true;
		if (pos.y() <= border)
			top = true;
		if (intabs(pos.x() - this->width()) <= border)
			right = true;
		if (intabs(pos.y() - this->height()) <= border)
			bottom = true;
		int sum = (int)left + (int)right + (int)top + (int)bottom;
		if (sum == 1){
			if (left)
				return ResizeMode::Left;
			if (right)
				return ResizeMode::Right;
			if (top)
				return ResizeMode::Top;
			return ResizeMode::Bottom;
		}
		if (sum == 2){
			if (top){
				if (left)
					return ResizeMode::TopLeft;
				if (right)
					return ResizeMode::TopRight;
			}else if (bottom){
				if (left)
					return ResizeMode::BottomLeft;
				if (right)
					return ResizeMode::BottomRight;
			}
		}
	}
	return ResizeMode::None;
}

void MainWindow::set_resize_mode(const QPoint &pos){
	this->resize_mode = this->get_resize_mode(pos);
}
