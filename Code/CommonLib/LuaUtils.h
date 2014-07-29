#pragma once

struct lua_State;
namespace fastbird
{
	struct LUA_STACK_WATCHER
	{
		LUA_STACK_WATCHER(lua_State* L)
		{
			lua = L;
			top = lua_gettop(L);
		}

		~LUA_STACK_WATCHER()
		{
			int now = lua_gettop(lua);
			assert(top == now);
		}

		lua_State* lua;
		int top;
	};
}

#define CHECK_NUM_LUA_ARGS_FB(x) \
	int numLuaArgs = lua_gettop(L); \
	if (numLuaArgs != (x)) \
	{\
		assert(0); \
		return luaL_error(L, "Got %d arguments, expected %d", numLuaArgs, (x));\
	}