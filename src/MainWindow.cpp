/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Misc.h"
#include "RotateDialog.h"
#include <algorithm>
#include <limits>
#include <QImage>
#include <QMetaEnum>
#include <QDir>
#include <exception>
#include <cassert>
#include "GenericException.h"

MainWindow::MainWindow(ImageViewerApplication &app, const QStringList &arguments, QWidget *parent):
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		app(&app){
	this->init(false);
	if (arguments.size() >= 2)
		this->open_path_and_display_image(arguments[1]);
}

TransparentMainWindow::TransparentMainWindow(ImageViewerApplication &app, const std::shared_ptr<WindowState> &state, QWidget *parent):
		MainWindow(app, state, parent){
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->ui->checkerboard->hide();
	this->ui->solid->hide();
}


MainWindow::MainWindow(ImageViewerApplication &app, const std::shared_ptr<WindowState> &state, QWidget *parent):
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		app(&app){
	this->init(true);
	this->restore_state(state);
	this->set_background();
}

MainWindow::~MainWindow(){
	this->cleanup();
}

void MainWindow::init(bool restoring){
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
	assert(this->desktop_sizes.size());
	if (restoring){
		this->current_desktop = unique_identifier(*this->screen());
		this->move(this->desktop_sizes[this->current_desktop].topLeft());
	}else{
		auto screen = this->app->screenAt(QCursor::pos());
		this->current_desktop = unique_identifier(*screen);
		auto pos = this->desktop_sizes[this->current_desktop].topLeft();
		this->move(pos);
		this->window_rect.moveTopLeft(pos);
	}

	this->setMouseTracking(true);
	this->ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
	this->setup_shortcuts();

	connect(this->ui->label, SIGNAL(transform_updated()), this, SLOT(label_transform_updated()));
}

void MainWindow::set_current_desktop_and_fix_positions_by_window_position(std::string old_desktop){
	this->current_desktop = unique_identifier(*this->screen());
	if (this->current_desktop != old_desktop)
		this->fix_positions_and_zoom();
}

void MainWindow::set_desktop_size(){
	for (auto s : this->app->screens())
		this->set_desktop_size(*s);
}

void MainWindow::set_desktop_size(QScreen &screen){
	auto s = unique_identifier(screen);
	this->desktop_sizes[s] = screen.availableGeometry();
	this->screen_sizes[s] = screen.geometry();
}

void MainWindow::save_image_pos(bool force){
	if (this->window_state->get_fullscreen() && !force)
		return;
	auto p = this->ui->label->pos();
	qDebug() << "Saving image position at " << p;
	this->image_pos = p;
}

void MainWindow::restore_image_pos(){
	QPoint p;
	if (this->image_pos){
		p = *this->image_pos;
		qDebug() << "Restoring image position: " << p;
	}else{
		qDebug() << "Saved image position in unset state. Restoring to " << p;
	}
	this->ui->label->move(p);
}

void MainWindow::clear_image_pos(){
	qDebug() << "Clearing image position.";
	this->image_pos.clear();
}

double ratio(const QSize &size){
	return (double)size.width() / (double)size.height();
}

int area(const QSize &size){
	return size.width() * size.height();
}

#define IS_WIDER >

