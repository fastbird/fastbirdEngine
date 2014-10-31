#pragma once
#include <CommonLib/luawrapperutil.hpp>

struct lua_State;

#define CHECK_NUM_LUA_ARGS_FB(x) \
	int numLuaArgs = lua_gettop(L); \
if (numLuaArgs != (x)) \
{\
	assert(0); \
	return luaL_error(L, "Got %d arguments, expected %d", numLuaArgs, (x)); \
}

#define LUA_SETCFUNCTION(lua, name) lua_pushcfunction((lua), (name));\
	lua_setglobal((lua), (#name));

#define LUA_PCALL(lua, arg, ret) if(int error = lua_pcall((lua), arg, ret, 0)) \
{\
	const char* errorString = lua_tostring(lua, -1); \
	fastbird::Error("Failed to call lua function. ErrorNo : %d, Msg : %s", error, errorString); \
	lua_pop(lua, 1); \
	assert(0);\
	return;\
}

#define LUA_PCALL_RET_FALSE(lua, arg, ret) if(int error = lua_pcall((lua), arg, ret, 0)) \
{\
	const char* errorString = lua_tostring(lua, -1); \
	fastbird::Error("Failed to call lua function. ErrorNo : %d, Msg : %s", error, errorString); \
	lua_pop(lua, 1); \
	assert(0); \
	return false; \
}
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

	// Generic lua call
	void CallLuaFunction(lua_State* L, const char* func, const char* sig, ...);
	bool CheckLuaGlobalExist(lua_State* L, const char* name);
	void PrintLuaDebugInfo(lua_State* L, int level);
	std::string GetLuaValueAsString(lua_State* L, int stackIndex);
}

// luawapper util
// std::string
template<>
struct luaU_Impl<std::string>
{
	static std::string luaU_check(lua_State* L, int index)
	{
		return std::string(luaL_checkstring(L, index));
	}

	static std::string luaU_to(lua_State* L, int index)
	{
		return std::string(lua_tostring(L, index));
	}

	static void luaU_push(lua_State* L, const std::string& val)
	{
		lua_pushstring(L, val.c_str());
	}
};