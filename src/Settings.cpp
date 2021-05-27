#include "Settings.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValueRef>
#include <QJsonValue>

#define DEFINE_JSON_STRING(name) const char * const json_string_##name = #name
#define READ_JSON(dst, src) parse_json(this->dst, src, json_string_##dst)
#define READ_JSON_DEFAULT(dst, src, def) parse_json(this->dst, src, json_string_##dst, def)
#define WRITE_JSON(src, dst) set_value(dst[json_string_##src], this->src)

DEFINE_JSON_STRING(main);
DEFINE_JSON_STRING(state);
DEFINE_JSON_STRING(shortcuts);
DEFINE_JSON_STRING(clamp_strength);
DEFINE_JSON_STRING(clamp_to_edges);
DEFINE_JSON_STRING(use_checkerboard_pattern);
DEFINE_JSON_STRING(center_when_displayed);
DEFINE_JSON_STRING(zoom_mode_for_new_windows);
DEFINE_JSON_STRING(fullscreen_zoom_mode_for_new_windows);
DEFINE_JSON_STRING(keep_application_in_background);
DEFINE_JSON_STRING(save_state_on_exit);
DEFINE_JSON_STRING(pos);
DEFINE_JSON_STRING(size);
DEFINE_JSON_STRING(label_pos);
DEFINE_JSON_STRING(using_checkerboard_pattern);
DEFINE_JSON_STRING(file_is_url);
DEFINE_JSON_STRING(current_directory);
DEFINE_JSON_STRING(current_filename);
DEFINE_JSON_STRING(current_url);
DEFINE_JSON_STRING(zoom);
DEFINE_JSON_STRING(fullscreen_zoom);
DEFINE_JSON_STRING(fullscreen);
DEFINE_JSON_STRING(zoom_mode);
DEFINE_JSON_STRING(fullscreen_zoom_mode);
DEFINE_JSON_STRING(border_size);
DEFINE_JSON_STRING(movement_size);
DEFINE_JSON_STRING(transform);
DEFINE_JSON_STRING(x);
DEFINE_JSON_STRING(y);
DEFINE_JSON_STRING(w);
DEFINE_JSON_STRING(h);
DEFINE_JSON_STRING(resize_windows_on_monitor_change);

template <typename T>
struct json_cast{
	static T f(const QJsonValueRef &src){
		return T(src);
	}
};

template <>
struct json_cast<int>{
	static int f(const QJsonValueRef &src){
		return src.toInt();
	}
};

template <>
struct json_cast<double>{
	static double f(const QJsonValueRef &src){
		return src.toDouble();
	}
};

template <>
struct json_cast<bool>{
	static bool f(const QJsonValueRef &src){
		return src.toBool();
	}
};

template <>
struct json_cast<QString>{
	static QString f(const QJsonValueRef &src){
		return src.toString();
	}
};

template <typename DstT>
void parse_json(DstT &dst, QJsonObject &json, const char *name, const DstT &default_value = {}){
	auto it = json.find(name);
	if (it != json.end())
		dst = json_cast<DstT>::f(it.value());
	else
		dst = default_value;
}

template <typename DstT>
void parse_json(std::shared_ptr<DstT> &dst, QJsonObject &json, const char *name){
	auto it = json.find(name);
	if (it == json.end())
		return;
	dst.reset(new DstT(it.value()));
}

void parse_json(QPoint &dst, QJsonObject &json, const char *name, const QPoint &default_value = {}){
	auto it = json.find(name);
	dst = QPoint();
	if (it == json.end()){
		dst = default_value;
		return;
	}
	auto val = it.value();
	if (!val.isObject()){
		dst = default_value;
		return;
	}
	auto obj = val.toObject();
	dst.setX(obj[json_string_x].toInt());
	dst.setY(obj[json_string_y].toInt());
}

