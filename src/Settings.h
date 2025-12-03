#ifdef _MSC_VER
#pragma once
#endif

#ifndef SETTINGS_GENERATED_H
#define SETTINGS_GENERATED_H

#include "Enums.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <QString>
#include <QPoint>
#include <QSize>
#include <QTransform>

class QJsonObject;
class QJsonValue;
class QJsonValueRef;

class Serializable{
public:
	virtual ~Serializable(){}
	virtual QJsonValue serialize() const = 0;
};

#define DEFINE_INLINE_GETTER(x) const decltype(x) &get_##x() const{ return this->x; } 
#define DEFINE_INLINE_NONCONST_GETTER(x) decltype(x) &get_##x(){ return this->x; } 
#define DEFINE_INLINE_SETTER(x) void set_##x(const decltype(x) &v){ this->x = v; }
#define DEFINE_ENUM_INLINE_GETTER(t, x) t get_##x() const{ return (t)this->x; } 
#define DEFINE_ENUM_INLINE_SETTER(t, x) void set_##x(const t &v){ this->x = (decltype(this->x))v; }
#define DEFINE_INLINE_SETTER_GETTER(x) DEFINE_INLINE_GETTER(x) DEFINE_INLINE_SETTER(x)
#define DEFINE_ENUM_INLINE_SETTER_GETTER(t, x) DEFINE_ENUM_INLINE_GETTER(t, x) DEFINE_ENUM_INLINE_SETTER(t, x)

class WindowPosition : public Serializable{
protected:
	QPoint pos;
	QSize size;
	QPoint label_pos;
	QTransform transform;
	virtual QJsonObject internal_serialize() const;
public:
	WindowPosition() = default;
	WindowPosition(const QJsonValueRef &);
	WindowPosition(const QJsonObject &);
	WindowPosition(const WindowPosition &) = default;
	WindowPosition &operator=(const WindowPosition &) = default;
	QJsonValue serialize() const override;
	DEFINE_INLINE_SETTER_GETTER(pos)
	DEFINE_INLINE_SETTER_GETTER(size)
	DEFINE_INLINE_SETTER_GETTER(label_pos)
	DEFINE_INLINE_SETTER_GETTER(transform)
};

class PreferredWindowPosition : public WindowPosition{
protected:
	double zoom = 1;

	QJsonObject internal_serialize() const override;
public:
	PreferredWindowPosition() = default;
	PreferredWindowPosition(const WindowPosition &other): WindowPosition(other){}
	PreferredWindowPosition(const QJsonValueRef &);
	PreferredWindowPosition(const QJsonObject &);
	PreferredWindowPosition(const PreferredWindowPosition &) = default;
	PreferredWindowPosition &operator=(const PreferredWindowPosition &) = default;
	DEFINE_INLINE_SETTER_GETTER(zoom)
};

