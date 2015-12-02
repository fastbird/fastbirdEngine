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

#include "StdAfx.h"
#include "UICommands.h"
#include "UIManager.h"
#include "FBConsole/ConsoleDataType.h"
#include "FBConsole/Console.h"
using namespace fb;

static void StartUIEditor(StringVector& arg);
static void KillUIEditor(StringVector& arg);

UICommandsPtr UICommands::Create(){
	UICommandsPtr p(new UICommands, [](UICommands* obj){ delete obj; });
	return p;
}

UICommands::UICommands()
{
	FB_REGISTER_CC("StartUIEditor", StartUIEditor, "Start ui editor");
	FB_REGISTER_CC("KillUIEditor", KillUIEditor, "Kill ui editor");

	r_UI = Console::GetInstance().GetIntVariable("r_UI", 1);
	FB_REGISTER_CVAR(r_UI, r_UI, CVAR_CATEGORY_CLIENT, "Render uis.");

	UI_Debug = Console::GetInstance().GetIntVariable("UI_Debug", 0);
	FB_REGISTER_CVAR(UI_Debug, UI_Debug, CVAR_CATEGORY_CLIENT, "UI debug");
}

UICommands::~UICommands()
{
	
}

bool UICommands::OnChangeCVar(CVarPtr pCVar)
{
	return false;
}

//typedef int(__cdecl *StartProc)(GlobalEnv* pEnv);
static bool uiEditorInitialized = false;
void StartUIEditor(StringVector& arg)
{
	/*if (uiEditorInitialized)
	{
		Log("Alreay started!");
		return;
	}
	auto moduleHandle = gFBEnv->pUIManager->GetUIEditorModuleHandle();
	if (!moduleHandle)
	{
		moduleHandle = fb::LoadFBLibrary("FBUIEditor.dll");
		gFBEnv->pUIManager->SetUIEditorModuleHandle(moduleHandle);
	}
	if (moduleHandle)
	{		
		StartProc startFunc;
		startFunc = (StartProc)GetProcAddress(moduleHandle, "StartUIEditor");
		if (startFunc)
		{
			gFBEnv->pUIManager->SetUIEditorModuleHandle(moduleHandle);
			startFunc(gFBEnv);
			uiEditorInitialized = true;
			LuaLock L;
			lua_pushboolean(L, 1);
			lua_setglobal(L, "gThreatHold");
		}
	}*/
}

typedef void(__cdecl *FinalizeProc)();
void KillUIEditor(StringVector& arg)
{
	//if (!uiEditorInitialized)
	//	return;

	//auto moduleHandle = gFBEnv->pUIManager->GetUIEditorModuleHandle();
	//if (moduleHandle)
	//{
	//	FinalizeProc finalizeFunc;
	//	finalizeFunc = (FinalizeProc)GetProcAddress(moduleHandle, "KillUIEditor");
	//	if (finalizeFunc)
	//		finalizeFunc();
	//	LuaLock L;
	//	lua_pushboolean(L, 0);
	//	lua_setglobal(L, "gThreatHold");

	//	//FreeLibrary(gFBEnv->pUIManager->GetUIEditorModuleHandle());
	//	//gFBEnv->pUIManager->SetUIEditorModuleHandle(0);
	//}
	//uiEditorInitialized = false;
}