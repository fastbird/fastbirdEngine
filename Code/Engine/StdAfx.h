#include <CommonLib/Config.h>

#include <string>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include <stdarg.h>

#pragma comment(lib, "CommonLib.lib")
#pragma comment(lib, "lua.lib")
#pragma comment(lib, "d3dx11d.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "zdll.lib")

#ifdef _DEBUG
#pragma comment(lib, "freeimaged.lib")
#else
#pragma comment(lib, "freeimage.lib")
#endif

// Open Colladas
#pragma comment(lib, "OpenCOLLADASaxFrameworkLoader.lib")
#pragma comment(lib, "OpenCOLLADAFramework.lib")
#pragma comment(lib, "OpenCOLLADABaseUtils.lib")
#pragma comment(lib, "GeneratedSaxParser.lib")
#pragma comment(lib, "pcre.lib")
#pragma comment(lib, "xml.lib")
#pragma comment(lib, "UTF.lib")
#pragma comment(lib, "MathMLSolver.lib")




extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <zlib.h>

#include <CommonLib/CommonLib.h>
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Color.h>
#include <CommonLib/SmartPtr.h>
#include <CommonLib/tinyxml2.h>
#include <CommonLib/threads.h>
#include <CommonLib/Timer.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/Math/Transformation.h>
#include <CommonLib/Unicode.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/VectorMap.h>
#include <CommonLib/Profiler.h>
#include <CommonLib/FileSystem.h>

#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IMaterial.h>
#include <Engine/IFont.h>
#include <Engine/IInputLayout.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IShader.h>
#include <Engine/IRenderState.h>
#include <Engine/IKeyboard.h>
#include <Engine/IMouse.h>
#include <Engine/D3DEventMarker.h>
#include <Engine/IConsole.h>