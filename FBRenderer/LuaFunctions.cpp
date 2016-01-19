#include "stdafx.h"
#include "LuaFunctions.h"
#include "Renderer.h"
#include "FBLua/LuaUtils.h"
namespace fb{
	int KeepTextureReference(lua_State* L);

	void RegisterRendererLuaFunctions(){
		auto L = LuaUtils::GetLuaState();
		LUA_SETCFUNCTION(L, KeepTextureReference);
	}

	int KeepTextureReference(lua_State* L){
		auto textureFile = LuaUtils::checkstring(L, 1);
		Renderer::GetInstance().KeepTextureReference(textureFile);		
		return 0;
	}
}