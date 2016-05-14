/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "lua.h"
#include "ImageStore.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include <QMessageBox>

#define MINIMIZE_CHECKING

double euclidean_modulo(double x, double y){
	if (x < 0)
		x = y - fmod(-x, y);
	if (x >= y)
		x = fmod(x, y);
	return x;
}

bool get_lua_global_function(lua_State *state, const char *name){
	lua_getglobal(state, name);
	if (!lua_isfunction(state, -1)){
		lua_pop(state, 1);
		return 0;
	}
	return 1;
}

void handle_call_to_c_error(lua_State *state, const char *function, const char *msg){
	lua_Debug debug;
	lua_getstack(state, 1, &debug);
	lua_getinfo(state, "nSl", &debug);
	int line = debug.currentline;
	std::stringstream stream;
	stream << "ERROR at line " << line << " calling function " << function << "(): " << msg;
	QMessageBox msgbox;
	msgbox.setText(QString::fromStdString(stream.str()));
	msgbox.setIcon(QMessageBox::Critical);
	msgbox.exec();
}

#define DECLARE_LUA_FUNCTION(x) int x(lua_State *state)

DECLARE_LUA_FUNCTION(load_image){
	std::string msg;
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 1)
		msg = "Not enough parameters.";
	else if (!lua_isstring(state, 1))
		msg = "The parameter should be a string.";
#endif
	if (!msg.size()){
		auto res = global_store.load(lua_tostring(state, 1));
		if (res.success){
			lua_pushinteger(state, res.results[0]);
			return 1;
		}
		msg = res.message;
	}

	handle_call_to_c_error(state, __FUNCTION__, msg.c_str());
	lua_pushnil(state);
	lua_pushstring(state, msg.c_str());
	return 2;
}

DECLARE_LUA_FUNCTION(allocate_image){
	std::string msg;
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 2){
		msg = "Not enough parameters.";
	}else if (!lua_isnumber(state, 1) || !lua_isnumber(state, 2)){
		msg = "Both parameters must be integers.";
	}
#endif
	if (!msg.size()){
		int w = (int)lua_tointeger(state, 1);
		int h = (int)lua_tointeger(state, 2);
		if (w <= 0 || h <= 0)
			msg = "Both parameters must be greater than zero.";
		else{
			auto res = global_store.allocate(w, h);
			if (res.success){
				lua_pushinteger(state, res.results[0]);
				return 1;
			}
			msg = res.message;
		}
	}
	handle_call_to_c_error(state, __FUNCTION__, msg.c_str());
	lua_pushnil(state);
	lua_pushstring(state, msg.c_str());
	return 2;
}

DECLARE_LUA_FUNCTION(traverse_image){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 2){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
	if (!lua_isnumber(state, 1) || !lua_isfunction(state, 2)){
		handle_call_to_c_error(state, __FUNCTION__, "Parameters are of incorrect types.");
		return 0;
	}
#endif
	int imgno = (int)lua_tointeger(state, 1);
	global_store.traverse(
		imgno,
		[state](int r, int g, int b, int a, int x, int y){
			lua_pushvalue(state, 2);
			lua_pushinteger(state, r);
			lua_pushinteger(state, g);
			lua_pushinteger(state, b);
			lua_pushinteger(state, a);
			lua_pushinteger(state, x);
			lua_pushinteger(state, y);
			lua_call(state, 6, 0);
		}
	);
	return 0;
}

