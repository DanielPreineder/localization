// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#define PY_SSIZE_T_CLEAN

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

#include <BlueExposure.h>
#include <Blue.h>
#include <IBluePython.h>
#include <IBlueOS.h>
#include <BlueStatistics.h>
