/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QScreen>
#include "LoadedImage.h"
#include "DirectoryListing.h"
#include "ImageViewerApplication.h"
#include <QStringList>
#include <QShortcut>
#include <vector>
#include <memory>
#include "Misc.h"
#include "Settings.h"

namespace Ui {
class MainWindow;
}

class MouseEvent{
public:
	bool left;
	bool right;
	bool middle;
	int button_sum;
	QPoint absolute;
	QPoint relative;
	MouseEvent(const QMouseEvent &);
	MouseEvent(const MouseEvent &) = default;
	MouseEvent &operator=(const MouseEvent &) = default;
};

class MainWindow : public QMainWindow{
	Q_OBJECT

protected:
	std::shared_ptr<Ui::MainWindow> ui;
	ImageViewerApplication *app;
	std::map<std::string, QRect> desktop_sizes,
		screen_sizes;
	std::string current_desktop;
	QRect window_rect;
	QPoint first_mouse_pos,
		first_window_pos,
		first_label_pos;
	Optional<QPoint> image_pos;
	QSize first_window_size;
	std::shared_ptr<LoadedGraphics> displayed_image;
	std::vector<int> horizontal_clampers,
		vertical_clampers;
	//QString current_directory,
	//	current_filename;
	std::shared_ptr<DirectoryIterator> directory_iterator;
	bool moving_forward;
	std::vector<std::shared_ptr<QShortcut> > shortcuts;
	bool not_moved;
	bool color_calculated;
	std::vector<QMetaObject::Connection> connections;
	bool last_set_by_user = true;

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

	std::shared_ptr<WindowState> window_state;

	bool move_image(const QPoint &new_position);
	QPoint compute_movement(const QPoint &new_position, const QPoint &mouse_position);
	bool compute_resize(QPoint &out_label_pos, QRect &out_window_rect, QPoint mouse_offset, const QPoint &mouse_position);
	void move_window(const QPoint &new_position, const QPoint &mouse_position);
	void reset_settings();
	void compute_average_color(QImage img);
	virtual void set_background(bool force = false);
	void show_nothing();
	void set_solid(const QColor &col);
	void set_background_sizes();
	ResizeMode get_resize_mode(const QPoint &pos);
	void set_resize_mode(const QPoint &pos);
	void setup_backgrounds();
	void resize_to_max(bool do_not_enlarge = false);
	bool perform_clamping();
	bool force_keep_window_in_desktop();
	void cleanup();
	void move_in_direction(bool forward);
	void advance();
	void init(bool restoring);
	void setup_shortcut(const QKeySequence &sequence, const char *slot);
	void show_context_menu(QMouseEvent *);
	void change_zoom(bool in);
	void apply_zoom(bool, double);
	void offset_image(const QPoint &);
	void set_current_desktop_and_fix_positions_by_window_position(std::string old_desktop);
	void set_desktop_size();
	void set_desktop_size(QScreen &);
	void set_iterator();
	double get_current_zoom() const;
	void set_current_zoom(double);
	void set_current_zoom_mode(const ZoomMode &);
	ZoomMode get_current_zoom_mode() const;
	void resolution_to_window_size();
	void reposition_window(bool do_not_enlarge = false);
	void reposition_image();
	void save_image_pos(bool force = false);
	void restore_image_pos();
	void clear_image_pos();
	void rotate(bool right, bool fine = false);
	void fix_positions_and_zoom(bool do_not_enlarge = false);

	struct ZoomResult{
		double zoom;
		QSize label_size;
	};
	
	ZoomResult compute_zoom(int override_rotation = -1);

protected:
	void mousePressEvent(QMouseEvent *ev) override;
	void mouseReleaseEvent(QMouseEvent *ev) override;
	void mouseMoveEvent(QMouseEvent *ev) override;
	void reset_left_mouse(const MouseEvent &);
	bool set_cursor_flags(const MouseEvent &);
	//void keyPressEvent(QKeyEvent *ev) override;
	//void keyReleaseEvent(QKeyEvent *ev) override;
	void resizeEvent(QResizeEvent *ev) override;
	void changeEvent(QEvent *ev) override;
	void closeEvent(QCloseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *) override;
	//bool event(QEvent *) override;

public:
	explicit MainWindow(ImageViewerApplication &app, const QStringList &arguments, QWidget *parent = 0);
	explicit MainWindow(ImageViewerApplication &app, const std::shared_ptr<WindowState> &state, QWidget *parent = 0);
	virtual ~MainWindow();
	bool open_path_and_display_image(QString path);
	void display_image_in_label(const std::shared_ptr<LoadedGraphics> &graphics, bool first_display);
	void display_filtered_image(const std::shared_ptr<LoadedGraphics> &);
	std::shared_ptr<WindowState> save_state() const;
	void restore_state(const std::shared_ptr<WindowState> &);
	bool is_null() const{
		return !this->displayed_image || this->displayed_image->is_null();
	}
	void resolution_change(QScreen &screen);
	void work_area_change(QScreen &screen);
	void resize_window_rect(const QSize &);
	void move_window_rect(const QPoint &);
	void set_window_rect(const QRect &);
	QTransform get_image_transform() const;
	double get_image_zoom() const;
	void set_image_zoom(double);
	double set_image_transform(const QTransform &);
	void setup_shortcuts();
	void build_context_menu(QMenu &main_menu);
	bool current_zoom_mode_is_auto() const{
		return check_flag(this->get_current_zoom_mode(), ZoomMode::AutomaticZoom);
	}
	bool current_zoom_mode_is_auto_rotation() const{
		return check_flag(this->get_current_zoom_mode(), ZoomMode::AutomaticRotation);
	}
	QImage get_image() const;
	ImageViewerApplication &get_app(){
		return *this->app;
	}
	bool is_loaded() const{
		return !!this->displayed_image;
	}

public slots:
	void label_transform_updated();
	virtual void transparent_background();

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

class TransparentMainWindow : public MainWindow{
protected:
	void set_background(bool force = false) override;
public:
	explicit TransparentMainWindow(ImageViewerApplication &app, const std::shared_ptr<WindowState> &state, QWidget *parent = 0);
	void transparent_background() override;
};

#endif // MAINWINDOW_H
