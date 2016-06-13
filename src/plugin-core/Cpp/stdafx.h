#ifdef _MSC_VER
#pragma once

#define USING_PRECOMPILED_HEADERS
#include "llvm_headers.h"
#include <sstream>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#endif

#endif
