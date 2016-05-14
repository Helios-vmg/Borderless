/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Misc.h"
#include "RotateDialog.h"
#include <algorithm>
#include <limits>
#include <QImage>
#include <QMetaEnum>
#include <QDir>
#include <exception>
#include <cassert>

MainWindow::MainWindow(ImageViewerApplication &app, const QStringList &arguments, QWidget *parent):
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		app(&app){
	this->init();
	if (arguments.size() >= 2)
		this->display_image(arguments[1]);
}

MainWindow::MainWindow(ImageViewerApplication &app, const std::shared_ptr<WindowState> &state, QWidget *parent):
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		app(&app){
	this->init();
	this->restore_state(state);
	this->set_background();
}

MainWindow::~MainWindow(){
	this->cleanup();
}

void MainWindow::init(){
	if (!this->window_state)
		this->window_state = std::make_shared<WindowState>();
	this->moving_forward = true;
	this->not_moved = false;
	this->window_state->set_zoom(1);
	this->window_state->set_fullscreen_zoom(1);
	this->color_calculated = false;
	this->window_state->set_fullscreen(false);
	this->ui->setupUi(this);
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
	this->reset_settings();
	this->setup_backgrounds();
	this->set_desktop_size();
	this->move(this->desktop_size.topLeft());
	this->setMouseTracking(true);
	this->ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
	this->setup_shortcuts();

	connect(this->ui->label, SIGNAL(transform_updated()), this, SLOT(label_transform_updated()));
}

void MainWindow::set_desktop_size(int screen){
	this->desktop_size = this->app->desktop()->availableGeometry(screen);
	this->screen_size = this->app->desktop()->screenGeometry(screen);
}

QPoint MainWindow::get_image_pos() const{
	return this->ui->label->pos();
}

void MainWindow::set_image_pos(const QPoint &p){
	this->ui->label->move(p);
	if (!this->window_state->get_fullscreen())
		this->image_pos = p;
}

void MainWindow::set_zoom(){
	auto ds = this->desktop_size.size();
	if (this->window_state->get_fullscreen())
		ds = this->screen_size.size();
	auto ps = this->displayed_image->get_size();
	this->ui->label->compute_size_no_zoom(ps);
	double dr = (double)ds.width() / (double)ds.height();
	double pr = (double)ps.width() / (double)ps.height();
	auto zoom = this->get_current_zoom();
	switch (this->get_current_zoom_mode()){
		case ZoomMode::Normal:
			zoom = 1;
			break;
		case ZoomMode::Locked:
			break;
		case ZoomMode::AutoFit:
			if (dr < pr)
				zoom = (double)ds.width() / (double)ps.width();
			else
				zoom = (double)ds.height() / (double)ps.height();
			break;
		case ZoomMode::AutoFill:
			if (dr > pr)
				zoom = (double)ds.width() / (double)ps.width();
			else
				zoom = (double)ds.height() / (double)ps.height();
			break;
	}
	this->set_current_zoom(zoom);
}

void MainWindow::apply_zoom(bool first_display, double old_zoom){
	auto label_pos = this->ui->label->pos();
	auto center = to_QPoint(this->size()) / 2;
	auto center_at = center - label_pos;

	auto zoom = this->get_current_zoom();

	this->display_image(this->displayed_image);

	auto new_location = center - center_at * (zoom / old_zoom);

	if (first_display){
		if (!this->app->get_center_when_displayed()){
			if (new_location.x() < 0)
				new_location.setX(0);
			if (new_location.y() < 0)
				new_location.setY(0);
		}
	}

	this->move_image(new_location);
}

void MainWindow::change_zoom(bool in){
	auto zoom = this->get_current_zoom();
	auto old_zoom = zoom;
#if 0
	if (in)
		zoom += 0.25;
	else if (zoom > quarter)
		zoom -= 0.25;
	else
		return;
#else
	zoom *= in ? 1.25 : (1.0 / 1.25);
#endif
	this->set_current_zoom(zoom);
	this->apply_zoom(false, old_zoom);
	if (this->current_zoom_mode_is_auto())
		this->set_current_zoom_mode(ZoomMode::Normal);
}

void MainWindow::setup_backgrounds(){
	{
		auto widget = this->ui->checkerboard;
		QPalette palette = widget->palette();
		QImage alpha(":/alpha.png");
		if (alpha.isNull())
			throw std::exception("Resource not found: \"alpha.png\"");
		palette.setBrush(QPalette::Window, QBrush(alpha));
		widget->setPalette(palette);
	}
	this->set_solid(Qt::black);
	this->ui->checkerboard->hide();
	this->ui->solid->hide();
	this->ui->checkerboard->move(0, 0);
	this->ui->solid->move(0, 0);
}

