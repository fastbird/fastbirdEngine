#pragma once

#pragma warning(disable : 4996) // The POSIX name for this item is deprecated
#pragma warning( disable : 4275 4091 ) // Because ReferenceCounter is not exported class.
#pragma warning(disable : 4819) // code page

//-------------------------------------------------------------------------------------------------
#define _FBENGINE_FOR_WINDOWS_

// use std::min(), std::max()
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(_DEBUG) && !defined(_WINDLL)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else
	#include <stdlib.h>
#endif

//-------------------------------------------------------------------------------------------------
#define CLASS_DECLSPEC_ENGINE __declspec(dllimport)
#define CLASS_DECLSPEC_UI __declspec(dllimport)
#define CLASS_DECLSPEC_PHYSICS __declspec(dllimport)

#if defined(_ENGINE)
	#undef CLASS_DECLSPEC_ENGINE
	#define CLASS_DECLSPEC_ENGINE __declspec(dllexport)
#elif defined(_UI)
	#undef CLASS_DECLSPEC_UI
	#define CLASS_DECLSPEC_UI __declspec(dllexport)
#elif defined(_PHYSICS)
#undef CLASS_DECLSPEC_PHYSICS
#define CLASS_DECLSPEC_PHYSICS __declspec(dllexport)
#endif

// need to check object.h file in engine project
#define USING_FB_MEMORY_MANAGER
#include <CommonLib/MemoryManager.h>

#define SAFE_RELEASE(x) (x) ? (x)->Release() : 0; (x)=0
// for engine obj
#define FB_RELEASE(x) (x) ? (x)->Delete() : 0; (x)=0