/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "PluginCoreState.h"
#include "lua.h"
#include "../LoadedImage.h"
#include "../MainWindow.h"
#include <QFile>
#include <QFileInfo>
#include <cassert>

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

void PluginCoreState::execute_lua(const QString &path){
	try{
		QFile file(path);
		file.open(QFile::ReadOnly);
		if (!file.isOpen())
			throw std::exception("Unknown error while reading file.");
		auto data = file.readAll();

		auto lua_state = init_lua_state(this);
		auto state = lua_state.get();
		try{
			luaL_loadbuffer(state, data.data(), data.size(), path.toUtf8().toStdString().c_str());
			lua_call(state, 0, 0);
			lua_getglobal(state, "is_pure_filter");
			bool pure_filter = false;
			if (lua_isboolean(state, -1))
				pure_filter = lua_toboolean(state, -1);
			lua_pop(state, 1);
			assert(!!this->latest_caller);
			if (pure_filter){
				lua_getglobal(state, "main");
				if (!lua_isfunction(state, -1))
					throw std::exception("Pure filter doesn't contain a main function.");
				auto imgno = this->image_store.store(this->latest_caller->get_image());
				lua_pushinteger(state, imgno);
				lua_call(state, 1, 1);
				if (lua_isnumber(state, -1))
					imgno = lua_tointeger(state, -1);
				auto image = this->image_store.get_image(imgno)->get_bitmap();
				this->latest_caller->display_filtered_image(std::make_shared<LoadedImage>(image));
			}
		}catch (std::exception &){
			throw;
		}catch (...){
			lua_panic_function(state);
		}
	}catch (LuaStackUnwind &){
	}
}

void PluginCoreState::execute_cpp(const QString &path){
	throw std::exception("Not implemented.");
}