void MainWindow::set_solid(const QColor &col){
	QPalette palette = this->ui->solid->palette();
	palette.setBrush(QPalette::Window, QBrush(col));
	this->ui->solid->setPalette(palette);
}

void MainWindow::set_background(bool force){
	if (!this->window_state->get_using_checkerboard_pattern_updated() && !force)
		return;

	QWidget *p1 = this->ui->checkerboard;
	QWidget *p2 = this->ui->solid;
	if (!this->window_state->get_using_checkerboard_pattern()){
		this->set_solid(this->displayed_image->get_background_color());
		std::swap(p1, p2);
	}
	p1->show();
	p2->hide();
	this->window_state->set_using_checkerboard_pattern_updated(false);
}

void MainWindow::set_background_sizes(){
	QRect wr(QPoint(0, 0), this->size());
	auto &label = this->ui->label;
	auto lr = label->get_geometry();
	lr &= wr;
	this->ui->checkerboard->setGeometry(lr);
	this->ui->solid->setGeometry(lr);
}

void MainWindow::show_nothing(){
	qDebug() << "MainWindow::show_nothing()";
	this->displayed_image.reset();
	this->ui->label->setText("no image");
	this->ui->label->setAlignment(Qt::AlignCenter);
	this->set_solid(Qt::black);
	this->resize(800, 600);
	this->ui->label->resize(this->size());
	this->window_state->reset_border_size();
}

void MainWindow::resize_to_max(){
	auto rect = this->geometry();
	auto &label = this->ui->label;
	label->resize(label->get_size());
	rect.setSize(label->size().boundedTo(this->desktop_size.size()));
	if (rect.left() < this->desktop_size.left())
		rect.moveLeft(this->desktop_size.left());
	else if (rect.right() > this->desktop_size.right())
		rect.moveRight(this->desktop_size.right());
	if (rect.top() < this->desktop_size.top())
		rect.moveTop(this->desktop_size.top());
	else if (rect.bottom() > this->desktop_size.bottom())
		rect.moveBottom(this->desktop_size.bottom());
	this->set_window_rect(rect);
}

void MainWindow::advance(){
	if (!this->directory_iterator)
		return;
	if (this->moving_forward)
		++*this->directory_iterator;
	else
		--*this->directory_iterator;
}

void MainWindow::set_iterator(){
	this->directory_iterator->advance_to(QString::fromStdWString(this->window_state->get_current_filename()));
}

double MainWindow::get_current_zoom() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom() : this->window_state->get_fullscreen_zoom();
}

void MainWindow::set_current_zoom(double value){
	if (this->window_state->get_fullscreen())
		this->window_state->set_fullscreen_zoom(value);
	else
		this->window_state->set_zoom(value);
}

void MainWindow::set_current_zoom_mode(const ZoomMode &mode){
	if (!this->window_state->get_fullscreen())
		this->window_state->set_zoom_mode(mode);
	else
		this->window_state->set_fullscreen_zoom_mode(mode);
}

ZoomMode MainWindow::get_current_zoom_mode() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom_mode() : this->window_state->get_fullscreen_zoom_mode();
}

void MainWindow::move_in_direction(bool forward){
	if (!this->directory_iterator)
		return;
	this->set_iterator();
	this->moving_forward = forward;
	size_t i = this->directory_iterator->pos();
	this->advance();
	if (this->directory_iterator->pos() == i)
		return;
	this->display_image(**this->directory_iterator);
}

void MainWindow::display_image(QString path){
	std::shared_ptr<LoadedGraphics> li;
	size_t i = 0;
	auto &label = this->ui->label;
	if (!!this->directory_iterator)
		i = this->directory_iterator->pos();
	while (true){
		li = LoadedGraphics::create(path);
		qDebug() << path;
		if (!li->is_null())
			break;
		if (!!this->directory_iterator){
			this->advance();
			if (this->directory_iterator->pos() == i)
				break;
			path = **this->directory_iterator;
		}else
			break;
	}
	if (li->is_null()){
		this->show_nothing();
		return;
	}
	this->color_calculated = false;
	label->move(0, 0);
	QString current_directory,
		current_filename;
	split_path(current_directory, current_filename, path);
	this->window_state->set_current_directory(current_directory.toStdWString());
	this->window_state->set_current_filename(current_filename.toStdWString());
	this->setWindowTitle(current_filename);
	if (!this->directory_iterator)
		this->directory_iterator = this->app->request_directory(current_directory);
	this->displayed_image = li;

	label->reset_transform();
	this->set_zoom();

	if (this->get_current_zoom() != 1)
		this->apply_zoom(true, 1);
	else
		this->display_image(li);
}

