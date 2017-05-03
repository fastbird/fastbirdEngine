#include "stdafx.h"
#include "SceneManagerOptions.h"
#include "FBLua/LuaUtils.h"
#include "FBConsole/Console.h"

using namespace fb;

SceneManagerOptionsPtr SceneManagerOptions::Create() {
	return std::make_shared<SceneManagerOptions>();
}

SceneManagerOptions::SceneManagerOptions() {
	LuaLock L(LuaUtils::GetLuaState());
	r_noPointLight = Console::GetInstance().GetIntVariable(L, "r_noPointLight", 0);
	FB_REGISTER_CVAR(r_noPointLight, r_noPointLight, CVAR_CATEGORY_CLIENT, "do not update object constans buffer");
}

bool SceneManagerOptions::OnChangeCVar(CVarPtr pCVar) {
	return false;
}