class WindowState : public Serializable{
	bool using_checkerboard_pattern;
	bool file_is_url = false;
	QString current_directory;
	QString current_filename;
	QString current_url;
	double zoom = 1;
	double fullscreen_zoom = 1;
	bool fullscreen;
	int zoom_mode;
	int fullscreen_zoom_mode;
	int border_size;
	int movement_size;
	WindowPosition position;
	bool using_checkerboard_pattern_updated = false; //Not saved.
	bool loaded_preferred_position = false;
public:
	WindowState();
	WindowState(const QJsonValueRef &);
	DEFINE_INLINE_GETTER(using_checkerboard_pattern)
	DEFINE_INLINE_SETTER_GETTER(file_is_url)
	void set_using_checkerboard_pattern(bool);
	void flip_using_checkerboard_pattern(){
		this->set_using_checkerboard_pattern(!this->using_checkerboard_pattern);
	}
	DEFINE_INLINE_SETTER_GETTER(using_checkerboard_pattern_updated)
	DEFINE_INLINE_SETTER_GETTER(current_directory)
	DEFINE_INLINE_SETTER_GETTER(current_filename)
	DEFINE_INLINE_SETTER_GETTER(current_url)
	DEFINE_INLINE_SETTER_GETTER(zoom)
	DEFINE_INLINE_SETTER_GETTER(fullscreen_zoom)
	DEFINE_INLINE_SETTER_GETTER(fullscreen)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, zoom_mode)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, fullscreen_zoom_mode)
	DEFINE_INLINE_SETTER_GETTER(movement_size)
	DEFINE_INLINE_SETTER_GETTER(border_size)
	static const decltype(border_size) default_border_size = 50;
	static const decltype(border_size) max_border_size = default_border_size;
	static const decltype(border_size) min_border_size = max_border_size / 10;
	void reset_border_size(){
		this->border_size = default_border_size;
	}
	void set_to_preferred_position(const PreferredWindowPosition &pos);
	void set_pos(const QPoint &pos);
	void set_size(const QSize &size);
	void set_label_pos(const QPoint &label_pos);
	void set_transform(const QTransform &transform);
	QPoint get_pos() const;
	QSize get_size() const;
	QPoint get_label_pos() const;
	QTransform get_transform() const;
	PreferredWindowPosition get_preferred_position() const;

	void reset_loaded_preferred_position(){
		this->loaded_preferred_position = false;
	}
	auto get_loaded_preferred_position() const{
		return this->loaded_preferred_position;
	}

	QJsonValue serialize() const override;
	QString get_path() const;
};

class MainSettings : public Serializable{
	int clamp_strength;
	bool clamp_to_edges;
	bool use_checkerboard_pattern;
	bool center_when_displayed;
	int zoom_mode_for_new_windows;
	int fullscreen_zoom_mode_for_new_windows;
	bool keep_application_in_background;
	bool save_state_on_exit;
	bool resize_windows_on_monitor_change = true;

public:
	MainSettings();
	MainSettings(const QJsonValueRef &);
	DEFINE_INLINE_SETTER_GETTER(clamp_strength)
	DEFINE_INLINE_SETTER_GETTER(clamp_to_edges)
	DEFINE_INLINE_SETTER_GETTER(use_checkerboard_pattern)
	DEFINE_INLINE_SETTER_GETTER(center_when_displayed)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, zoom_mode_for_new_windows)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, fullscreen_zoom_mode_for_new_windows)
	DEFINE_INLINE_SETTER_GETTER(keep_application_in_background)
	DEFINE_INLINE_SETTER_GETTER(save_state_on_exit)
	DEFINE_INLINE_SETTER_GETTER(resize_windows_on_monitor_change)
	bool operator==(const MainSettings &other) const;
	bool operator!=(const MainSettings &other) const{
		return !(*this == other);
	}
	QJsonValue serialize() const override;
};

class ApplicationState : public Serializable{
	std::vector<std::shared_ptr<WindowState>> windows;
	std::vector<std::shared_ptr<WindowState>> temporary_failures;
public:
	ApplicationState() = default;
	ApplicationState(const QJsonValueRef &);
	void reset_failures();
	DEFINE_INLINE_GETTER(windows)
	DEFINE_INLINE_NONCONST_GETTER(windows)
	DEFINE_INLINE_GETTER(temporary_failures)
	DEFINE_INLINE_NONCONST_GETTER(temporary_failures)
	QJsonValue serialize() const override;
};

class StateFile : public Serializable{
public:
	std::shared_ptr<ApplicationState> state;
	StateFile() = default;
	StateFile(const QJsonValueRef &json);
	StateFile(QJsonObject &&);
	QJsonValue serialize() const override;
};

class Shortcuts : public Serializable{
public:
	std::map<QString, std::vector<QString>> shortcuts;

	Shortcuts() = default;
	Shortcuts(const QJsonValueRef &);
	void initialize_to_defaults();
	QJsonValue serialize() const override;
};

class Settings : public Serializable{
public:
	std::shared_ptr<MainSettings> main;
	std::shared_ptr<Shortcuts> shortcuts;

	Settings() = default;
	Settings(const QJsonValueRef &json);
	Settings(QJsonObject &&);
	QJsonValue serialize() const override;
};


#endif
