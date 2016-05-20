/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include "main.h"
#include <string>
#include <array>
#include <memory>
#include <lua.hpp>
#include <functional>

struct ImageOperationResult{
	bool success;
	int results[4];
	std::string message;
	ImageOperationResult(const char *msg = nullptr): success(!msg), message(msg ? msg : ""){}
};

typedef std::array<std::uint8_t, 4> pixel_t;

struct SaveOptions{
	int compression;
	std::string format;
	SaveOptions(): compression(-1){}
};

struct traversal_stack_frame{
	traversal_stack_frame *prev;
	image_info info;
	unsigned char *current_pixel;
};

class LuaInterpreter{
	LuaInterpreterParameters parameters;
	std::function<void(char *)> release_function;
	std::shared_ptr<lua_State> lua_state;
	traversal_stack_frame *frame = nullptr;
public:
	LuaInterpreter(const LuaInterpreterParameters &params);
	~LuaInterpreter();
	LuaCallResult execute_buffer(const char *filename, const void *buffer, size_t size);
	void message_box(const char *title, const char *message, bool is_error);
	ImageOperationResult load_image(const char *path);
	ImageOperationResult unload_image(int handle);
	ImageOperationResult allocate_image(int w, int h);
	ImageOperationResult save_image(int handle, const char *path, const SaveOptions &);
	typedef void (*traverse_callback_t)(void *, int r, int g, int b, int a, int x, int y);
	ImageOperationResult traverse(int handle, traverse_callback_t cb, void *ud);
	void set_current_pixel(const pixel_t &);
	ImageOperationResult get_pixel(int handle, int x, int y);
	ImageOperationResult get_image_dimensions(int handle);
	int get_caller_image();
	ImageOperationResult display_in_current_window(int handle);
	void debug_print(const char *string);
};

class LuaCallResultImpl{
public:
	bool success;
	std::string message;

	LuaCallResultImpl(): success(true){}
	LuaCallResultImpl(const char *message): message(message){}
	LuaCallResultImpl(const std::string &message): message(message){}
};

#endif
