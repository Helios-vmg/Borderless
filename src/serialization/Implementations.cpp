/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "settings.generated.h"

#define DEFINE_TRIVIAL_IMPLEMENTATIONS(x) x::~x(){} void x::rollback_deserialization(){}

DEFINE_TRIVIAL_IMPLEMENTATIONS(IntPair)
DEFINE_TRIVIAL_IMPLEMENTATIONS(UintPair)
DEFINE_TRIVIAL_IMPLEMENTATIONS(DoubleQuadTuple)
DEFINE_TRIVIAL_IMPLEMENTATIONS(WindowState)
DEFINE_TRIVIAL_IMPLEMENTATIONS(WindowsState)
DEFINE_TRIVIAL_IMPLEMENTATIONS(Settings)
DEFINE_TRIVIAL_IMPLEMENTATIONS(MainSettings)
DEFINE_TRIVIAL_IMPLEMENTATIONS(Shortcuts)
