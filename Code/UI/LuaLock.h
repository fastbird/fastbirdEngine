#pragma once
#include <Engine/IScriptSystem.h>
namespace fastbird{
	struct LuaLock{
		LuaLock();
		~LuaLock();

		inline operator lua_State*() const { return gFBEnv->pScriptSystem->GetLuaState(); }
	};
}