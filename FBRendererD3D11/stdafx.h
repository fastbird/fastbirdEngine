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
\defgroup FBRendererD3D11
Direct3D 11 renderer

Required libraries: \b FBMemoryManager, \b FBDebugLib, \b FBMath
*/
#pragma warning (disable : 4251)
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11d.lib")
#endif
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "zdll.lib")

#define FB_DLL_RENDERERD3D11 __declspec(dllexport)
#define FB_DLL_FILESYSTEM __declspec(dllimport)

#define NOMINMAX
#include <memory>
#include <iostream>
#include <D3DX11.h>
#include <DXGI.h>
#include <D3Dcompiler.h>
#include <map>
#include <d3d9.h>
#include <assert.h>
#include "zlib.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBMathLib/Math.h"
#include "FBMemoryManagerLib/MemoryManager.h"
#include "FBDebugLib/Logger.h"

#define SAFE_RELEASE(x) (x) ? (x)->Release() : 0; (x)=0