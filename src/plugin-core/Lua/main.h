/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef LUAINTERPRETERMAIN_H
#define LUAINTERPRETERMAIN_H

#ifndef __cplusplus
#error Only use this file from C++!
#endif

#include "../CallResult.h"

#define LUA_PLUGIN_EXTERN_C extern "C"

#if defined WIN32 && defined BUILDING_LUAINTERPRETER
#define LUA_PLUGIN_EXPORT_C LUA_PLUGIN_EXTERN_C __declspec(dllexport)
#else
#define LUA_PLUGIN_EXPORT_C LUA_PLUGIN_EXTERN_C
#endif

class LuaInterpreter;
typedef void *external_state;

struct ImageOperationResultExternal{
	bool success = true;
	int results[4];
	char *message = nullptr;
};

struct image_info{
	int handle;
	int w, h, stride, pitch;
	void *pixels;
};

struct LuaInterpreterParameters{
	external_state state;

#define LuaInterpreterParameters_DECLARE_FUNCTION(rt, x, ...) \
	typedef rt (*x##_f)(external_state, __VA_ARGS__); \
	x##_f x
#define LuaInterpreterParameters_DECLARE_FUNCTION0(rt, x) \
	typedef rt (*x##_f)(external_state); \
	x##_f x
	LuaInterpreterParameters_DECLARE_FUNCTION(void, release_returned_string, char *);
	LuaInterpreterParameters_DECLARE_FUNCTION(void, show_message_box, const char *title, const char *message, bool is_error);
	LuaInterpreterParameters_DECLARE_FUNCTION(ImageOperationResultExternal, get_image_info, int handle, image_info *);
	LuaInterpreterParameters_DECLARE_FUNCTION(ImageOperationResultExternal, load_image, const char *path);
	LuaInterpreterParameters_DECLARE_FUNCTION(ImageOperationResultExternal, unload_image, int handle);
	LuaInterpreterParameters_DECLARE_FUNCTION(ImageOperationResultExternal, allocate_image, int w, int h);
	LuaInterpreterParameters_DECLARE_FUNCTION(ImageOperationResultExternal, save_image, int handle, const char *path, int compression, const char *format);
	LuaInterpreterParameters_DECLARE_FUNCTION0(int, get_caller_image);
	LuaInterpreterParameters_DECLARE_FUNCTION(void, display_in_current_window, int handle);
	LuaInterpreterParameters_DECLARE_FUNCTION(void, debug_print, const char *string);
};

#define Lua_DECLARE_EXPORTED_FUNCTION(rt, x, ...) \
	typedef rt (*x##_f)(__VA_ARGS__); \
	LUA_PLUGIN_EXPORT_C rt x(__VA_ARGS__)

Lua_DECLARE_EXPORTED_FUNCTION(LuaInterpreter *, new_LuaInterpreter, LuaInterpreterParameters *parameters);
Lua_DECLARE_EXPORTED_FUNCTION(void, delete_LuaInterpreter, LuaInterpreter *);
Lua_DECLARE_EXPORTED_FUNCTION(void, LuaInterpreter_execute, CallResult *result, LuaInterpreter *, const char *filename, const void *buffer, size_t size);
Lua_DECLARE_EXPORTED_FUNCTION(void, delete_LuaCallResult, CallResult *);

#endif
