#include <Engine/StdAfx.h>
#include <CommonLib/luawrapper.hpp>

using namespace fastbird;

namespace fastbird
{
	int GetResolution(lua_State* L);
	int GetFrameCounter(lua_State* L);
	int PrintSpatialObject(lua_State* L);

	void InitEngineLuaFuncs(lua_State* L)
	{
		LUA_SETCFUNCTION(L, PrintSpatialObject);
		LUA_SETCFUNCTION(L, GetResolution);
		LUA_SETCFUNCTION(L, GetFrameCounter);
	}

	int GetResolution(lua_State* L)
	{
		const auto& size = gFBEnv->pRenderer->GetMainRTSize();
		luaU_push<Vec2I>(L, size);
		return 1;
	}

	int GetFrameCounter(lua_State* L)
	{
		lua_pushunsigned(L, gFBEnv->mFrameCounter);
		return 1;
	}
	
	int PrintSpatialObject(lua_State* L)
	{
		if (gFBEnv)
		{
			auto scene = gFBEnv->pRenderer->GetMainScene();
			if (scene)
			{
				scene->PrintSpatialObject();
			}
		}
		return 0;
	}
}
