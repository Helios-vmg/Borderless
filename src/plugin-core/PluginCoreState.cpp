/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "PluginCoreState.h"
#include "../LoadedImage.h"
#include "../MainWindow.h"
#include "../ClangErrorMessage.hpp"
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#ifdef WIN32
#include <Windows.h>
#endif

const char * const accepted_cpp_extensions[] = {
	"cpp",
	"c",
	"cc",
	"cxx",
	"c++",
};
const size_t accepted_cpp_extensions_size = sizeof(accepted_cpp_extensions) / sizeof(*accepted_cpp_extensions);

bool is_cpp_path(const QString &path){
	auto file_extension = QFileInfo(path).suffix();
	for (auto &ext : accepted_cpp_extensions)
		if (!file_extension.compare(QString(ext), Qt::CaseInsensitive))
			return true;
	return false;
}

bool is_lua_path(const QString &path){
	auto file_extension = QFileInfo(path).suffix().toLower();
	return file_extension == "lua";
}

PluginCoreState::PluginCoreState(){
}

void PluginCoreState::execute(const QString &path){
	if (!QFile::exists(path))
		throw std::exception("File not found.");
	if (is_cpp_path(path))
		this->execute_cpp(path);
	if (is_lua_path(path))
		this->execute_lua(path);
}

#define RESOLVE_FUNCTION(lib, x) auto x = (x##_f)lib.resolve(#x)

