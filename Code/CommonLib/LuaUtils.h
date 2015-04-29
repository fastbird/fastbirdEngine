#pragma once
#include <CommonLib/luawrapperutil.hpp>
#include <CommonLib/Math/Vec2I.h>

struct lua_State;
namespace fastbird
{ 
	const char* GetCWD();
}

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
	fastbird::Error("Failed to call lua function. Error(%d)", error);\
	fastbird::PrintLuaErrorString(lua, errorString);\
	lua_pop(lua, 1); \
	assert(0);\
	return;\
}

#define LUA_PCALL_RET_FALSE(lua, arg, ret) if(int error = lua_pcall((lua), arg, ret, 0)) \
{\
	const char* errorString = lua_tostring(lua, -1);\
	fastbird::Error("Failed to call lua function. Error(%d)", error);\
	fastbird::PrintLuaErrorString(lua, errorString);\
	lua_pop(lua, 1); \
	assert(0); \
	return false; \
}

#define LUA_PCALL_NO_RET(lua, arg, ret) if(int error = lua_pcall((lua), arg, ret, 0)) \
{\
	const char* errorString = lua_tostring(lua, -1); \
	fastbird::Error("Failed to call lua function. Error(%d)", error);\
	fastbird::PrintLuaErrorString(lua, errorString);\
	lua_pop(lua, 1); \
	assert(0); \
}

namespace fastbird
{
	struct LUA_STACK_WATCHER
	{
		LUA_STACK_WATCHER(lua_State* L, const char* name)
			: lua(L), mName(name)
		{
			assert(name != 0);
			top = lua_gettop(L);
		}

		~LUA_STACK_WATCHER()
		{
			int now = lua_gettop(lua);
			if (top != now)
			{
				Log("LuaStackWather : %s", mName);
			}
			assert(top == now);
		}

		lua_State* lua;
		int top;
		const char* mName;
	};

	struct LUA_STACK_CLIPPER
	{
		LUA_STACK_CLIPPER(lua_State* L)
		{
			lua = L;
			top = lua_gettop(L);
		}

		~LUA_STACK_CLIPPER()
		{
			lua_settop(lua, top);
		}

		lua_State* lua;
		int top;
	};

	// Generic lua call
	void CallLuaFunction(lua_State* L, const char* func, const char* sig, ...);
	bool CheckLuaGlobalExist(lua_State* L, const char* name);
	void PrintLuaErrorString(lua_State* L, const char* luaString);
	void PrintLuaDebugInfo(lua_State* L, int level);
	std::string GetLuaValueAsString(lua_State* L, int stackIndex);
	std::string GetLuaVarAsString(lua_State* L, const char* varName, const char* luaFile=0);
	bool GetLuaVarAsBoolean(lua_State* L, const char* varName);
	Vec2I GetLuaVarAsVec2I(lua_State* L, const char* varname);
	void SetLuaVar(lua_State* L, const char* varName, bool value);
	
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