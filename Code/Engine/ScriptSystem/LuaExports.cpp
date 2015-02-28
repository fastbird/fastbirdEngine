#include <Engine/StdAfx.h>
#include <CommonLib/luawrapper.hpp>

using namespace fastbird;

namespace fastbird
{
	int GetResolution(lua_State* L);
	int GetFrameCounter(lua_State* L);

	void InitEngineLuaFuncs(lua_State* L)
	{
		LUA_SETCFUNCTION(L, GetResolution);
		LUA_SETCFUNCTION(L, GetFrameCounter);
	}

	int GetResolution(lua_State* L)
	{
		luaU_push<Vec2I>(L, Vec2I(gFBEnv->pRenderer->GetWidth(), gFBEnv->pRenderer->GetHeight()));
		return 1;
	}

	int GetFrameCounter(lua_State* L)
	{
		lua_pushunsigned(L, gFBEnv->mFrameCounter);
		return 1;
	}

}