DECLARE_LUA_FUNCTION(rgb_to_hsv){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 3){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
#endif
	double rgb[3];
	for (int i = 0; i < 3; i++){
#ifndef MINIMIZE_CHECKING
		if (!lua_isnumber(state, i + 1)){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be integers.");
			return 0;
		}
#endif
		rgb[i] = (double)lua_tonumber(state, i + 1);
		if (rgb[i] < 0 || rgb[i] > 255){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be in the range [0; 255].");
			return 0;
		}
		rgb[i] /= 255.0;
	}
	double &r = rgb[0];
	double &g = rgb[1];
	double &b = rgb[2];
	double hue, sat, val, max, min, delta;
	max = std::max(r, std::max(g, b));
	min = std::min(r, std::min(g, b));
	delta = max - min;

	val = max;
	sat = !max ? 0 : delta / max;

	if (!delta || !sat)
		hue = 0;
	else{
		if (val == r){
			hue = euclidean_modulo((g - b) / delta, 6);
		}else if (val == g)
			hue = (b - r) / delta + 2;
		else
			hue = (r - g) / delta + 4;
		hue *= 60;
	}

	lua_pushnumber(state, hue);
	lua_pushnumber(state, sat);
	lua_pushnumber(state, val);
	return 3;
}

DECLARE_LUA_FUNCTION(hsv_to_rgb){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 3){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
#endif
	double hsv[3];
	for (int i = 0; i < 3; i++){
#ifndef MINIMIZE_CHECKING
		if (!lua_isnumber(state, i + 1)){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be numbers.");
			return 0;
		}
#endif
		hsv[i] = lua_tonumber(state, i + 1);
	}

	double &hue = hsv[0],
		&sat = hsv[1],
		&val = hsv[2];

	hue = euclidean_modulo(hue, 360);

	if (sat < 0 || sat > 1){
		handle_call_to_c_error(state, __FUNCTION__, "Saturation should be in the range [0;1].");
		return 0;
	}
	if (val < 0 || val > 1){
		handle_call_to_c_error(state, __FUNCTION__, "Value should be in the range [0;1].");
		return 0;
	}

	double chroma = val * sat,
		c = chroma;
	double x = chroma * (1 - abs(euclidean_modulo(hue / 60, 2) - 1));
	double m = val - chroma;
	double values[][3] = {
		{ c, x, 0 },
		{ x, c, 0 },
		{ 0, c, x },
		{ 0, x, c },
		{ x, 0, c },
		{ c, 0, x },
	};
	int j = (int)(hue / 60);
	assert(j >= 0 && j < 6);
	for (int i = 0; i < 3; i++){
		auto x = (int)((values[j][i] + m) * 255);
		lua_pushnumber(state, x);
	}

	return 3;
}

DECLARE_LUA_FUNCTION(set_current_pixel){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 3){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
#endif
	pixel_t rgba;
	memset(rgba.data(), 0, rgba.size() * sizeof(rgba[0]));
	for (int i = 0; i < 4; i++){
#ifndef MINIMIZE_CHECKING
		if (!lua_isnumber(state, i + 1)){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be integers.");
			return 0;
		}
#endif
		int c = (int)lua_tointeger(state, i + 1);
		if (c < 0 || c > 255){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be in the range [0; 255].");
			return 0;
		}
		rgba[i] = (std::uint8_t)c;
	}
	global_store.set_current_pixel(rgba);
	return 0;
}

void to_lower(std::string &s){
	std::transform(s.begin(), s.end(), s.begin(), tolower);
}

DECLARE_LUA_FUNCTION(save_image){
	std::string msg;
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 2)
		msg = "Not enough parameters.";
	else if (!lua_isnumber(state, 1))
		msg = "The first parameter should be a number.";
	else if (!lua_isstring(state, 2))
		msg = "The second parameter should be a string.";
