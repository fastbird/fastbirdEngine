/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "FBLua/LuaObject.h"
#include "FBLuaMathLib/FBLuaMath.h"
#include "FBRenderer/Renderer.h"
#include "FBSceneManager/SpatialObject.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBEngineFacade/MeshFacade.h"
#include "FBSceneManager/Scene.h"
#include "FBConsole/Console.h"
#include "FBFileSystem/FileSystem.h"
namespace fb{

	int FileExists(lua_State* L) {
		auto filepath = LuaUtils::checkstring(L, 1);
		auto ret = FileSystem::Exists(filepath);
		LuaUtils::pushboolean(L, ret);
		return 1;
	}

	int GetMasterGain(lua_State* L) {		
		auto masterGain = EngineFacade::GetInstance().GetMasterGain();
		LuaUtils::pushnumber(L, masterGain);
		return 1;
	}

	int GetMusicGain(lua_State* L) {		
		auto musicGain = EngineFacade::GetInstance().GetMusicGain();
		LuaUtils::pushnumber(L, musicGain);
		return 1;

	}

	int GetSoundGain(lua_State* L) {				
		auto soundGain = EngineFacade::GetInstance().GetSoundGain();
		LuaUtils::pushnumber(L, soundGain);
		return 1;
	}

	int SetMasterGain(lua_State* L) {
		auto volume = (float)LuaUtils::checknumber(L, 1);
		EngineFacade::GetInstance().SetMasterGain(volume, true);
		return 0;
	}
	int SetMusicGain(lua_State* L) {
		auto volume = (float)LuaUtils::checknumber(L, 1);
		EngineFacade::GetInstance().SetMusicGain(volume, true);
		return 0;
	}

	int SetSoundGain(lua_State* L) {
		auto volume = (float)LuaUtils::checknumber(L, 1);
		EngineFacade::GetInstance().SetSoundGain(volume, true);
		return 0;
	}

	int GetResolution(lua_State* L)
	{
		const auto& size = Renderer::GetInstance().GetMainRenderTargetSize();
		luaU_push<Vec2I>(L, size);
		return 1;
	}

	int GetFrameCounter(lua_State* L)
	{
		LuaUtils::pushunsigned(L, gpTimer->GetFrame());
		return 1;
	}

	int PrintSpatialObject(lua_State* L)
	{
		auto scene = EngineFacade::GetInstance().GetMainScene();
		if (scene)
		{
			//Logger::Log(FB_DEFAULT_LOG_ARG, "PrintSpatialObject() is under construction.");
			scene->PrintSpatialObject();
		}
		return 0;
	}

	int InvalidateMouseKeyboard(lua_State* L)
	{
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		if (injector)
		{
			injector->Invalidate(InputDevice::Keyboard);
			injector->ClearBuffer();
			injector->InvalidateClickTime();
			injector->ClearWheel();
		}
		return 0;
	}

	int IsKeyPressed(lua_State* L)
	{
		unsigned short keyCode = LuaUtils::checkunsigned(L, 1);
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		bool pressed = injector->IsKeyPressed(keyCode);
		LuaUtils::pushboolean(L, pressed);
		return 1;
	}
	int IsKeyDown(lua_State* L)
	{
		unsigned short keyCode = LuaUtils::checkunsigned(L, 1);
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		bool down = injector->IsKeyDown(keyCode);
		LuaUtils::pushboolean(L, down);
		return 1;
	}
	int IsLButtonClicked(lua_State* L)
	{
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		LuaUtils::pushboolean(L, injector->IsLButtonClicked());
		return 1;
	}
	int IsRButtonClicked(lua_State* L)
	{
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		LuaUtils::pushboolean(L, injector->IsRButtonClicked());
		return 1;
	}
	int IsLButtonDown(lua_State* L)
	{
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		LuaUtils::pushboolean(L, injector->IsLButtonDown());
		return 1;
	}

	int GenGGX(lua_State* L) {
		Logger::Log(FB_ERROR_LOG_ARG, "GenGGX() lua function is Deprecated.");
		return 0;
	}

	int LoadMesh(lua_State* L) {
		auto meshPath = LuaUtils::checkstring(L, 1);
		if (strlen(meshPath) != 0) {
			MeshFacadePtr mesh = MeshFacade::Create()->LoadMeshObject(meshPath);
			if (mesh) {
				mesh->AttachToScene();
				EngineFacade::GetInstance().SetMainCameraTarget(mesh->GetSpatialObject());
				EngineFacade::GetInstance().EnableCameraInput(true);
			}
			else {
				EngineFacade::GetInstance().EnableCameraInput(false);
			}
		}
		else {
			EngineFacade::GetInstance().EnableCameraInput(false);
		}
		return 0;
	}

	int FBConsole(lua_State* L) {
		auto str = LuaUtils::checkstring(L, 1);
		EngineFacade::GetInstance().QueueProcessConsoleCommand(str, false);
		return 0;
	}

	int FBGetConsoleValue(lua_State* L) {
		auto str = LuaUtils::checkstring(L, 1);
		auto var = Console::GetInstance().GetVariable(str);
		switch(var->mType) {
		case CVAR_TYPE_INT:
			LuaUtils::pushinteger(L, var->GetInt());
			break;
		case CVAR_TYPE_REAL:
			LuaUtils::pushnumber(L, var->GetFloat());
			break;
		case CVAR_TYPE_STRING:
			LuaUtils::pushstring(L, var->GetString().c_str());
			break;
		case CVAR_TYPE_VEC2I:
			LuaUtils::pushVec2I(L, var->GetVec2I());
			break;
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid console variable: %s", str).c_str());
			return 0;
		}
		return 1;
	}

	void InitEngineLua(lua_State* L){		
		LuaUtils::LoadConfig(L, "configEngine.lua");
		LUA_SETCFUNCTION(L, FBGetConsoleValue);		
		LUA_SETCFUNCTION(L, FBConsole);
		LUA_SETCFUNCTION(L, LoadMesh);
		LUA_SETCFUNCTION(L, GenGGX);
		LUA_SETCFUNCTION(L, IsKeyPressed);
		LUA_SETCFUNCTION(L, IsKeyDown);
		LUA_SETCFUNCTION(L, IsLButtonClicked);
		LUA_SETCFUNCTION(L, IsRButtonClicked);
		LUA_SETCFUNCTION(L, IsLButtonDown);
		LUA_SETCFUNCTION(L, PrintSpatialObject); 
		LUA_SETCFUNCTION(L, GetResolution);
		LUA_SETCFUNCTION(L, GetFrameCounter);
		LUA_SETCFUNCTION(L, InvalidateMouseKeyboard);
		LUA_SETCFUNCTION(L, SetMasterGain);
		LUA_SETCFUNCTION(L, SetMusicGain);
		LUA_SETCFUNCTION(L, SetSoundGain);
		LUA_SETCFUNCTION(L, GetMasterGain);
		LUA_SETCFUNCTION(L, GetMusicGain);
		LUA_SETCFUNCTION(L, GetSoundGain);
		LUA_SETCFUNCTION(L, FileExists);
	}
}