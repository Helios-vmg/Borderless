/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "LuaInterpreter.h"
#include "lua.h"
#include <cassert>
#include <sstream>

LuaInterpreter::LuaInterpreter(const LuaInterpreterParameters &params): parameters(params){
	auto state = this->parameters.state;
	auto f = this->parameters.release_returned_string;
	this->release_function = [=](char *s){ f(state, s); };
	this->lua_state = init_lua_state(this);
}

LuaInterpreter::~LuaInterpreter(){}

LuaCallResult LuaInterpreter::execute_buffer(const char *filename, const void *buffer, size_t size){
	LuaCallResult ret;
	auto state = this->lua_state.get();
	try{
		luaL_loadbuffer(state, (const char *)buffer, size, filename);
		lua_call(state, 0, 0);
		lua_getglobal(state, "is_pure_filter");
		bool pure_filter = false;
		if (lua_isboolean(state, -1))
			pure_filter = !!lua_toboolean(state, -1);
		lua_pop(state, 1);
		if (pure_filter){
			lua_getglobal(state, "main");
			if (!lua_isfunction(state, -1))
				throw std::exception("Pure filter doesn't contain a main function.");

			auto imgno = this->get_caller_image();
			lua_pushinteger(state, imgno);
			lua_call(state, 1, 1);
			if (lua_isnumber(state, -1))
				imgno = (int)lua_tointeger(state, -1);
			auto image = this->display_in_current_window(imgno);
		}
	}catch (LuaStackUnwind &){
	}catch (std::exception &e){
		ret.impl = new LuaCallResultImpl(e.what());
		ret.success = false;
		ret.error_message = ret.impl->message.c_str();
		return ret;
	}catch (...){
		std::stringstream stream;
		stream << "Lua threw an error: " << lua_tostring(state, -1);
		ret.impl = new LuaCallResultImpl(stream.str());
		ret.success = false;
		ret.error_message = ret.impl->message.c_str();
		return ret;
	}
	return ret;
}

void LuaInterpreter::message_box(const char *title, const char *message, bool is_error){
	this->parameters.show_message_box(this->parameters.state, title, message, is_error);
}

ImageOperationResult to_ImageOperationResult(const ImageOperationResultExternal &src, std::function<void(char *)> &release){
	ImageOperationResult ret;
	ret.success = src.success;
	if (src.message)
		ret.message = src.message;
	static_assert(sizeof(ret.results) == sizeof(src.results), "Inconsistent struct definitions!");
	memcpy(ret.results, src.results, sizeof(ret.results));
	release(src.message);
	return ret;
}

ImageOperationResult LuaInterpreter::load_image(const char *path){
	return to_ImageOperationResult(this->parameters.load_image(this->parameters.state, path), this->release_function);
}

ImageOperationResult LuaInterpreter::unload_image(int handle){
	return to_ImageOperationResult(this->parameters.unload_image(this->parameters.state, handle), this->release_function);
}

ImageOperationResult LuaInterpreter::allocate_image(int w, int h){
	return to_ImageOperationResult(this->parameters.allocate_image(this->parameters.state, w, h), this->release_function);
}

ImageOperationResult LuaInterpreter::save_image(int handle, const char *path, const SaveOptions &options){
	auto ret = this->parameters.save_image(this->parameters.state, handle, path, options.compression, options.format.c_str());
	return to_ImageOperationResult(ret, this->release_function);
}

ImageOperationResult LuaInterpreter::traverse(int handle, traverse_callback_t cb, void *ud){
	image_info info;
	auto ret = this->parameters.get_image_info(this->parameters.state, handle, &info);
	if (!ret.results)
		return to_ImageOperationResult(ret, this->release_function);
	this->release_function(ret.message);

	auto pixels = (unsigned char *)info.pixels;
	traversal_stack_frame current_frame = {
		this->frame,
		info,
		nullptr,
	};
	for (int y = 0; y < info.h; y++){
		//auto scanline = pixels + this->pitch * (this->h - 1 - y);
		auto scanline = pixels + info.pitch * y;
		for (int x = 0; x < info.w; x++){
			auto pixel = scanline + x * info.stride;
			int r = pixel[0];
			int g = pixel[1];
			int b = pixel[2];
			int a = pixel[3];

			current_frame.current_pixel = pixel;

			this->frame = &current_frame;
			cb(ud, r, g, b, a, x, y);
			this->frame = this->frame->prev;
		}
	}

	return ImageOperationResult();
}

void LuaInterpreter::set_current_pixel(const pixel_t &rgba){
	if (!this->frame)
		return;
	auto pixel = this->frame->current_pixel;
	for (int i = 0; i < 4; i++)
		pixel[i] = rgba[i];
}

ImageOperationResult LuaInterpreter::get_pixel(int handle, int x, int y){
	image_info info;
	auto result = this->parameters.get_image_info(this->parameters.state, handle, &info);
	if (!result.results)
		return to_ImageOperationResult(result, this->release_function);
	this->release_function(result.message);

	if (x >= info.w || y >= info.h)
		return "Invalid coordinates.";
	auto pixels = (unsigned char *)info.pixels;
	auto pixel = pixels + info.pitch * y + info.stride * x;
	ImageOperationResult ret;
	for (int i = 0; i < 4; i++)
		ret.results[i] = pixel[i];
	return ret;
}

ImageOperationResult LuaInterpreter::get_image_dimensions(int handle){
	image_info info;
	auto result = this->parameters.get_image_info(this->parameters.state, handle, &info);
	if (!result.results)
		return to_ImageOperationResult(result, this->release_function);
	this->release_function(result.message);

	ImageOperationResult ret;
	ret.results[0] = info.w;
	ret.results[1] = info.h;
	return ret;
}

int LuaInterpreter::get_caller_image(){
	return this->parameters.get_caller_image(this->parameters.state);
}

ImageOperationResult LuaInterpreter::display_in_current_window(int handle){
	this->parameters.display_in_current_window(this->parameters.state, handle);
	return ImageOperationResult();
}

void LuaInterpreter::debug_print(const char *string){
	this->parameters.debug_print(this->parameters.state, string);
}
