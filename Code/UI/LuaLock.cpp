#include <UI/StdAfx.h>
#include <UI/LuaLock.h>
namespace fastbird{
	LuaLock::LuaLock(){
		gFBEnv->pScriptSystem->LockLua();
	}
	LuaLock::~LuaLock(){
		gFBEnv->pScriptSystem->UnlockLua();
	}
}