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
#include "AudioOptions.h"
#include "EngineFacade.h"
#include "FBConsole/Console.h"
using namespace fb;

AudioOptionsPtr AudioOptions::Create(){
	return AudioOptionsPtr(new AudioOptions, [](AudioOptions* obj){delete obj; });
}

AudioOptions::AudioOptions(){
	LuaLock L(LuaUtils::GetLuaState());
	a_MasterGain = Console::GetInstance().GetRealVariable(L, "a_MasterGain", 1.0);
	FB_REGISTER_CVAR(a_MasterGain, a_MasterGain, 
		CVAR_CATEGORY_CLIENT, "Audio Master Gain");
	EngineFacade::GetInstance().SetMasterGain(a_MasterGain, false);

	a_MusicGain = Console::GetInstance().GetRealVariable(L, "a_MusicGain", 1.0);
	FB_REGISTER_CVAR(a_MusicGain, a_MusicGain,
		CVAR_CATEGORY_CLIENT, "Music Gain");
	EngineFacade::GetInstance().SetMusicGain(a_MusicGain, false);

	a_SoundGain = Console::GetInstance().GetRealVariable(L, "a_SoundGain", 1.0);
	FB_REGISTER_CVAR(a_SoundGain, a_SoundGain,
		CVAR_CATEGORY_CLIENT, "Sound Gain");
	EngineFacade::GetInstance().SetSoundGain(a_SoundGain, false);
}

AudioOptions::~AudioOptions(){

}

bool AudioOptions::OnChangeCVar(CVarPtr pCVar){	
	if (pCVar->mName == "a_mastergain"){
		EngineFacade::GetInstance().SetMasterGain(pCVar->GetFloat(), true);
		return true;
	}
	else if (pCVar->mName == "a_musicgain"){
		EngineFacade::GetInstance().SetMusicGain(pCVar->GetFloat(), true);
		return true;
	}
	else if (pCVar->mName == "a_soundgain") {
		EngineFacade::GetInstance().SetSoundGain(pCVar->GetFloat(), true);
		return true;
	}	
	
	return false;
}