MainWindow::ZoomResult MainWindow::compute_zoom(int override_rotation){
	auto &screen = this->current_desktop;
	auto desktop_size = this->desktop_sizes[screen].size();
	if (this->window_state->get_fullscreen())
		desktop_size = this->screen_sizes[screen].size();

	auto image_size = this->displayed_image->get_size();
	double desktop_ratio = ratio(desktop_size);

	auto zoom_mode = this->get_current_zoom_mode();
	auto old_transform = this->ui->label->get_transform();
	if (override_rotation < 0){
		if (this->current_zoom_mode_is_auto_rotation()){
			auto res0 = this->compute_zoom(0);
			auto res1 = this->compute_zoom(1);
			switch (zoom_mode){
				case ZoomMode::AutoRotFit:
					this->ui->label->override_rotation(area(res1.label_size) > area(res0.label_size) ? -90 : 0);
					break;
				case ZoomMode::AutoRotFill:
					this->ui->label->override_rotation(area(res1.label_size) < area(res0.label_size) ? -90 : 0);
					break;
			}
		}
	}else
		this->ui->label->override_rotation(override_rotation ? -90 : 0);

	auto label_size = this->ui->label->compute_size_no_zoom(image_size);
	double label_ratio = ratio(label_size);

	auto zoom = this->get_current_zoom();
	switch (zoom_mode){
		case ZoomMode::Normal:
			zoom = 1;
			break;
		case ZoomMode::Locked:
			break;
		case ZoomMode::AutoFit:
		case ZoomMode::AutoRotFit:
			if (label_ratio IS_WIDER desktop_ratio)
				zoom = (double)desktop_size.width() / (double)label_size.width();
			else
				zoom = (double)desktop_size.height() / (double)label_size.height();
			break;
		case ZoomMode::AutoFill:
		case ZoomMode::AutoRotFill:
			if (desktop_ratio IS_WIDER label_ratio)
				zoom = (double)desktop_size.width() / (double)label_size.width();
			else
				zoom = (double)desktop_size.height() / (double)label_size.height();
			break;
	}
	label_size = this->ui->label->compute_size(image_size, zoom);
	if (override_rotation >= 0)
		this->ui->label->set_transform(old_transform);
	return { zoom, label_size };
}

void MainWindow::set_zoom(){
	this->set_current_zoom(this->compute_zoom().zoom);
}

void MainWindow::apply_zoom(bool first_display, double old_zoom){
	auto label_pos = this->ui->label->pos();
#ifdef _DEBUG
	qDebug() << "MainWindow::apply_zoom(): label_pos = " << label_pos;
#endif
	auto center = to_QPoint(this->size()) / 2;
#ifdef _DEBUG
	qDebug() << "MainWindow::apply_zoom(): center = " << center;
#endif
	auto center_at = center - label_pos;
#ifdef _DEBUG
	qDebug() << "MainWindow::apply_zoom(): center_at = " << center_at;
#endif

	auto zoom = this->get_current_zoom();
#ifdef _DEBUG
	qDebug() << "MainWindow::apply_zoom(): zoom = " << zoom;
#endif

	this->display_image_in_label(this->displayed_image, first_display);

	if (zoom != 1){
		auto new_location = center - center_at * (zoom / old_zoom);
#ifdef _DEBUG
		qDebug() << "MainWindow::apply_zoom(): center - center_at * (zoom / old_zoom) = "
			<< center << " - " << center_at << " * (" << zoom << " / " << old_zoom << ") = " << new_location;
#endif

		if (first_display && !this->app->get_center_when_displayed()){
			if (new_location.x() < 0)
				new_location.setX(0);
			if (new_location.y() < 0)
				new_location.setY(0);
		}

		this->move_image(new_location);
	}
}

void MainWindow::change_zoom(bool in){
	auto zoom = this->get_current_zoom();
	auto old_zoom = zoom;
	zoom *= in ? 1.25 : (1.0 / 1.25);
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
			throw GenericException("Resource not found: \"alpha.png\"");
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
	if (!this->window_state->get_using_checkerboard_pattern() && this->displayed_image){
		this->set_solid(this->displayed_image->get_background_color());
		std::swap(p1, p2);
	}
	p1->show();
	p2->hide();
	this->window_state->set_using_checkerboard_pattern_updated(false);
	this->app->save_settings();
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
	this->resize(800, 600);
	this->ui->label->move(0, 0);
	this->ui->label->resize(this->size());
	this->window_state->reset_border_size();
	this->set_background_sizes();
	this->resize_to_max();
}

