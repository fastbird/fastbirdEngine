#include "stdafx.h"
#include "SceneObjectFactoryOptions.h"
#include "FBConsole/Console.h"

namespace fb {

SceneObjectFactoryOptions::SceneObjectFactoryOptions() {
	o_rawCollada = Console::GetInstance().GetIntVariable("o_rawCollada", 0);
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