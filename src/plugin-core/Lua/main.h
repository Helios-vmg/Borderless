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

#define EXTERN_C extern "C"

#if defined WIN32 && defined BUILDING_LUAINTERPRETER
#define EXPORT_C EXTERN_C __declspec(dllexport)
#else
#define EXPORT_C EXTERN_C
#endif

class LuaInterpreter;
typedef void *external_state;

struct ImageOperationResultExternal{
	bool success;
	int results[4];
	char *message;
};

struct image_info{
	int handle;
	int w, h, stride, pitch;
	void *pixels;
};

struct LuaInterpreterParameters{
	external_state state;

#define LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(rt, x, ...) \
	typedef rt (*x##_f)(external_state, __VA_ARGS__); \
	x##_f x
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(void, release_returned_string, char *);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(void, show_message_box, const char *title, const char *message, bool is_error);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(ImageOperationResultExternal, get_image_info, int handle, image_info *);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(ImageOperationResultExternal, load_image, const char *path);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(ImageOperationResultExternal, unload_image, int handle);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(ImageOperationResultExternal, allocate_image, int w, int h);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(ImageOperationResultExternal, save_image, int handle, const char *path, int compression, const char *format);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(int, get_caller_image);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(void, display_in_current_window, int handle);
	LuaInterpreterParameters_DECLARE_FUNCTION_POINTER(void, debug_print, const char *string);
};

class LuaCallResultImpl;

struct LuaCallResult{
	LuaCallResultImpl *impl = nullptr;
	bool success = true;
	const char *error_message = nullptr;
};

EXPORT_C LuaInterpreter *new_LuaInterpreter(LuaInterpreterParameters *parameters);
EXPORT_C void delete_LuaInterpreter(LuaInterpreter *);
EXPORT_C void LuaInterpreter_execute(LuaCallResult *result, LuaInterpreter *, const char *filename, const void *buffer, size_t size);
EXPORT_C void delete_LuaCallResult(LuaCallResult);

#endif
