#include <Engine/StdAfx.h>
#include <Engine/EngineCommand.h>
#include <Engine/IConsole.h>
#include <Engine/IParticleEmitter.h>
#include <Engine/ICamera.h>
#include <Engine/ParticleManager.h>
#include <Engine/IScriptSystem.h>
#pragma init_seg(user) 
namespace fastbird
{
static void EditParticle(StringVector& arg);
static void ScaleEditingParticle(StringVector& arg);
static void Run(StringVector& arg);
static void DebugRenderTarget(StringVector& arg);
static void SetFov(StringVector& arg);
static void Clear(StringVector& arg);
static ConsoleCommand ccSpawnParticle("EditParticle", EditParticle, "EditParticle");
static ConsoleCommand ccScaleEditingParticle("ScaleEditingParticle", ScaleEditingParticle, "ScaleEditingParticle");
static ConsoleCommand ccRun("Run", Run, "Run command");
static ConsoleCommand ccDebugRenderTarget("DebugRenderTarget", DebugRenderTarget, "DebugRenderTarget");
static ConsoleCommand ccSetFov("SetFov", SetFov, "SetFov");
static ConsoleCommand ccClear("Clear", Clear, "Clear console messsage");
EngineCommand::EngineCommand()
{
	WheelSens = gFBEnv->pScriptSystem->GetRealVariable("WheelSens", 0.005f);
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

	r_HDRMiddleGray = gFBEnv->pScriptSystem->GetRealVariable("r_HDRMiddleGray", 0.25);
	REGISTER_CVAR(r_HDRMiddleGray, r_HDRMiddleGray, CVAR_CATEGORY_CLIENT, "Decide scene key.");

	r_HDRCpuLuminance = gFBEnv->pScriptSystem->GetIntVariable("r_HDRCpuLuminance", 0);
	REGISTER_CVAR(r_HDRCpuLuminance, r_HDRCpuLuminance, CVAR_CATEGORY_CLIENT, "If true, luminance will be calculated in CPU. Debug purpose");

	r_HDRFilmic = gFBEnv->pScriptSystem->GetIntVariable("r_HDRFilmic", 1);
	REGISTER_CVAR(r_HDRFilmic, r_HDRFilmic, CVAR_CATEGORY_CLIENT, "Use Filmic tone mapping");

	r_BloomPower = gFBEnv->pScriptSystem->GetRealVariable("r_BloomPower", 0.2f);
	REGISTER_CVAR(r_BloomPower, r_BloomPower, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_StarPower = gFBEnv->pScriptSystem->GetRealVariable("r_StarPower", 0.4f);
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

	r_ShadowMapWidth = gFBEnv->pScriptSystem->GetIntVariable(
		"r_ShadowMapWidth", 4096);
	REGISTER_CVAR(r_ShadowMapWidth, r_ShadowMapWidth, CVAR_CATEGORY_CLIENT,
		"ShadowMap width");

	r_ShadowMapHeight = gFBEnv->pScriptSystem->GetIntVariable(
		"r_ShadowMapHeight", 4096);
	REGISTER_CVAR(r_ShadowMapHeight, r_ShadowMapHeight, CVAR_CATEGORY_CLIENT,
		"ShadowMap height");

	r_ShadowCamWidth = gFBEnv->pScriptSystem->GetRealVariable(
		"r_ShadowCamWidth", 150.f);
	REGISTER_CVAR(r_ShadowCamWidth, r_ShadowCamWidth, CVAR_CATEGORY_CLIENT,
		"ShadowMap width");

	r_ShadowCamHeight = gFBEnv->pScriptSystem->GetRealVariable(
		"r_ShadowCamHeight", 150.f);
	REGISTER_CVAR(r_ShadowCamHeight, r_ShadowCamHeight, CVAR_CATEGORY_CLIENT,
		"ShadowMap height");

	r_ShadowNear = gFBEnv->pScriptSystem->GetRealVariable(
		"r_ShadowNear", 2.0f);
	REGISTER_CVAR(r_ShadowNear, r_ShadowNear, CVAR_CATEGORY_CLIENT, 
		"Shadow camera near.");

	r_ShadowFar = gFBEnv->pScriptSystem->GetRealVariable(
		"r_ShadowFar", 200.0f);
	REGISTER_CVAR(r_ShadowFar, r_ShadowFar, CVAR_CATEGORY_CLIENT, 
		"Shadow camera far");

	r_ShadowCamDist = gFBEnv->pScriptSystem->GetRealVariable(
		"r_ShadowCamDist", 100.f);
	REGISTER_CVAR(r_ShadowCamDist, r_ShadowCamDist, CVAR_CATEGORY_CLIENT,
		"Shadow camera far");

	REGISTER_CVAR(MoveEditParticle, 0, CVAR_CATEGORY_CLIENT, "MoveEditParticle");

	r_UseShaderCache = gFBEnv->pScriptSystem->GetIntVariable("r_UseShaderCache", 1);
	REGISTER_CVAR(r_UseShaderCache, r_UseShaderCache, CVAR_CATEGORY_CLIENT, "Use shader cache");

	r_GenerateShaderCache = gFBEnv->pScriptSystem->GetIntVariable("r_GenerateShaderCache", 1);
	REGISTER_CVAR(r_GenerateShaderCache, r_GenerateShaderCache, CVAR_CATEGORY_CLIENT, "generate shader cache");

	r_numRenderTargets = gFBEnv->pScriptSystem->GetIntVariable("r_numRenderTargets", 0);
	REGISTER_CVAR(r_numRenderTargets, r_numRenderTargets, CVAR_CATEGORY_CLIENT, "Log render targets");
	
	r_numPointLights = gFBEnv->pScriptSystem->GetIntVariable("r_numPointLights", 0);
	REGISTER_CVAR(r_numPointLights, r_numPointLights, CVAR_CATEGORY_CLIENT, "Log number of points lights");

	r_numParticleEmitters = gFBEnv->pScriptSystem->GetIntVariable("r_numParticleEmitters", 0);
	REGISTER_CVAR(r_numParticleEmitters, r_numParticleEmitters, CVAR_CATEGORY_CLIENT, "Log number of particle emitters");

	r_debugDraw = gFBEnv->pScriptSystem->GetIntVariable("r_debugDraw", 1);
	REGISTER_CVAR(r_debugDraw, r_debugDraw, CVAR_CATEGORY_CLIENT, "Debug draw");

	REGISTER_CC(&ccSpawnParticle);
	REGISTER_CC(&ccRun);
	REGISTER_CC(&ccDebugRenderTarget);
	REGISTER_CC(&ccSetFov);
	REGISTER_CC(&ccClear);
	REGISTER_CC(&ccScaleEditingParticle);
}
EngineCommand::~EngineCommand()
{
	for (auto p : mCVars)
	{
		FB_SAFE_DEL(p);
	}

	for (auto p : mCommands)
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

void ScaleEditingParticle(StringVector& arg)
{
	if (arg.size() < 2)
		return;
	ParticleManager::GetParticleManager().ScaleEditingParticle(StringConverter::parseReal(arg[1]));
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

void DebugRenderTarget(StringVector& arg)
{
	if (arg.size() < 2)
		return;

	gFBEnv->pRenderer->SetDebugRenderTarget(StringConverter::parseUnsignedInt(arg[1]),
		arg.size() == 3 ? arg[2].c_str() : "");
}

void SetFov(StringVector& arg)
{
	if (arg.size() < 2)
		return;

	gFBEnv->pRenderer->GetMainCamera()->SetFOV( Radian(StringConverter::parseReal(arg[1]) ) );
}

void Clear(StringVector& arg)
{
	gFBEnv->pConsole->Clear();
}




}