void parse_json(QSize &dst, QJsonObject &json, const char *name, const QSize &default_value = {}){
	auto it = json.find(name);
	dst = QSize();
	if (it == json.end()){
		dst = default_value;
		return;
	}
	auto val = it.value();
	if (!val.isObject()){
		dst = default_value;
		return;
	}
	auto obj = val.toObject();
	dst.setWidth(obj[json_string_w].toInt());
	dst.setHeight(obj[json_string_h].toInt());
}

void parse_json(QMatrix &dst, QJsonObject &json, const char *name, const QMatrix &default_value = {}){
	auto it = json.find(name);
	dst = QMatrix();
	if (it == json.end()){
		dst = default_value;
		return;
	}
	auto val = it.value();
	if (!val.isArray()){
		dst = default_value;
		return;
	}
	auto array = val.toArray();
	double m[4];
	for (int i = 0; i < 4; i++)
		m[i] = array[i].toDouble();
	dst = QMatrix(m[0], m[2], m[1], m[3], 0, 0);
}

template <typename T>
void set_value(QJsonValueRef &&dst, const T &src){
	dst = src;
}

void set_value(QJsonValueRef &&dst, const QPoint &src){
	QJsonObject temp;
	set_value(temp[json_string_x], src.x());
	set_value(temp[json_string_y], src.y());
	dst = temp;
}

void set_value(QJsonValueRef &&dst, const QSize &src){
	QJsonObject temp;
	set_value(temp[json_string_w], src.width());
	set_value(temp[json_string_h], src.height());
	dst = temp;
}

void set_value(QJsonValueRef &&dst, const QMatrix &src){
	QJsonArray temp;
	temp.push_back(src.m11());
	temp.push_back(src.m21());
	temp.push_back(src.m12());
	temp.push_back(src.m22());
	dst = temp;
}

StateFile::StateFile(const QJsonValueRef &json): StateFile(json.toObject()){}

StateFile::StateFile(QJsonObject &&object){
	READ_JSON(state, object);
}

QJsonValue StateFile::serialize() const{
	QJsonObject ret;
	if (this->state)
		ret[json_string_state] = this->state->serialize();
	return ret;
}

Settings::Settings(const QJsonValueRef &json): Settings(json.toObject()){}

Settings::Settings(QJsonObject &&object){
	READ_JSON(main, object);
	READ_JSON(shortcuts, object);
}

QJsonValue Settings::serialize() const{
	QJsonObject ret;
	if (this->main)
		ret[json_string_main] = this->main->serialize();
	if (this->shortcuts)
		ret[json_string_shortcuts] = this->shortcuts->serialize();
	return ret;
}

Shortcuts::Shortcuts(const QJsonValueRef &json){
	auto object = json.toObject();
	for (auto &key : object.keys()){
		auto val = object[key];
		if (!val.isArray())
			continue;
		auto &v = this->shortcuts[key];
		auto array = val.toArray();
		v.reserve(array.size());
		for (const auto &i : array)
			v.emplace_back(i.toString());
	}
}

QJsonValue Shortcuts::serialize() const{
	QJsonObject ret;
	for (auto &kv : this->shortcuts){
		QJsonArray array;
		for (auto &i : kv.second)
			array.push_back(i);
		ret[kv.first] = std::move(array);
	}
	return ret;
}

ApplicationState::ApplicationState(const QJsonValueRef &json){
	for (const auto &val : json.toArray())
		this->windows.emplace_back(std::make_shared<WindowState>(val));
}

QJsonValue ApplicationState::serialize() const{
	QJsonArray ret;
	for (auto &w : this->windows)
		ret.push_back(w->serialize());
	return ret;
}

MainSettings::MainSettings(const QJsonValueRef &json){
	auto object = json.toObject();
	READ_JSON(clamp_strength, object);
	READ_JSON(clamp_to_edges, object);
	READ_JSON(use_checkerboard_pattern, object);
	READ_JSON(center_when_displayed, object);
	READ_JSON(zoom_mode_for_new_windows, object);
	READ_JSON(fullscreen_zoom_mode_for_new_windows, object);
	READ_JSON(keep_application_in_background, object);
	READ_JSON(save_state_on_exit, object);
	READ_JSON_DEFAULT(resize_windows_on_monitor_change, object, true);
}

