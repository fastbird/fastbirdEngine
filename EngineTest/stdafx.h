// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#define FB_DLL_TIMER __declspec(dllimport)
#define FB_DLL_SCENEMANAGER __declspec(dllimport)
#define FB_DLL_ENGINEFACADE __declspec(dllimport)
#define FB_DLL_AUDIOPLAYER __declspec(dllimport)
#define FB_DLL_VIDEOPLAYER __declspec(dllimport)
#include "FBTimer/Timer.h"
#include "FBMathLib/Math.h"