void PluginCoreState::execute_lua(const QString &path){
	if (!this->lua_library.isLoaded()){
		this->lua_library.setFileName("LuaInterpreter");
		this->lua_library.load();
	}
	if (!this->lua_library.isLoaded())
		return;
	this->caller_image_handle = -1;
	QFile file(path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		throw std::exception("Unknown error while reading file.");
	auto data = file.readAll();
	
	auto filename = QFileInfo(path).fileName();

	RESOLVE_FUNCTION(this->lua_library, new_LuaInterpreter);
	RESOLVE_FUNCTION(this->lua_library, delete_LuaInterpreter);
	RESOLVE_FUNCTION(this->lua_library, LuaInterpreter_execute);
	RESOLVE_FUNCTION(this->lua_library, delete_LuaCallResult);

	auto params = this->construct_LuaInterpreterParameters();
	std::shared_ptr<LuaInterpreter> interpreter(new_LuaInterpreter(&params), [=](LuaInterpreter *i){ delete_LuaInterpreter(i); });

	CallResult result;
	LuaInterpreter_execute(&result, interpreter.get(), filename.toUtf8().toStdString().c_str(), data.data(), data.size());
	delete_LuaCallResult(&result);
}

int PluginCoreState::get_caller_image_handle(){
	if (this->caller_image_handle >= 0)
		return this->caller_image_handle;
	return this->caller_image_handle = this->image_store.store(this->latest_caller->get_image());
}

void PluginCoreState::display_in_caller(int handle){
	this->display_in_caller(this->get_store().get_image(handle).get());
}

void PluginCoreState::display_in_caller(Image *image){
	if (image)
		this->latest_caller->display_filtered_image(std::make_shared<LoadedImage>(image->get_bitmap()));
}

char *clone_string(const char *s){
	if (!s)
		return nullptr;
	auto n = strlen(s);
	auto ret = new char[n + 1];
	memcpy(ret, s, n);
	ret[n] = 0;
	return ret;
}

char *clone_string(const std::string &s){
	auto n = s.size();
	if (!n)
		return nullptr;
	auto ret = new char[n + 1];
	memcpy(ret, &s[0], n);
	ret[n] = 0;
	return ret;
}

ImageOperationResultExternal to_ImageOperationResultExternal(const ImageOperationResult &src){
	ImageOperationResultExternal ret;
	ret.success = src.success;
	static_assert(sizeof(ret.results) == sizeof(src.results), "Inconsistent struct definitions!");
	memcpy(ret.results, src.results, sizeof(ret.results));
	ret.message = clone_string(src.message);
	return ret;
}

namespace lua_implementations{
#define LUA_FUNCTION_SIGNATURE(rt, x, ...) rt x(external_state state, __VA_ARGS__)

LUA_FUNCTION_SIGNATURE(void, release_returned_string, char *s){
	delete[] s;
}

LUA_FUNCTION_SIGNATURE(void, show_message_box, const char *title, const char *message, bool is_error){
	QMessageBox msgbox;
	if (title)
		msgbox.setWindowTitle(QString::fromUtf8(title));
	msgbox.setText(QString::fromUtf8(message));
	if (is_error)
		msgbox.setIcon(QMessageBox::Critical);
	msgbox.exec();
}

LUA_FUNCTION_SIGNATURE(ImageOperationResultExternal, get_image_info, int handle, image_info *info){
	ImageOperationResultExternal ret;
	auto This = (PluginCoreState *)state;
	auto image = This->get_store().get_image(handle);
	if (!image){
		ret.success = false;
		ret.message = clone_string(HANDLE_NOT_FOUND_MSG);
		return ret;
	}
	info->handle = handle;
	unsigned stride, pitch;
	info->pixels = image->get_pixels_pointer(stride, pitch);
	info->stride = stride;
	info->pitch = pitch;
	auto temp = image->get_dimensions();
	info->w = temp.results[0];
	info->h = temp.results[1];
	ret.success = true;
	return ret;
}

LUA_FUNCTION_SIGNATURE(ImageOperationResultExternal, load_image, const char *path){
	auto This = (PluginCoreState *)state;
	return to_ImageOperationResultExternal(This->get_store().load(path));
}

LUA_FUNCTION_SIGNATURE(ImageOperationResultExternal, unload_image, int handle){
	auto This = (PluginCoreState *)state;
	return to_ImageOperationResultExternal(This->get_store().unload(handle));
}

LUA_FUNCTION_SIGNATURE(ImageOperationResultExternal, allocate_image, int w, int h){
	auto This = (PluginCoreState *)state;
	return to_ImageOperationResultExternal(This->get_store().allocate(w, h));
}

LUA_FUNCTION_SIGNATURE(ImageOperationResultExternal, save_image, int handle, const char *path, int compression, const char *format){
	auto This = (PluginCoreState *)state;
	SaveOptions opt;
	opt.compression = compression;
	if (format)
		opt.format = format;
	return to_ImageOperationResultExternal(This->get_store().save(handle, QString::fromUtf8(path), opt));
}

LUA_FUNCTION_SIGNATURE(int, get_caller_image){
	auto This = (PluginCoreState *)state;
	return This->get_caller_image_handle();
}

LUA_FUNCTION_SIGNATURE(void, display_in_current_window, int handle){
	auto This = (PluginCoreState *)state;
	This->display_in_caller(handle);
}

LUA_FUNCTION_SIGNATURE(void, debug_print, const char *string){
#ifdef WIN32
	auto temp = QString::fromUtf8(string);
	OutputDebugStringW(temp.toStdWString().c_str());
#endif
}

}

LuaInterpreterParameters PluginCoreState::construct_LuaInterpreterParameters(){
	LuaInterpreterParameters ret;
	ret.state = this;
#define PASS_FUNCTION_TO_LUA(x) ret.x = lua_implementations::x
	PASS_FUNCTION_TO_LUA(release_returned_string);
	PASS_FUNCTION_TO_LUA(show_message_box);
	PASS_FUNCTION_TO_LUA(get_image_info);
	PASS_FUNCTION_TO_LUA(load_image);
	PASS_FUNCTION_TO_LUA(unload_image);
	PASS_FUNCTION_TO_LUA(allocate_image);
	PASS_FUNCTION_TO_LUA(save_image);
	PASS_FUNCTION_TO_LUA(get_caller_image);
	PASS_FUNCTION_TO_LUA(display_in_current_window);
	PASS_FUNCTION_TO_LUA(debug_print);

	return ret;
}

namespace cpp_implementations{

QThreadStorage<uintptr_t> tls;

#define CPP_FUNCTION_SIGNATURE(rt, x, ...) rt x(external_state state, __VA_ARGS__)

CPP_FUNCTION_SIGNATURE(void, release_returned_string, char *s){
	delete[] s;
}

CPP_FUNCTION_SIGNATURE(void, store_tls, void *s){
	auto This = (PluginCoreState *)tls.localData();
	This->store_tls(s);
}

CPP_FUNCTION_SIGNATURE(void *, retrieve_tls){
	auto This = (PluginCoreState *)tls.localData();
	return This->retrieve_tls();
}

CPP_FUNCTION_SIGNATURE(void, display_in_current_window, void *image){
	if (!image)
		return;
	auto This = (PluginCoreState *)tls.localData();
	This->display_in_caller((Image *)image);
}

}

CppInterpreterParameters PluginCoreState::construct_CppInterpreterParameters(){
	CppInterpreterParameters ret;
	ret.state = this;
	ret.caller_image = this->image_store.get_image(this->get_caller_image_handle()).get();
#define PASS_FUNCTION_TO_CPP(x) ret.x = cpp_implementations::x
	PASS_FUNCTION_TO_CPP(release_returned_string);
	PASS_FUNCTION_TO_CPP(store_tls);
	PASS_FUNCTION_TO_CPP(retrieve_tls);
	PASS_FUNCTION_TO_CPP(display_in_current_window);

	return ret;
}

void PluginCoreState::execute_cpp(const QString &path){
	if (!this->cpp_library.isLoaded()){
		this->cpp_library.setFileName("CppInterpreter");
		this->cpp_library.load();
	}
	if (!this->cpp_library.isLoaded())
		return;
	this->caller_image_handle = -1;
	{
		QFile file(path);
		file.open(QFile::ReadOnly);
		if (!file.isOpen())
			throw std::exception("Unknown error while reading file.");
	}

	RESOLVE_FUNCTION(this->cpp_library, new_CppInterpreter);
	RESOLVE_FUNCTION(this->cpp_library, delete_CppInterpreter);
	RESOLVE_FUNCTION(this->cpp_library, CppInterpreter_execute);
	RESOLVE_FUNCTION(this->cpp_library, delete_CppCallResult);

	auto params = this->construct_CppInterpreterParameters();
	std::shared_ptr<CppInterpreter> interpreter(new_CppInterpreter(&params), [=](CppInterpreter *i){ delete_CppInterpreter(i); });

	auto old_tls = cpp_implementations::tls.localData();
	cpp_implementations::tls.setLocalData((uintptr_t)this);
	auto old_size = this->cpp_tls_size;
	this->cpp_tls_size = this->cpp_tls.size();

	CallResult result;
	auto parameter = QDir::toNativeSeparators(path).toUtf8().toStdString();
	CppInterpreter_execute(&result, interpreter.get(), parameter.c_str());
	if (!result.success){
		ClangErrorMessage msgbox;
		msgbox.set_error_message(QString::fromUtf8(result.error_message));
		msgbox.exec();
	}
	delete_CppCallResult(&result);

	this->cpp_tls.resize(this->cpp_tls_size);
	this->cpp_tls_size = old_size;
	cpp_implementations::tls.setLocalData(old_tls);
}

void PluginCoreState::store_tls(void *p){
	if (this->cpp_tls_size == this->cpp_tls.size())
		this->cpp_tls.push_back(p);
	else
		this->cpp_tls.back() = p;
}

void *PluginCoreState::retrieve_tls(){
	if (this->cpp_tls_size == this->cpp_tls.size())
		return nullptr;
	return this->cpp_tls.back();
}