void MainWindow::display_image(std::shared_ptr<LoadedGraphics> graphics){
	auto zoom = this->get_current_zoom();
	auto &label = this->ui->label;
	label->set_image(*graphics);
	//label->setPixmap(pixmap);
	label->set_zoom(zoom);
	auto size = label->get_size();
	int mindim = std::min(size.width(), size.height());
	this->window_state->set_border_size(
		mindim < this->window_state->default_border_size * 3 ?
		mindim / 3 :
		this->window_state->default_border_size
	);
	label->resize(size);

	if (!this->color_calculated && this->displayed_image->has_alpha()){
		this->set_background(true);
		this->color_calculated = true;
	}

	this->reposition_window();
	this->set_background_sizes();
}

void MainWindow::show_rotate_dialog(){
	RotateDialog dialog(*this);
	dialog.exec();
}

void MainWindow::show_context_menu(QMouseEvent *ev){
	this->app->postEvent(this, new QContextMenuEvent(QContextMenuEvent::Other, ev->screenPos().toPoint()));
}

void MainWindow::build_context_menu(QMenu &main_menu, QMenu &lua_submenu){
	main_menu.addAction("Transform...", this, SLOT(show_rotate_dialog()));
	main_menu.addMenu(&lua_submenu);
	main_menu.addAction("Close", this, SLOT(close_slot()), this->app->get_shortcuts().get_current_sequence(close_command));
}

//void MainWindow::keyReleaseEvent(QKeyEvent *ev){
//	QMainWindow::keyReleaseEvent(ev);
//}

void MainWindow::resizeEvent(QResizeEvent *){
	this->set_background_sizes();
}

void MainWindow::changeEvent(QEvent *ev){
	if (ev->type() == QEvent::WindowStateChange){
		auto &event = dynamic_cast<QWindowStateChangeEvent &>(*ev);
		if (this->isMaximized())
			this->setWindowState(event.oldState());
		else
			ev->accept();
	}else
		ev->accept();
}

void MainWindow::closeEvent(QCloseEvent *){
	this->cleanup();
	emit closing(this);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *ev){
	if (ev->reason() != QContextMenuEvent::Other)
		return;
	this->not_moved = false;
	auto menu = this->app->build_context_menu(this);
	menu->move(ev->pos());
	menu->exec();
}

//bool MainWindow::event(QEvent *event){
//
//}

void MainWindow::cleanup(){
	this->app->release_directory(this->directory_iterator);
	this->directory_iterator.reset();
}

void MainWindow::resolution_change(int screen){
	auto desktop = this->app->desktop();
	int this_screen = desktop->screenNumber(this);
	if (screen != this_screen)
		return;
	this->set_desktop_size(this_screen);
	this->fix_positions_and_zoom();
}

void MainWindow::resolution_to_window_size(){
	this->setGeometry(this->screen_size);
}

void MainWindow::fix_positions_and_zoom(){
	if (this->current_zoom_mode_is_auto()){
		auto zoom = this->get_current_zoom();
		this->set_zoom();
		this->apply_zoom(false, zoom);
	}
	this->reposition_window();
	this->reposition_image();
	this->ui->label->repaint();
}

void MainWindow::work_area_change(int screen){
	auto desktop = this->app->desktop();
	int this_screen = desktop->screenNumber(this);
	if (screen != this_screen)
		return;
	this->set_desktop_size(this_screen);
	this->fix_positions_and_zoom();
}

void MainWindow::resize_window_rect(const QSize &s){
	this->window_rect.setSize(s);
	if (!this->window_state->get_fullscreen())
		this->resize(s);
}

void MainWindow::move_window_rect(const QPoint &p){
	this->window_rect.setX(p.x());
	this->window_rect.setY(p.y());
	if (!this->window_state->get_fullscreen())
		this->move(p);
}

void MainWindow::set_window_rect(const QRect &r){
	this->window_rect = r;
	if (!this->window_state->get_fullscreen())
		this->setGeometry(r);
}

void MainWindow::label_transform_updated(){
	this->set_background_sizes();
}

QMatrix MainWindow::get_image_transform() const{
	return this->ui->label->get_transform();
}

double MainWindow::set_image_transform(const QMatrix &m){
	this->ui->label->set_transform(m);
	this->fix_positions_and_zoom();
	return this->window_state->get_zoom();
}

double MainWindow::get_image_zoom() const{
	return this->window_state->get_zoom();
}

void MainWindow::set_image_zoom(double x){
	//double &zoom = this->get_current_zoom();
	double last = this->get_current_zoom();
	this->set_current_zoom(x);
	this->apply_zoom(false, last);
	this->window_state->set_zoom(x);
	this->ui->label->set_zoom(x);
	this->set_current_zoom_mode(ZoomMode::Normal);
}
