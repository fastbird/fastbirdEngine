#include <CommonLib/Config.h>

#pragma comment(lib, "CommonLib.lib")
#pragma comment(lib, "lua.lib")

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <assert.h>
#include <string>
#include <vector>
#include <limits>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>

#include <CommonLib/CommonLib.h>
#include <CommonLib/SmartPtr.h>
#include <CommonLib/Timer.h>
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Color.h>
#include <CommonLib/threads.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/BlockFrequent.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/Unicode.h>
#include <CommonLib/VectorMap.h>
#include <CommonLib/tinyxml2.h>
#include <CommonLib/Math/Transformation.h>

#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IScene.h>
#include <Engine/ICamera.h>
#include <Engine/IScriptSystem.h>
#include <Engine/GlobalEnv.h>
extern fastbird::GlobalEnv* gEnv;
#include <Engine/DllMain.h>
#include <Engine/IMouse.h>
#include <Engine/IKeyboard.h>
#include <Engine/IConsole.h>
#include <Engine/IMeshObject.h>
#include <Engine/IInputListener.h>