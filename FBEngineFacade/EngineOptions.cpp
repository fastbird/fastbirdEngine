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
#include "EngineOptions.h"
#include "EngineFacade.h"
#include "FBConsole/Console.h"
#include "FBThread/TaskScheduler.h"
#include "FBFileSystem/FileSystem.h"
using namespace fb;
void NumTasks(StringVector& args);
void EngineCrashTest(StringVector& args);

EngineOptionsPtr EngineOptions::Create(lua_State* L) {
	return std::make_shared<EngineOptions>(L);
}

EngineOptions::EngineOptions(lua_State* L){
	if (!Console::HasInstance()){
		Logger::Log(FB_ERROR_LOG_ARG, "Console is not initialized! Engine Options won't work.");
		return;
	}
	WheelSens = Console::GetInstance().GetRealVariable(L, "WheelSens", 0.005f);
	FB_REGISTER_CVAR(WheelSens, WheelSens, CVAR_CATEGORY_CLIENT, "WheelSensitivity");

	MouseSens = Console::GetInstance().GetRealVariable(L, "MouseSens", 0.03f);
	FB_REGISTER_CVAR(MouseSens, MouseSens, CVAR_CATEGORY_CLIENT, "MouseSensitivity");	

	e_profile = Console::GetInstance().GetIntVariable(L, "e_profile", 0);
	FB_REGISTER_CVAR(e_profile, e_profile, CVAR_CATEGORY_CLIENT, "Display profile information");

	e_NoMeshLoad = Console::GetInstance().GetIntVariable(L, "e_NoMeshLoad", 0);
	FB_REGISTER_CVAR(e_NoMeshLoad, e_NoMeshLoad, CVAR_CATEGORY_CLIENT, "Skip mesh loading");

	AudioDebug = Console::GetInstance().GetIntVariable(L, "AudioDebug", 0);
	FB_REGISTER_CVAR(AudioDebug, AudioDebug, CVAR_CATEGORY_CLIENT, "Audio debug");
	
	FB_REGISTER_CC(NumTasks, "NumTasks");
	FB_REGISTER_CC(EngineCrashTest, "EngineCrashTest");
}

EngineOptions::~EngineOptions(){

}

void NumTasks(StringVector& args) {
	auto str = FormatString("Num taks = %u", TaskScheduler::GetInstance().GetNumTasks());
	Logger::Log(FB_DEFAULT_LOG_ARG, str.c_str());
	Console::GetInstance().Log(str.c_str());
}

bool EngineOptions::OnChangeCVar(CVarPtr pCVar) {

	return false;
}

void EngineCrashTest(StringVector& args) {
	Logger::Log(FB_ERROR_LOG_ARG, "Engine Crash test!");
	int* a = 0;
	*a = 1;
}