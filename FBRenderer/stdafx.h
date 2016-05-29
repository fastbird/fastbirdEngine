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

/**
\defgroup FBRenderer
Main hub module for rendering. 

Required libraries: \b FBDebugLib, \b FBMath, \b FBSystemLib, \b FBMemoryManagerLib, \b FBStringMathLib, \b FBStringLib
Required modules: \b FBColladaImporter, \b FBRenderableFactory
Plugin: \b FBRendererD3D11
*/
#include "FBCommonHeaders/platform.h"
#if defined(_PLATFORM_WINDOWS_) 
#define FB_DLL_RENDERER __declspec(dllexport)
#define FB_DLL_TIMER __declspec(dllimport)
#define FB_DLL_FILESYSTEM __declspec(dllimport)
#define FB_DLL_INPUTMANAGER __declspec(dllimport)
#define FB_DLL_LUA __declspec(dllimport)
#define FB_DLL_SCENEMANAGER __declspec(dllimport)
#define FB_DLL_CONSOLE __declspec(dllimport)
#define FB_DLL_THREAD __declspec(dllimport)
#else
#define FB_DLL_TIMER
#endif

#if defined(_PLATFORM_WINDOWS_)
#else
#include "PrefixHeader.pch"
#endif

#include <memory>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>
#include <iostream>
#include <cstdarg>
#include <mutex>
#include <regex>
#include <array>
#include <unordered_map>

#include "FBMemoryManagerLib/MemoryManager.h"
#include "FBDebugLib/Logger.h"
#include "FBStringLib/StringLib.h"
#include "FBMathLib/Math.h"