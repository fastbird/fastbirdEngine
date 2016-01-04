#include "stdafx.h"
#include "AudioOptions.h"
#include "EngineFacade.h"
#include "FBConsole/Console.h"
using namespace fb;

AudioOptionsPtr AudioOptions::Create(){
	return AudioOptionsPtr(new AudioOptions, [](AudioOptions* obj){delete obj; });
}

AudioOptions::AudioOptions(){
	a_MusicGain = Console::GetInstance().GetRealVariable("a_MusicGain", 1.0);
	FB_REGISTER_CVAR(a_MusicGain, a_MusicGain, 
		CVAR_CATEGORY_CLIENT, "Music Gain");
	EngineFacade::GetInstance().SetMusicGain(a_MusicGain);

	a_MasterGain = Console::GetInstance().GetRealVariable("a_MasterGain", 1.0);
	FB_REGISTER_CVAR(a_MasterGain, a_MasterGain, 
		CVAR_CATEGORY_CLIENT, "Audio Master Gain");
	EngineFacade::GetInstance().SetMasterGain(a_MasterGain);

	a_Enabled = Console::GetInstance().GetIntVariable("a_Enabled", 1);
	FB_REGISTER_CVAR(a_Enabled, a_Enabled,
		CVAR_CATEGORY_CLIENT, "Audio enabled");
	EngineFacade::GetInstance().SetEnabled(a_Enabled ? true : false);
}

AudioOptions::~AudioOptions(){

}

bool AudioOptions::OnChangeCVar(CVarPtr pCVar){
	if (pCVar->mName == "a_musicgain"){
		EngineFacade::GetInstance().SetMusicGain(pCVar->GetFloat());
		return true;
	}
	else if (pCVar->mName == "a_mastergain"){
		EngineFacade::GetInstance().SetMasterGain(pCVar->GetFloat());
		return true;
	}
	else if (pCVar->mName == "a_enabled"){
		EngineFacade::GetInstance().SetEnabled(pCVar->GetInt() ? true : false);
		return true;
	}
	return false;
}