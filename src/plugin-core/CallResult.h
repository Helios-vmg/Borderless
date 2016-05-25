/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CALLRESULT_H
#define CALLRESULT_H

class CallResultImpl;

struct CallResult{
	CallResultImpl *impl = nullptr;
	bool success = true;
	const char *error_message = nullptr;
};

#endif
