#include <CommonLib/Config.h>

#pragma comment(lib, "CommonLib.lib")

#include <assert.h>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <algorithm>
#include <functional>
#include "ComponentType.h"
#include "Align.h"

#include <CommonLib/CommonLib.h>
#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Vec2.h>
#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Color.h>
#include <CommonLib/Unicode.h>
#include <CommonLib/tinydir.h>
#include <CommonLib/Timer.h>
#include <CommonLib/tinyxml2.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/Profiler.h>

#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IKeyboard.h>
#include <Engine/IMouse.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IUIObject.h>
#include <Engine/IObject.h>
#include <Engine/IMaterial.h>
#include <Engine/IFont.h>
#define gEnv gFBEnv