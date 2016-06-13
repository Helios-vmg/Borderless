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
#include <QThreadStorage>
#include "Lua/main.h"
#include "Cpp/main.h"

class QString;
class MainWindow;

class PluginCoreState{
	MainWindow *latest_caller = nullptr;
	ImageStore image_store;
	QLibrary lua_library;
	QLibrary cpp_library;
	std::vector<void *> cpp_tls;
	size_t cpp_tls_size;
	int caller_image_handle = -1;
	std::shared_ptr<CppInterpreter> cpp_interpreter;
	void (*CppInterpreter_execute)(CallResult *, CppInterpreter *, const char *);
	void (*delete_CppCallResult)(CallResult *);
	void (*CppInterpreter_reset_imag)(CppInterpreter *, external_state);

	void execute_lua(const QString &);
	void execute_cpp(const QString &);
	void execute_cpp_ready(const QString &);
	void *get_image_pointer();
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
	CppInterpreterParameters construct_CppInterpreterParameters();
	int get_caller_image_handle();
	void display_in_caller(int handle);
	void display_in_caller(Image *img);
	void store_tls(void *);
	void *retrieve_tls();
};

bool is_cpp_path(const QString &);
extern const char * const accepted_cpp_extensions[];
extern const size_t accepted_cpp_extensions_size;

#endif
