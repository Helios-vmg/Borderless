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
#include <QMatrix>

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

class WindowState : public Serializable{
	QPoint pos;
	QSize size;
	QPoint label_pos;
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
	QMatrix transform;
	bool using_checkerboard_pattern_updated = false; //Not saved.
public:
	WindowState();
	WindowState(const QJsonValueRef &);
	DEFINE_INLINE_SETTER_GETTER(pos)
	DEFINE_INLINE_SETTER_GETTER(size)
	DEFINE_INLINE_SETTER_GETTER(label_pos)
	DEFINE_INLINE_GETTER(using_checkerboard_pattern)
	void set_using_checkerboard_pattern(bool);
	void flip_using_checkerboard_pattern(){
		this->set_using_checkerboard_pattern(!this->using_checkerboard_pattern);
	}
	DEFINE_INLINE_SETTER_GETTER(using_checkerboard_pattern_updated)
	DEFINE_INLINE_SETTER_GETTER(current_directory)
	DEFINE_INLINE_SETTER_GETTER(current_filename)
	DEFINE_INLINE_SETTER_GETTER(zoom)
	DEFINE_INLINE_SETTER_GETTER(fullscreen_zoom)
	DEFINE_INLINE_SETTER_GETTER(fullscreen)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, zoom_mode)
	DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, fullscreen_zoom_mode)
	DEFINE_INLINE_SETTER_GETTER(movement_size)
	DEFINE_INLINE_SETTER_GETTER(transform)
	DEFINE_INLINE_SETTER_GETTER(border_size)
	static const decltype(border_size) default_border_size = 50;
	void reset_border_size(){
		this->border_size = default_border_size;
	}

	QJsonValue serialize() const override;
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
public:
	ApplicationState() = default;
	ApplicationState(const QJsonValueRef &);
	DEFINE_INLINE_GETTER(windows)
	DEFINE_INLINE_NONCONST_GETTER(windows)
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
	std::shared_ptr<ApplicationState> state;
	std::shared_ptr<Shortcuts> shortcuts;

	Settings() = default;
	Settings(const QJsonValueRef &json);
	Settings(QJsonObject &&);
	QJsonValue serialize() const override;
};


#endif