void MainWindow::resize_to_max(bool do_not_enlarge){
	auto &label = this->ui->label;
	auto label_size = label->get_size();
	label->resize(label_size);
	auto rect = this->geometry();
	if (this->app->get_option_values()->get_resize_windows_on_monitor_change()){
		auto ds = this->desktop_sizes[this->current_desktop];
		auto new_size = ds.size();
		new_size = label->size().boundedTo(new_size);
		if (do_not_enlarge)
			new_size = new_size.boundedTo(this->size());
		rect.setSize(new_size);
		if (rect.left() < ds.left())
			rect.moveLeft(ds.left());
		else if (rect.right() > ds.right())
			rect.moveRight(ds.right());
		if (rect.top() < ds.top())
			rect.moveTop(ds.top());
		else if (rect.bottom() > ds.bottom())
			rect.moveBottom(ds.bottom());
	}else
		rect.setSize(label_size);
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
	this->directory_iterator->advance_to(this->window_state->get_current_filename());
}

double MainWindow::get_current_zoom() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom() : this->window_state->get_fullscreen_zoom();
}

void MainWindow::set_current_zoom(double value){
	if (this->window_state->get_fullscreen())
		this->window_state->set_fullscreen_zoom(value);
	else
		this->window_state->set_zoom(value);
	this->app->save_settings();
}

void MainWindow::set_current_zoom_mode(const ZoomMode &mode){
	if (!this->window_state->get_fullscreen())
		this->window_state->set_zoom_mode(mode);
	else
		this->window_state->set_fullscreen_zoom_mode(mode);
	this->app->save_settings();
}

ZoomMode MainWindow::get_current_zoom_mode() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom_mode() : this->window_state->get_fullscreen_zoom_mode();
}

void MainWindow::move_in_direction(bool forward){
	if (!this->directory_iterator)
		return;
	this->set_iterator();
	this->moving_forward = forward;
	auto old_pos = this->directory_iterator->pos();
	this->advance();
	if (this->directory_iterator->pos() == old_pos)
		return;
	this->clear_image_pos();
	this->open_path_and_display_image(**this->directory_iterator);
	this->app->save_settings();
}

class ElapsedTimer{
	QString task;
	clock_t t0;
public:
	ElapsedTimer(const QString &task): task(task){
		qDebug() << "Task " << this->task << " started.";
		this->t0 = clock();
	}
	~ElapsedTimer(){
		auto t1 = clock();
		qDebug() << "Task " << this->task << " took " << (t1 - t0) / (double)CLOCKS_PER_SEC << " seconds.";
	}
};

