/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef ENUM_H
#define ENUM_H

enum class ZoomMode {
	Normal            = 0,
	Locked            = 1 << 0,
	AutomaticZoom     = 1 << 1,
	AutomaticRotation = 1 << 2,
	AutoFit           =                     AutomaticZoom | 0,
	AutoFill          =                     AutomaticZoom | 1,
	AutoRotFit        = AutomaticRotation | AutomaticZoom | 0,
	AutoRotFill       = AutomaticRotation | AutomaticZoom | 1,
};

#endif
