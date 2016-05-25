/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CALLRESULTIMPL_H
#define CALLRESULTIMPL_H

#include <string>
#include "CallResult.h"

class CallResultImpl{
public:
	bool success;
	std::string message;

	CallResultImpl(): success(true){}
	CallResultImpl(const char *message): message(message){}
	CallResultImpl(const std::string &message): message(message){}
};

#endif
