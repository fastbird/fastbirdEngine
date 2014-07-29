#include <Engine/StdAfx.h>
#include <Engine/ScriptSystem/ScriptSystem.h>
#include <Engine/GlobalEnv.h>

namespace fastbird
{

//-------------------------------------------------------------------------
ScriptSystem::ScriptSystem()
{
	mLuaState = luaL_newstate();
	luaL_openlibs(mLuaState);
	assert(mLuaState);

	RunScript("configEngine.lua");
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
void ScriptSystem::InitLua()
{
	mLuaState = luaL_newstate();
	luaL_openlibs(mLuaState);
}

//--------------------------------------------------------------------------
void ScriptSystem::CloseLua()
{
	lua_close(mLuaState);
}

//--------------------------------------------------------------------------
bool ScriptSystem::ExecuteLua(const std::string& chunk)
{
	int error;
	error = luaL_loadbuffer(mLuaState, chunk.c_str(), chunk.size(), "line") ||
		lua_pcall(mLuaState, 0, 0, 0);
	if (error)
	{
		std::string err = lua_tostring(mLuaState, -1);
		lua_pop(mLuaState, 1);
		if (gFBEnv->pEngine)
		{
			gFBEnv->pEngine->Error(err.c_str());
		}
		else
		{
			std::cerr << err << std::endl;
		}
		return false;
	}
	return true;
}

std::string ScriptSystem::GetStringVariable(const char* name, 
		const std::string& def)
{
	std::string ret = def;
	if (name==0 || strlen(name)==0)
		return ret;

	fastbird::LUA_STACK_WATCHER watcher(mLuaState);
	lua_getglobal(mLuaState, name);
	const char* str = lua_tostring(mLuaState, -1);
	if (str)
	{
		ret = str;
	}

	lua_pop(mLuaState, 1);
	return ret;
}

}