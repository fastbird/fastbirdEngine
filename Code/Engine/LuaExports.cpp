#include <Engine/StdAfx.h>
#include <Engine/ICamera.h>
#include <Engine/IMeshObject.h>
#include <Engine/IRenderTarget.h>
#include <CommonLib/luawrapper.hpp>
#include <CommonLib/LuaObject.h>

using namespace fastbird;

namespace fastbird
{
	int GetResolution(lua_State* L);
	int GetFrameCounter(lua_State* L);
	int PrintSpatialObject(lua_State* L);
	int InvalidateMouseKeyboard(lua_State* L);
	int FBSetKeyMouseEventOverride(lua_State* L);

	int IsKeyPressed(lua_State* L);
	int IsKeyDown(lua_State* L);
	int IsLButtonClicked(lua_State* L);
	int IsRButtonClicked(lua_State* L);
	int IsLButtonDown(lua_State* L);

	int GenGGX(lua_State* L);
	int LoadMesh(lua_State* L);

	void InitEngineLuaFuncs(lua_State* L)
	{
		LUA_SETCFUNCTION(L, LoadMesh);
		LUA_SETCFUNCTION(L, GenGGX);
		LUA_SETCFUNCTION(L, IsKeyPressed);
		LUA_SETCFUNCTION(L, IsKeyDown);
		LUA_SETCFUNCTION(L, IsLButtonClicked);
		LUA_SETCFUNCTION(L, IsRButtonClicked);
		LUA_SETCFUNCTION(L, IsLButtonDown);

		LUA_SETCFUNCTION(L, FBSetKeyMouseEventOverride);
		LUA_SETCFUNCTION(L, PrintSpatialObject);
		LUA_SETCFUNCTION(L, GetResolution);
		LUA_SETCFUNCTION(L, GetFrameCounter);
		LUA_SETCFUNCTION(L, InvalidateMouseKeyboard);
	}

	int GetResolution(lua_State* L)
	{
		const auto& size = gFBEnv->pRenderer->GetMainRTSize();
		luaU_push<Vec2I>(L, size);
		return 1;
	}

	int GetFrameCounter(lua_State* L)
	{
		lua_pushunsigned(L, gFBEnv->mFrameCounter);
		return 1;
	}
	
	int PrintSpatialObject(lua_State* L)
	{
		if (gFBEnv)
		{
			auto scene = gFBEnv->pRenderer->GetMainScene();
			if (scene)
			{
				scene->PrintSpatialObject();
			}
		}
		return 0;
	}

	int InvalidateMouseKeyboard(lua_State* L)
	{
		if (gFBEnv)
		{
			auto keyboard = gFBEnv->pEngine->GetKeyboard();
			auto mouse = gFBEnv->pEngine->GetMouse();

			if (keyboard)
			{
				keyboard->Invalidate();
				keyboard->ClearBuffer();
			}
			if (mouse)
			{
				mouse->Invalidate();
				mouse->ClearWheel();
			}
		}
		return 0;
	}

	int FBSetKeyMouseEventOverride(lua_State* L)
	{
		LuaObject overrideFunc(L, 1);
		gFBEnv->pEngine->SetInputOverride(overrideFunc);
		return 0;
	}

	int IsKeyPressed(lua_State* L)
	{
		unsigned short keyCode = luaL_checkunsigned(L, 1);
		bool pressed = gFBEnv->pEngine->GetKeyboard()->IsKeyPressed(keyCode);
		lua_pushboolean(L, pressed);
		return 1;
	}
	int IsKeyDown(lua_State* L)
	{
		unsigned short keyCode = luaL_checkunsigned(L, 1);
		bool down = gFBEnv->pEngine->GetKeyboard()->IsKeyDown(keyCode);
		lua_pushboolean(L, down);
		return 1;
	}
	int IsLButtonClicked(lua_State* L)
	{
		lua_pushboolean(L, gFBEnv->pEngine->GetMouse()->IsLButtonClicked());
		return 1;
	}
	int IsRButtonClicked(lua_State* L)
	{
		lua_pushboolean(L, gFBEnv->pEngine->GetMouse()->IsRButtonClicked());
		return 1;
	}
	int IsLButtonDown(lua_State* L)
	{
		lua_pushboolean(L, gFBEnv->pEngine->GetMouse()->IsLButtonDown());
		return 1;
	}
	
	int GenGGX(lua_State* L){
		gFBEnv->pRenderer->GenGGX();
		return 0;
	}
	static IMeshObject* gTempMesh = 0;
	int LoadMesh(lua_State* L){
		if (gTempMesh){
			gFBEnv->pEngine->ReleaseMeshObject(gTempMesh);
			gTempMesh = 0;
		}

		auto meshPath = luaL_checkstring(L, 1);
		gTempMesh = gFBEnv->pEngine->GetMeshObject(meshPath);
		if (gTempMesh){
			gFBEnv->pRenderer->GetMainRenderTarget()->GetScene()->AttachObject(gTempMesh);
			auto mainCam = gFBEnv->pRenderer->GetMainRenderTarget()->GetOrCreateOverridingCamera();
			mainCam->SetTarget(gTempMesh);
			mainCam->SetEnalbeInput(true);
		}
		else{
			gFBEnv->pRenderer->GetMainRenderTarget()->RemoveOverridingCamera();
		}
		return 0;
	}
	
}
