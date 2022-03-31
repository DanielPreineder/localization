// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include <sstream>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define NOMINMAX

// Windows Header Files:
#include <windows.h>

#include <usp10.h>

#elif defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>

#endif

#include "BlueExposure/include/BlueExposure.h"
#include <blue/Include/Blue.h>
#include <blue/Include/IBluePython.h>
#include <blue/Include/IBlueOS.h>
#include "blue/Include/BlueStatistics.h"
