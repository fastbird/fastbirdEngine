#include <Engine/StdAfx.h>
#include <Engine/Misc/EngineCommand.h>
#include <Engine/IConsole.h>
#include <Engine/IParticleEmitter.h>
#include <Engine/ICamera.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/IScriptSystem.h>
namespace fastbird
{
void EditParticle(StringVector& arg);
static void Run(StringVector& arg);
static ConsoleCommand ccSpawnParticle("EditParticle", EditParticle, "EditParticle");
static ConsoleCommand ccRun("Run", Run, "Run command");

EngineCommand::EngineCommand()
{
	WheelSens = gFBEnv->pScriptSystem->GetRealVariable("WheelSens", 0.01f);
	REGISTER_CVAR(WheelSens, WheelSens, CVAR_CATEGORY_CLIENT, "WheelSensitivity");

	MouseSens = gFBEnv->pScriptSystem->GetRealVariable("MouseSens", 0.03f);
	REGISTER_CVAR(MouseSens, MouseSens, CVAR_CATEGORY_CLIENT, "MouseSensitivity");

	UI_Debug = gFBEnv->pScriptSystem->GetIntVariable("UI_Debug", 0);
	REGISTER_CVAR(UI_Debug, UI_Debug, CVAR_CATEGORY_CLIENT, "UI debug");

	e_NoMeshLoad = gFBEnv->pScriptSystem->GetIntVariable("e_NoMeshLoad", 0);
	REGISTER_CVAR(e_NoMeshLoad, e_NoMeshLoad, CVAR_CATEGORY_CLIENT, "NoMeshLoad");
	
	e_profile = gFBEnv->pScriptSystem->GetIntVariable("e_profile", 0);
	REGISTER_CVAR(e_profile, e_profile, CVAR_CATEGORY_CLIENT, "enable profiler");

	r_noObjectConstants = gFBEnv->pScriptSystem->GetIntVariable("r_noObjectConstants", 0);
	REGISTER_CVAR(r_noObjectConstants, r_noObjectConstants, CVAR_CATEGORY_CLIENT, "do not update object constans buffer");

	r_noParticleDraw = gFBEnv->pScriptSystem->GetIntVariable("r_noParticleDraw", 0);
	REGISTER_CVAR(r_noParticleDraw, r_noParticleDraw, CVAR_CATEGORY_CLIENT, "No particle Draw");

	r_particleProfile = gFBEnv->pScriptSystem->GetIntVariable("r_particleProfile", 0);
	REGISTER_CVAR(r_particleProfile, r_particleProfile, CVAR_CATEGORY_CLIENT, "enable profiler");

	r_HDR = gFBEnv->pScriptSystem->GetIntVariable("r_HDR", 1);
	REGISTER_CVAR(r_HDR, r_HDR, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_HDRMiddleGray = gFBEnv->pScriptSystem->GetRealVariable("r_HDRMiddleGray", 0.5f);
	REGISTER_CVAR(r_HDRMiddleGray, r_HDRMiddleGray, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_BloomPower = gFBEnv->pScriptSystem->GetRealVariable("r_BloomPower", 0.1f);
	REGISTER_CVAR(r_BloomPower, r_BloomPower, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_StarPower = gFBEnv->pScriptSystem->GetRealVariable("r_StarPower", 0.2f);
	REGISTER_CVAR(r_StarPower, r_StarPower, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_GodRay = gFBEnv->pScriptSystem->GetIntVariable("r_GodRay", 1);
	REGISTER_CVAR(r_GodRay, r_GodRay, CVAR_CATEGORY_CLIENT, "enable GodRay rendering");

	r_GodRayWeight = gFBEnv->pScriptSystem->GetRealVariable("r_GodRayWeight", 0.1f);
	REGISTER_CVAR(r_GodRayWeight, r_GodRayWeight, CVAR_CATEGORY_CLIENT, "enable GodRay rendering");

	r_GodRayDecay = gFBEnv->pScriptSystem->GetRealVariable("r_GodRayDecay", 0.9f);
	REGISTER_CVAR(r_GodRayDecay, r_GodRayDecay, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_GodRayDensity = gFBEnv->pScriptSystem->GetRealVariable("r_GodRayDensity", 0.2f);
	REGISTER_CVAR(r_GodRayDensity, r_GodRayDensity, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_GodRayExposure = gFBEnv->pScriptSystem->GetRealVariable("r_GodRayExposure", 0.8f);
	REGISTER_CVAR(r_GodRayExposure, r_GodRayExposure, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_ReportDeviceObjectLeak = gFBEnv->pScriptSystem->GetIntVariable("r_ReportDeviceObjectLeak");
	REGISTER_CVAR(r_ReportDeviceObjectLeak, r_ReportDeviceObjectLeak, CVAR_CATEGORY_CLIENT, "ReportResourceLeak");

	r_Glow = gFBEnv->pScriptSystem->GetIntVariable("r_Glow", 1);
	REGISTER_CVAR(r_Glow, r_Glow, CVAR_CATEGORY_CLIENT, "Glow");


	r_UI = gFBEnv->pScriptSystem->GetIntVariable("r_UI", 1);
	REGISTER_CVAR(r_UI, r_UI, CVAR_CATEGORY_CLIENT, "UI Rendering");

	r_Shadow = gFBEnv->pScriptSystem->GetIntVariable("r_Shadow", 1);
	REGISTER_CVAR(r_Shadow, r_Shadow, CVAR_CATEGORY_CLIENT, "enable shadow");

	REGISTER_CVAR(MoveEditParticle, 0, CVAR_CATEGORY_CLIENT, "MoveEditParticle");

	REGISTER_CC(&ccSpawnParticle);
	REGISTER_CC(&ccRun);
}
EngineCommand::~EngineCommand()
{
	for each (auto p in mCVars)
	{
		FB_SAFE_DEL(p);
	}

	for each (auto p in mCommands)
	{
		gFBEnv->pConsole->UnregisterCommand(p);
	}
}

void EditParticle(StringVector& arg)
{
	if (arg.size() < 2)
		return;

	ParticleManager::GetParticleManager().EditThisParticle(arg[1].c_str());
}

void Run(StringVector& arg)
{
	if (arg.size() < 2)
		return;

	std::ifstream file(arg[1]);
	if (file.is_open())
	{
		while (file.good())
		{
			char buffer[512];
			file.getline(buffer, 512);
			gFBEnv->pConsole->ProcessCommand(buffer);
		}
	}
	
}

}

