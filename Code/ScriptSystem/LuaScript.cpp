#include "StdAfx.h"
#include "LuaScript.h"

namespace fastbird
{

void LuaScript::LuaError(int error)
{
	if (error)
	{
		switch(error)
		{
		case LUA_ERRFILE:
			{
				std::cerr << "Could not read a file." << std::endl;
			}
			break;
		default:
			{
				std::cerr << "Lua error!" << std::endl;
				int a=0;
				a++;
			}
		}
	}
}

LuaScript::LuaScript(const char* filename)
{
	L = luaL_newstate();
}

LuaScript::~LuaScript()
{
	lua_close(L);
}

bool LuaScript::LoadScriptFile(const char* filename)
{
	assert(filename);
	mFilename = filename;
	int error = luaL_loadfile(L, filename);
	if (error)
	{
		LuaError(error);
		return false;
	}

	error = lua_pcall(L, 0, 0, 0);
	if (error)
	{
		LuaError(error);
		return false;
	}
	return true;
}

}