#include <Engine/StdAfx.h>
#include <Engine/ScriptSystem.h>
#include <Engine/GlobalEnv.h>
#include <CommonLib/LuaUtils.h>

int _FBPrint(lua_State* L)
{
	std::string msg = std::string("Lua: ") + luaL_checkstring(L, -1);
	fastbird::Log(msg.c_str());

	return 0;
}

namespace fastbird
{
	void InitEngineLuaFuncs(lua_State* L);
//-------------------------------------------------------------------------
ScriptSystem::ScriptSystem()
{
	mLuaState = luaL_newstate();
	luaL_openlibs(mLuaState);
	assert(mLuaState);
	ExportsDefaultFunctions();
	RunScript("configEngine.lua");
	InitEngineLuaFuncs(mLuaState);
}

//-------------------------------------------------------------------------
ScriptSystem::~ScriptSystem()
{
	lua_close(mLuaState);
}

//-------------------------------------------------------------------------
lua_State* ScriptSystem::GetLuaState() const
{
	return mLuaState;
}

//--------------------------------------------------------------------------
bool ScriptSystem::RunScript(const char* filename)
{
	int error = luaL_dofile(mLuaState, filename);
	if (error)
	{
		gFBEnv->pEngine->Error(lua_tostring(mLuaState, -1));
		gFBEnv->pEngine->Error("RunScript error!");
		return false;
	}
	return true;
}

//--------------------------------------------------------------------------
bool ScriptSystem::ExecuteLua(const std::string& chunk)
{
	int error;
	error = luaL_loadbuffer(mLuaState, chunk.c_str(), chunk.size(), "line") ||
		lua_pcall(mLuaState, 0, 0, 0);
	if (error)
	{
		const char* errorString = lua_tostring(mLuaState, -1);
		lua_pop(mLuaState, 1);
		if (gFBEnv->pEngine)
		{
			PrintLuaErrorString(mLuaState, errorString);
		}
		else
		{
			std::cerr << errorString << std::endl;
		}
		return false;
	}
	return true;
}

std::string ScriptSystem::GetStringVariable(const char* name, const std::string& def)
{
	std::string ret = def;
	if (name==0 || strlen(name)==0)
		return ret;

	fastbird::LUA_STACK_WATCHER watcher(mLuaState, "std::string ScriptSystem::GetStringVariable(const char* name, const std::string& def)");
	lua_getglobal(mLuaState, name);
	const char* str = lua_tostring(mLuaState, -1);
	if (str)
	{
		ret = str;
	}

	lua_pop(mLuaState, 1);
	return ret;
}

int ScriptSystem::GetIntVariable(const char* name, int def/* = 0*/)
{
	int ret = def;
	if (name == 0 || strlen(name) == 0)
		return ret;

	fastbird::LUA_STACK_WATCHER watcher(mLuaState, "int ScriptSystem::GetIntVariable(const char* name, int def/* = 0*/)");
	lua_getglobal(mLuaState, name);
	if (!lua_isnil(mLuaState, -1))
		ret = lua_tointeger(mLuaState, -1);
	lua_pop(mLuaState, 1);
	return ret;
}

float ScriptSystem::GetRealVariable(const char* name, float def)
{
	float ret = def;
	if (name == 0 || strlen(name) == 0)
		return ret;

	fastbird::LUA_STACK_WATCHER watcher(mLuaState, "float ScriptSystem::GetRealVariable(const char* name, float def)");
	int top = lua_gettop(mLuaState);
	lua_getglobal(mLuaState, name);
	int top2 = lua_gettop(mLuaState);
	if (!lua_isnil(mLuaState, -1))
		ret = (float)lua_tonumber(mLuaState, -1);
	lua_pop(mLuaState, 1);
	return ret;
}

void ScriptSystem::ExportsDefaultFunctions()
{
	LUA_SETCFUNCTION(mLuaState, _FBPrint);
}

}