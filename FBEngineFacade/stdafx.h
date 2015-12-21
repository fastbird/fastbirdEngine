/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "FBCommonHeaders/platform.h"
#if defined(_PLATFORM_WINDOWS_)
#define FB_DLL_ENGINEFACADE __declspec(dllexport)
#define FB_DLL_RENDERER __declspec(dllimport)
#define FB_DLL_LUA __declspec(dllimport)
#define FB_DLL_SCENEMANAGER __declspec(dllimport)
#define FB_DLL_SCENEOBJECTFACTORY __declspec(dllimport)
#define FB_DLL_TIMER __declspec(dllimport)
#define FB_DLL_INPUTMANAGER __declspec(dllimport)
#define FB_DLL_CONSOLE __declspec(dllimport)
#define FB_DLL_FILEMONITOR __declspec(dllimport)
#define FB_DLL_VIDEOPLAYER __declspec(dllimport)
#define FB_DLL_AUDIOPLAYER __declspec(dllimport)
#define FB_DLL_COLLADA __declspec(dllimport)
#define FB_DLL_PARTICLESYSTEM __declspec(dllimport)
#define FB_DLL_FILESYSTEM __declspec(dllimport)
#define FB_DLL_AUDIODEBUGGER __declspec(dllimport)

#else
#define FB_DLL_ENGINEFACADE 
#endif

#include <assert.h>

#include "FBRenderer/Renderer.h"
#include "FBLua/LuaUtils.h"
#include "FBMemoryManagerLib/MemoryManager.h"
#include "FBTimer/Timer.h"
#include "FBMathLib/Math.h"