#endif
	if (!msg.size()){
		int handle = lua_tointeger(state, 1);
		auto path = QString::fromUtf8(lua_tostring(state, 2));
		SaveOptions opt;
		if (lua_gettop(state) >= 3 && lua_istable(state, 3)){
			{
				lua_pushstring(state, "format");
				lua_gettable(state, 3);
				if (lua_isstring(state, -1)){
					std::string s = lua_tostring(state, -1);
					to_lower(s);
					opt.format = s;
				}
				lua_pop(state, 1);
			}
			{
				lua_pushstring(state, "compression");
				lua_gettable(state, 3);
				if (lua_isnumber(state, -1))
					opt.compression = lua_tonumber(state, -1);
				lua_pop(state, 1);
			}
		}
		auto res = global_store.save(handle, path, opt);
		if (res.success){
			lua_pushboolean(state, 1);
			return 1;
		}
		msg = res.message;
	}
	handle_call_to_c_error(state, __FUNCTION__, msg.c_str());
	lua_pushboolean(state, 0);
	return 1;
}

DECLARE_LUA_FUNCTION(bitwise_and){
	auto x = lua_tointeger(state, 1);
	auto y = lua_tointeger(state, 2);
	x &= y;
	lua_pushinteger(state, x);
	return 1;
}

DECLARE_LUA_FUNCTION(bitwise_or){
	auto x = lua_tointeger(state, 1);
	auto y = lua_tointeger(state, 2);
	x |= y;
	lua_pushinteger(state, x);
	return 1;
}

DECLARE_LUA_FUNCTION(bitwise_xor){
	auto x = lua_tointeger(state, 1);
	auto y = lua_tointeger(state, 2);
	x ^= y;
	lua_pushinteger(state, x);
	return 1;
}

DECLARE_LUA_FUNCTION(bitwise_not){
	auto x = lua_tointeger(state, 1);
	x = ~x;
	lua_pushinteger(state, x);
	return 1;
}

DECLARE_LUA_FUNCTION(unload_image){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 1){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
	if (!lua_isnumber(state, 1)){
		handle_call_to_c_error(state, __FUNCTION__, "Parameter should be an integer.");
		return 0;
	}
#endif
	auto res = global_store.unload(lua_tointeger(state, 1));
	if (!res.success)
		handle_call_to_c_error(state, __FUNCTION__, res.message.c_str());
	return 0;
}

