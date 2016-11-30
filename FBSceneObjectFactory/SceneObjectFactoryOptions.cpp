#include "stdafx.h"
#include "SceneObjectFactoryOptions.h"
#include "FBLua/LuaUtils.h"
#include "FBConsole/Console.h"

namespace fb {

SceneObjectFactoryOptions::SceneObjectFactoryOptions() {
	LuaLock L(LuaUtils::GetLuaState());
	o_rawCollada = Console::GetInstance().GetIntVariable(L, "o_rawCollada", 0);
	FB_REGISTER_CVAR(o_rawCollada, o_rawCollada, CVAR_CATEGORY_CLIENT, "Use .dae directly.");
}
SceneObjectFactoryOptions::~SceneObjectFactoryOptions() {

}

bool SceneObjectFactoryOptions::OnChangeCVar(CVarPtr pCVar) {
	return false;
}

SceneObjectFactoryOptionsPtr SceneObjectFactoryOptions::Create() {
	return SceneObjectFactoryOptionsPtr(new SceneObjectFactoryOptions,
		[](SceneObjectFactoryOptions* obj) {delete obj; });
}

}