QJsonValue MainSettings::serialize() const{
	QJsonObject object;
	WRITE_JSON(clamp_strength, object);
	WRITE_JSON(clamp_to_edges, object);
	WRITE_JSON(use_checkerboard_pattern, object);
	WRITE_JSON(center_when_displayed, object);
	WRITE_JSON(zoom_mode_for_new_windows, object);
	WRITE_JSON(fullscreen_zoom_mode_for_new_windows, object);
	WRITE_JSON(keep_application_in_background, object);
	WRITE_JSON(save_state_on_exit, object);
	WRITE_JSON(resize_windows_on_monitor_change, object);
	return object;
}

WindowState::WindowState(const QJsonValueRef &json){
	auto object = json.toObject();
	READ_JSON(pos, object);
	READ_JSON(size, object);
	READ_JSON(label_pos, object);
	READ_JSON(using_checkerboard_pattern, object);
	READ_JSON(file_is_url, object);
	READ_JSON(current_directory, object);
	READ_JSON(current_filename, object);
	READ_JSON(current_url, object);
	READ_JSON(zoom, object);
	READ_JSON(fullscreen_zoom, object);
	READ_JSON(fullscreen, object);
	READ_JSON(zoom_mode, object);
	READ_JSON(fullscreen_zoom_mode, object);
	READ_JSON(border_size, object);
	READ_JSON(movement_size, object);
	READ_JSON(transform, object);
}

QJsonValue WindowState::serialize() const{
	QJsonObject object;
	WRITE_JSON(pos, object);
	WRITE_JSON(size, object);
	WRITE_JSON(label_pos, object);
	WRITE_JSON(using_checkerboard_pattern, object);
	WRITE_JSON(file_is_url, object);
	WRITE_JSON(current_directory, object);
	WRITE_JSON(current_filename, object);
	WRITE_JSON(current_url, object);
	WRITE_JSON(zoom, object);
	WRITE_JSON(fullscreen_zoom, object);
	WRITE_JSON(fullscreen, object);
	WRITE_JSON(zoom_mode, object);
	WRITE_JSON(fullscreen_zoom_mode, object);
	WRITE_JSON(border_size, object);
	WRITE_JSON(movement_size, object);
	WRITE_JSON(transform, object);
	return object;
}

void WindowState::set_using_checkerboard_pattern(bool b){
	this->using_checkerboard_pattern = b;
	this->using_checkerboard_pattern_updated = true;
}

WindowState::WindowState(){
	this->using_checkerboard_pattern = true;
	this->zoom = 1;
	this->fullscreen = false;
	this->zoom_mode = (int)ZoomMode::Normal;
	this->fullscreen_zoom_mode = (int)ZoomMode::AutoFit;
	this->border_size = this->default_border_size;
	this->movement_size = 100;
}

MainSettings::MainSettings(){
	this->clamp_strength = 25;
	this->clamp_to_edges = true;
	this->use_checkerboard_pattern = true;
	this->center_when_displayed = true;
	this->keep_application_in_background = true;
	this->set_zoom_mode_for_new_windows(ZoomMode::Normal);
	this->set_fullscreen_zoom_mode_for_new_windows(ZoomMode::AutoFit);
	this->set_save_state_on_exit(true);
}

bool MainSettings::operator==(const MainSettings &other) const{
#define CHECK_EQUALITY(x) if (this->x != other.x) return false
	CHECK_EQUALITY(clamp_strength);
	CHECK_EQUALITY(clamp_to_edges);
	CHECK_EQUALITY(use_checkerboard_pattern);
	CHECK_EQUALITY(center_when_displayed);
	CHECK_EQUALITY(zoom_mode_for_new_windows);
	CHECK_EQUALITY(fullscreen_zoom_mode_for_new_windows);
	CHECK_EQUALITY(keep_application_in_background);
	CHECK_EQUALITY(save_state_on_exit);
	CHECK_EQUALITY(resize_windows_on_monitor_change);
	return true;
}
