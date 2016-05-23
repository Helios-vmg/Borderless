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
public:
	CppInterpreter(const CppInterpreterParameters &);
	~CppInterpreter();
	CallResult execute_buffer(const char *filename);
	void pass_main_arguments(void *&, int &) const;
	void set_return_value(int);
};

#endif
