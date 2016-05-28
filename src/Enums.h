/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef ENUM_H
#define ENUM_H

enum class ZoomMode {
	Normal = 0,
	Locked = 1,
	Automatic = 2,
	AutoFit = Automatic | 0,
	AutoFill = Automatic | 1,
};

#endif
