/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef PLUGINCORESTATE_H
#define PLUGINCORESTATE_H

#include "ImageStore.h"
#include <memory>
#include <cassert>
#include <QLibrary>
#include "Lua/main.h"

class QString;
class MainWindow;

class PluginCoreState{
	MainWindow *latest_caller = nullptr;
	ImageStore image_store;
	QLibrary lua_library;
	QLibrary cpp_library;
	int caller_image_handle = -1;

	void execute_lua(const QString &);
	void execute_cpp(const QString &);
public:
	PluginCoreState();
	void set_current_caller(MainWindow *mw){
		this->latest_caller = mw;
	}
	void execute(const QString &path);
	MainWindow *get_caller() const{
		assert(!!this->latest_caller);
		return this->latest_caller;
	}
	ImageStore &get_store(){
		return this->image_store;
	}
	LuaInterpreterParameters construct_LuaInterpreterParameters();
	int get_caller_image_handle();
	void display_in_caller(int handle);
};

bool is_cpp_path(const QString &);
extern const char * const accepted_cpp_extensions[];
extern const size_t accepted_cpp_extensions_size;

#endif
