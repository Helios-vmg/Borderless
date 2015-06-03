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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "LoadedImage.h"
#include "DirectoryListing.h"
#include "ImageViewerApplication.h"
#include "Settings.h"
#include "Rational.h"
#include <QStringList>
#include <QShortcut>
#include <vector>
#include <memory>

namespace Ui {
class MainWindow;
}

class SingleInstanceApplication;

class MainWindow : public QMainWindow{
	Q_OBJECT

	std::shared_ptr<Ui::MainWindow> ui;
	ImageViewerApplication *app;
	QRect desktop_size,
		screen_size,
		window_rect;
	QPoint first_mouse_pos,
		first_window_pos,
		first_label_pos,
		image_pos;
	QSize first_window_size;
	std::shared_ptr<LoadedImage> displayed_image;
	std::vector<int> horizontal_clampers,
		vertical_clampers;
	QString current_directory,
		current_filename;
	std::shared_ptr<DirectoryIterator> directory_iterator;
	bool moving_forward;
	std::vector<std::shared_ptr<QShortcut> > shortcuts;
	bool not_moved;
	double zoom,
		fullscreen_zoom;
	bool color_calculated;
	bool fullscreen;
	std::vector<QMetaObject::Connection> connections;

	enum class ResizeMode{
		None        = 0,
		Top         = 1 << 1,
		Right       = 1 << 2,
		Bottom      = 1 << 3,
		Left        = 1 << 4,
		TopLeft     = ResizeMode::Top | ResizeMode::Left,
		TopRight    = ResizeMode::Top | ResizeMode::Right,
		BottomRight = ResizeMode::Bottom | ResizeMode::Right,
		BottomLeft  = ResizeMode::Bottom | ResizeMode::Left,
	};
	ResizeMode resize_mode;

	DEFINE_SETTING(int, border_size, 50);
	DEFINE_SETTING(int, movement_size, 100);
	DEFINE_SETTING2(ZoomMode, zoom_mode);
	DEFINE_SETTING2(ZoomMode, fullscreen_zoom_mode);
	bool using_checkerboard_pattern;
	bool use_checkerboard_pattern;

	bool move_image(const QPoint &new_position);
	QPoint compute_movement(const QPoint &new_position);
	void compute_resize(QPoint &out_label_pos, QRect &out_window_rect, QPoint mouse_offset);
	void move_window(const QPoint &new_position);
	void reset_settings();
	void compute_average_color(QImage img);
	void set_background(bool force = false);
	void show_nothing();
	void set_solid(const QColor &col);
	void set_background_sizes();
	ResizeMode get_resize_mode(const QPoint &pos);
	void set_resize_mode(const QPoint &pos);
	void setup_backgrounds();
	void resize_to_max();
	bool perform_clamping();
	bool force_keep_window_in_desktop();
	void cleanup();
	void move_in_direction(bool forward);
	void advance();
	void init();
	void setup_shortcut(const QKeySequence &sequence, const char *slot);
	void show_context_menu(QMouseEvent *);
	void change_zoom(bool in);
	void apply_zoom(const double & = 1);
	void offset_image(const QPoint &);
	void set_desktop_size(int screen = -1);
	void set_iterator();
	double &get_current_zoom();
	ZoomMode &get_current_zoom_mode();
	const ZoomMode &get_current_zoom_mode() const;
	void resolution_to_window_size();
	void reposition_window();
	void reposition_image();
	QPoint get_image_pos() const;
	void set_image_pos(const QPoint &);
	bool current_zoom_mode_is_auto() const{
		return check_flag(this->get_current_zoom_mode(), ZoomMode::Automatic);
	}
	void rotate(bool right, bool fine = false);
	void fix_positions_and_zoom();

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	//void keyPressEvent(QKeyEvent *ev);
	void keyReleaseEvent(QKeyEvent *ev);
	void resizeEvent(QResizeEvent *ev);
	void changeEvent(QEvent *ev);
	void closeEvent(QCloseEvent * event);
	void display_image(const QPixmap &);

public:
	explicit MainWindow(ImageViewerApplication &app, const QStringList &arguments, QWidget *parent = 0);
	explicit MainWindow(ImageViewerApplication &app, const SettingsTree &tree, QWidget *parent = 0);
	~MainWindow();
	void display_image(QString path);
	std::shared_ptr<SettingsTree> save_state() const;
	void restore_state(const SettingsTree &tree);
	bool is_null() const{
		return !this->displayed_image || this->displayed_image->is_null();
	}
	void resolution_change(int screen);
	void work_area_change(int screen);
	void resize_window_rect(const QSize &);
	void move_window_rect(const QPoint &);
	void set_window_rect(const QRect &);
	QMatrix get_image_transform() const;
	void set_image_transform(const QMatrix &);
	void setup_shortcuts();
	void build_context_menu(QMenu &);

public slots:
	void label_transform_updated();

	void quit_slot();
	void quit2_slot();
	void next_slot();
	void back_slot();
	void background_swap_slot();
	void close_slot();
	void zoom_in_slot();
	void zoom_out_slot();
	void reset_zoom_slot();
	void up_slot();
	void down_slot();
	void left_slot();
	void right_slot();
	void up_big_slot();
	void down_big_slot();
	void left_big_slot();
	void right_big_slot();
	void cycle_zoom_mode_slot();
	void set_zoom();
	void toggle_lock_zoom_slot();
	void go_to_start();
	void go_to_end();
	void toggle_fullscreen();
	void rotate_left();
	void rotate_right();
	void rotate_left_fine();
	void rotate_right_fine();
	void minimize_slot();
	void minimize_all_slot();
	void flip_h();
	void flip_v();
	void show_rotate_dialog();
	void show_options_dialog();

signals:
	void closing(MainWindow *);

};

#endif // MAINWINDOW_H
