#pragma once
#include "FBCommonHeaders/platform.h"
#if defined(_PLATFORM_WINDOWS_)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#endif

#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <clocale>
#include <sstream>