DECLARE_LUA_FUNCTION(get_pixel){
#ifndef MINIMIZE_CHECKING
	if (lua_gettop(state) < 3){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
#endif
	int params[3];
	for (int i = 0; i < 3; i++){
#ifndef MINIMIZE_CHECKING
		if (!lua_isnumber(state, i + 1)){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be integers.");
			return 0;
		}
#endif
		params[i] = (int)lua_tointeger(state, i + 1);
		if (i && params[i] < 0){
			handle_call_to_c_error(state, __FUNCTION__, "Coordinates may not be negative.");
			return 0;
		}
	}

	auto res = global_store.get_pixel(params[0], (unsigned)params[1], (unsigned)params[2]);
	if (!res.success){
		handle_call_to_c_error(state, __FUNCTION__, res.message.c_str());
		return 0;
	}
	for (int i = 0; i < 4; i++)
		lua_pushinteger(state, res.results[i]);
	return 4;
}

DECLARE_LUA_FUNCTION(get_image_dimensions){
	if (lua_gettop(state) < 1){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
	if (!lua_isnumber(state, 1)){
		handle_call_to_c_error(state, __FUNCTION__, "The parameter should be an integer.");
		return 0;
	}
	int img = (int)lua_tointeger(state, 1);
	auto res = global_store.get_dimensions(img);

	if (!res.success){
		handle_call_to_c_error(state, __FUNCTION__, res.message.c_str());
		return 0;
	}
	for (int i = 0; i < 2; i++)
		lua_pushinteger(state, res.results[i]);
	return 2;
}

enum class ZigZagState{
	Initial = 0,
	RightwardsOnTop,
	DownLeft,
	DownwardsOnLeft,
	UpRight,
	DownwardsOnRight,
	RightwardsOnBottom,
	End = -1,
};

DECLARE_LUA_FUNCTION(zig_zag_order){
	if (lua_gettop(state) < 5){
		handle_call_to_c_error(state, __FUNCTION__, "Not enough parameters.");
		return 0;
	}
	int params[5];
	for (int i = 0; i < 5; i++){
		if (!lua_isnumber(state, i + 1)){
			handle_call_to_c_error(state, __FUNCTION__, "All parameters should be integers.");
			return 0;
		}
		params[i] = lua_tointeger(state, i + 1);
	}
	int x = params[0],
		y = params[1],
		w = params[2],
		h = params[3];
	ZigZagState s = (ZigZagState)params[4];

#define allleft (!x)
#define allright (x == w - 1)
#define attop (!y)
#define atbottom (y == h - 1)
	if (atbottom && allright)
		s = ZigZagState::End;
	for (int repeat = 1; repeat--; ){
		switch (s){
		case ZigZagState::Initial:
			x = 0;
			y = 0;
			s = ZigZagState::RightwardsOnTop;
			break;
		case ZigZagState::RightwardsOnTop:
			if (allright){
				s = ZigZagState::DownwardsOnRight;
				repeat++;
				break;
			}
			x++;
			s = ZigZagState::DownLeft;
			break;
		case ZigZagState::DownLeft:
			if (atbottom){
				s = ZigZagState::RightwardsOnBottom;
				repeat++;
				break;
			}
			if (allleft){
				s = ZigZagState::DownwardsOnLeft;
				repeat++;
				break;
			}
			x--;
			y++;
			//maintain state
			break;
		case ZigZagState::DownwardsOnLeft:
			if (atbottom){
				s = ZigZagState::RightwardsOnBottom;
				repeat++;
				break;
			}
			y++;
			s = ZigZagState::UpRight;
			break;
		case ZigZagState::UpRight:
			if (allright){
				s = ZigZagState::DownwardsOnRight;
				repeat++;
				break;
			}
			if (attop){
				s = ZigZagState::RightwardsOnTop;
				repeat++;
				break;
			}
			x++;
			y--;
			//maintain state
			break;



		case ZigZagState::DownwardsOnRight:
			if (atbottom){
				s = ZigZagState::End;
				repeat++;
				break;
			}
			y++;
			s = ZigZagState::DownLeft;
			break;
		case ZigZagState::RightwardsOnBottom:
			if (allright){
				s = ZigZagState::End;
				repeat++;
				break;
			}
			x++;
			s = ZigZagState::UpRight;
			break;
		case ZigZagState::End:
			repeat = 0;
			break;
		}
	}
	lua_pushinteger(state, x);
	lua_pushinteger(state, y);
	lua_pushinteger(state, (int)s);
	return 3;
}

lua_State *init_lua_state(){
	lua_State *state = lua_open();
	luaL_openlibs(state);
	luaL_Reg c_functions[] = {
		{ "load_image", load_image },
		{ "allocate_image", allocate_image },
		{ "unload_image", unload_image },
		{ "traverse_image", traverse_image },
		{ "rgb_to_hsv", rgb_to_hsv },
		{ "hsv_to_rgb", hsv_to_rgb },
		{ "set_current_pixel", set_current_pixel },
		{ "save_image", save_image },
		{ "bitwise_and", bitwise_and },
		{ "bitwise_or", bitwise_or },
		{ "bitwise_xor", bitwise_xor },
		{ "bitwise_not", bitwise_not },
		{ "get_pixel", get_pixel },
		{ "get_image_dimensions", get_image_dimensions },
		{ "zig_zag_order", zig_zag_order },
	};
	for (auto &r : c_functions){
		lua_pushcfunction(state, r.func);
		lua_setglobal(state, r.name);
	}

	lua_atpanic(state,
		[](lua_State *state){
			std::stringstream stream;
			stream << "Lua threw an error: " << + lua_tostring(state, -1);
			QMessageBox msgbox;
			msgbox.setText(QString::fromStdString(stream.str()));
			msgbox.setIcon(QMessageBox::Critical);
			msgbox.exec();
			return 0;
		}
	);

	return state;
}