bool MainWindow::open_path_and_display_image(QString path){
	ElapsedTimer et((QString)"open_path_and_display_image(" + path + ")");
	std::shared_ptr<LoadedGraphics> li;
	size_t i = 0;
	auto &label = this->ui->label;
	if (!!this->directory_iterator)
		i = this->directory_iterator->pos();
	while (true){
		li = LoadedGraphics::create(*this->app, path);
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

	QString current_filename;

	bool unset = true;
	if (!this->directory_iterator){
		auto di = this->app->request_directory_iterator_by_url(path);
		if (di){
			this->directory_iterator = di;
			current_filename = this->app->get_filename_from_url(path);
			this->window_state->set_file_is_url(true);
			this->window_state->set_current_url(path);
			this->window_state->set_current_filename(current_filename);
			unset = false;
		}
	}else if (!this->directory_iterator->get_is_local()){
		current_filename = this->directory_iterator->get_current_filename();
		this->window_state->set_file_is_url(true);
		this->window_state->set_current_url(path);
		this->window_state->set_current_filename(current_filename);
		unset = false;
	}
	if (unset){
		QString current_directory;
		split_path(current_directory, current_filename, path);
		this->window_state->set_current_directory(current_directory);
		this->window_state->set_current_filename(current_filename);
		this->window_state->set_file_is_url(false);
		if (!this->directory_iterator)
			this->directory_iterator = this->app->request_local_directory_iterator(current_directory);
	}

	if (li->is_null()){
		this->show_nothing();
		return false;
	}
	this->color_calculated = false;
	label->move(0, 0);
	this->setWindowTitle(current_filename);
	this->displayed_image = li;

	label->reset_transform();
	this->set_zoom();

	this->apply_zoom(true, 1);
	return true;
}

void MainWindow::display_filtered_image(const std::shared_ptr<LoadedGraphics> &graphics){
	this->displayed_image = graphics;
	this->display_image_in_label(graphics, false);
}

void MainWindow::display_image_in_label(const std::shared_ptr<LoadedGraphics> &graphics, bool first_display){
	auto zoom = this->get_current_zoom();
	auto &label = this->ui->label;
	label->set_image(*graphics);
	label->set_zoom(zoom);
	auto size = label->get_size();
	int mindim = std::min(size.width(), size.height());
	this->window_state->set_border_size(
		(std::uint32_t)mindim < this->window_state->default_border_size * 3 ?
		mindim / 3 :
		this->window_state->default_border_size
	);
	if (size.width() <= 0 || size.height() <= 0){
		//Too far.
		this->reset_zoom_slot();
		return;
	}
	label->resize(size);

	if (!this->color_calculated && this->displayed_image->has_alpha()){
		this->set_background(true);
		this->color_calculated = true;
	}

	this->reposition_window();
	this->set_background_sizes();

	if (first_display && this->app->get_center_when_displayed()){
		auto label_size = label->size();
		auto window_size = this->size();
		auto new_label_position = (window_size - label_size) / 2;
		this->move_image(to_QPoint(new_label_position));
	}
}

void MainWindow::show_rotate_dialog(){
	RotateDialog dialog(*this);
	dialog.exec();
}

void MainWindow::show_context_menu(QMouseEvent *ev){
	this->app->postEvent(this, new QContextMenuEvent(QContextMenuEvent::Other, ev->screenPos().toPoint()));
}

void MainWindow::build_context_menu(QMenu &main_menu){
	main_menu.addAction("Transform...", this, SLOT(show_rotate_dialog()));
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

void MainWindow::resolution_change(QScreen &screen, const QRect &resolution){
	this->set_desktop_size(screen);
	if (&screen != this->screen())
		return;
	this->fix_positions_and_zoom();
}

void MainWindow::resolution_to_window_size(){
	this->setGeometry(this->screen_sizes[this->current_desktop]);
}

void MainWindow::fix_positions_and_zoom(bool do_not_enlarge){
	if (this->current_zoom_mode_is_auto()){
		auto zoom = this->get_current_zoom();
		this->set_zoom();
		this->apply_zoom(false, zoom);
	}
	this->reposition_window(do_not_enlarge);
	this->reposition_image();
	this->ui->label->repaint();
}

void MainWindow::work_area_change(QScreen &screen, const QRect &resolution){
	this->set_desktop_size(screen);
	if (&screen != this->screen())
		return;
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
	this->set_current_desktop_and_fix_positions_by_window_position(this->current_desktop);
}

void MainWindow::set_window_rect(const QRect &r){
	this->window_rect = r;
	if (!this->window_state->get_fullscreen())
		this->setGeometry(r);
	this->set_current_desktop_and_fix_positions_by_window_position(this->current_desktop);
}

void MainWindow::label_transform_updated(){
	this->set_background_sizes();
}

void MainWindow::transparent_background(){
	this->app->turn_transparent(*this, true);
	this->close();
}

QTransform MainWindow::get_image_transform() const{
	return this->ui->label->get_transform();
}

double MainWindow::set_image_transform(const QTransform &m){
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

QImage MainWindow::get_image() const{
	return this->displayed_image->get_QImage();
}

void TransparentMainWindow::set_background(bool force){
}

void TransparentMainWindow::transparent_background(){
	this->app->turn_transparent(*this, false);
	this->close();
}
