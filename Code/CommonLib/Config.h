#pragma once

#pragma warning(disable : 4996) // The POSIX name for this item is deprecated
#pragma warning( disable : 4275 4091 ) // Because ReferenceCounter is not exported class.
#pragma warning(disable : 4819) // code page

//-------------------------------------------------------------------------------------------------
#define _FBENGINE_FOR_WINDOWS_
//#define _ITERATOR_DEBUG_LEVEL 0

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
// need to check object.h file in engine project
#define USING_FB_MEMORY_MANAGER
#include <CommonLib/MemoryManager.h>

#define SAFE_RELEASE(x) (x) ? (x)->Release() : 0; (x)=0

// for engine obj
#define FB_RELEASE(x) (x) ? (x)->Delete() : 0; (x)=0

#include <CommonLib/CommonLib.h>