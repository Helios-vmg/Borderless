/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CPPINTERPRETER_H
#define CPPINTERPRETER_H

#include "main.h"
#include <string>

class CppInterpreter{
	CppInterpreterParameters parameters;
	void *return_value;
public:
	CppInterpreter(const CppInterpreterParameters &);
	~CppInterpreter();
	CallResult execute_buffer(const char *filename);
	void pass_main_arguments(void *&, void *&) const;
	void set_return_value(void *rv){
		this->return_value = rv;
	}
};

#endif
