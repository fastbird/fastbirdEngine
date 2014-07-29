#include <CommonLib/Config.h>

#include <assert.h>
#include <process.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#pragma comment(lib, "CommonLib.lib")
#pragma comment(lib, "lua.lib")
#pragma comment(lib, "freeimage.lib")
#pragma comment(lib, "d3dx11d.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "zdll.lib")

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <zlib.h>

#include <CommonLib/CommonLib.h>
#include <CommonLib/Color.h>
#include <CommonLib/SmartPtr.h>
#include <CommonLib/tinyxml2.h>
#include <CommonLib/threads.h>
#include <CommonLib/Timer.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Math/Transformation.h>
#include <CommonLib/Unicode.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/VectorMap.h>

#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IMaterial.h>
#include <Engine/IFont.h>
#include <Engine/IInputLayout.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IShader.h>
#include <Engine/Renderer/IRenderState.h>
#include <Engine/IKeyboard.h>
#include <Engine/IMouse.h>