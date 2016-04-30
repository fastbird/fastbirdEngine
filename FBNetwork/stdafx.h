#pragma once
#include "FBCommonHeaders/platform.h"

#if defined(_PLATFORM_WINDOWS_) 
#define FB_DLL_NETWORK __declspec(dllexport)
#define FB_DLL_TIMER __declspec(dllimport)
#define FB_DLL_FILESYSTEM __declspec(dllimport)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#endif

#include "FBCommonHeaders/Helpers.h"
#include "FBDebugLib/DebugLib.h"
#include "FBStringLib/StringLib.h"
#include "FBTimer/Timer.h"
#include "FBFileSystem/FileSystem.h"

#include <map>
#include <atomic>

#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "cppnetlib-uri.lib")