#include "FBCommonHeaders/platform.h"
#if defined(_PLATFORM_WINDOWS_) 
#define FB_DLL_AUDIODEBUGGER __declspec(dllexport)
#define FB_DLL_AUDIOPLAYER __declspec(dllimport)
#define FB_DLL_RENDERER __declspec(dllimport)
#else
#define FB_DLL_AUDIOPLAYER
#endif

#include "FBMathLib/Math.h"
#include "FBDebugLib/Logger.h"
#include "FBStringLib/StringLib.h"