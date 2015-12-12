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
#include "RendererOptions.h"
#include "Renderer.h"
#include "FBConsole/Console.h"
#include "ResourceProvider.h"
#include "RenderTarget.h"
using namespace fb;

RendererOptionsPtr RendererOptions::Create(){
	return RendererOptionsPtr(new RendererOptions, [](RendererOptions* obj){ delete obj; });
}

static void ReloadFonts(StringVector& arg);

RendererOptions::RendererOptions(){	
	FB_REGISTER_CC(ReloadFonts, "Reload fonts");

	r_noObjectConstants =  Console::GetInstance().GetIntVariable("r_noObjectConstants", 0);
	FB_REGISTER_CVAR(r_noObjectConstants, r_noObjectConstants, CVAR_CATEGORY_CLIENT, "do not update object constans buffer");

	r_noMesh = Console::GetInstance().GetIntVariable("r_noMesh", 0);
	FB_REGISTER_CVAR(r_noMesh, r_noMesh, CVAR_CATEGORY_CLIENT, "do not render meshes");

	r_noSky = Console::GetInstance().GetIntVariable("r_noSky", 0);
	FB_REGISTER_CVAR(r_noSky, r_noSky, CVAR_CATEGORY_CLIENT, "do not render sky");

	r_noParticleDraw = Console::GetInstance().GetIntVariable("r_noParticleDraw", 0);
	FB_REGISTER_CVAR(r_noParticleDraw, r_noParticleDraw, CVAR_CATEGORY_CLIENT, "No particle Draw");

	r_particleProfile = Console::GetInstance().GetIntVariable("r_particleProfile", 0);
	FB_REGISTER_CVAR(r_particleProfile, r_particleProfile, CVAR_CATEGORY_CLIENT, "enable profiler");

	r_HDR = Console::GetInstance().GetIntVariable("r_HDR", 1);
	FB_REGISTER_CVAR(r_HDR, r_HDR, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_HDRMiddleGray = Console::GetInstance().GetRealVariable("r_HDRMiddleGray", 0.25);
	FB_REGISTER_CVAR(r_HDRMiddleGray, r_HDRMiddleGray, CVAR_CATEGORY_CLIENT, "Decide scene key.");

	r_HDRCpuLuminance = Console::GetInstance().GetIntVariable("r_HDRCpuLuminance", 0);
	FB_REGISTER_CVAR(r_HDRCpuLuminance, r_HDRCpuLuminance, CVAR_CATEGORY_CLIENT, "If true, luminance will be calculated in CPU. Debug purpose");

	r_HDRFilmic = Console::GetInstance().GetIntVariable("r_HDRFilmic", 1);
	FB_REGISTER_CVAR(r_HDRFilmic, r_HDRFilmic, CVAR_CATEGORY_CLIENT, "Use Filmic tone mapping");

	r_BloomPower = Console::GetInstance().GetRealVariable("r_BloomPower", 0.2f);
	FB_REGISTER_CVAR(r_BloomPower, r_BloomPower, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_StarPower = Console::GetInstance().GetRealVariable("r_StarPower", 0.4f);
	FB_REGISTER_CVAR(r_StarPower, r_StarPower, CVAR_CATEGORY_CLIENT, "enable hdr rendering");

	r_GodRay = Console::GetInstance().GetIntVariable("r_GodRay", 1);
	FB_REGISTER_CVAR(r_GodRay, r_GodRay, CVAR_CATEGORY_CLIENT, "enable GodRay rendering");

	r_GodRayWeight = Console::GetInstance().GetRealVariable("r_GodRayWeight", 0.1f);
	FB_REGISTER_CVAR(r_GodRayWeight, r_GodRayWeight, CVAR_CATEGORY_CLIENT, "enable GodRay rendering");

	r_GodRayDecay = Console::GetInstance().GetRealVariable("r_GodRayDecay", 0.9f);
	FB_REGISTER_CVAR(r_GodRayDecay, r_GodRayDecay, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_GodRayDensity = Console::GetInstance().GetRealVariable("r_GodRayDensity", 0.2f);
	FB_REGISTER_CVAR(r_GodRayDensity, r_GodRayDensity, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_GodRayExposure = Console::GetInstance().GetRealVariable("r_GodRayExposure", 0.8f);
	FB_REGISTER_CVAR(r_GodRayExposure, r_GodRayExposure, CVAR_CATEGORY_CLIENT, "GodRay property");

	r_ReportDeviceObjectLeak = Console::GetInstance().GetIntVariable("r_ReportDeviceObjectLeak", 0);
	FB_REGISTER_CVAR(r_ReportDeviceObjectLeak, r_ReportDeviceObjectLeak, CVAR_CATEGORY_CLIENT, "ReportResourceLeak");

	r_Glow = Console::GetInstance().GetIntVariable("r_Glow", 1);
	FB_REGISTER_CVAR(r_Glow, r_Glow, CVAR_CATEGORY_CLIENT, "Glow");

	r_Shadow = Console::GetInstance().GetIntVariable("r_Shadow", 1);
	FB_REGISTER_CVAR(r_Shadow, r_Shadow, CVAR_CATEGORY_CLIENT, "enable shadow");

	r_ShadowMapWidth = Console::GetInstance().GetIntVariable(
		"r_ShadowMapWidth", 4096);
	FB_REGISTER_CVAR(r_ShadowMapWidth, r_ShadowMapWidth, CVAR_CATEGORY_CLIENT,
		"ShadowMap width");

	r_ShadowMapHeight = Console::GetInstance().GetIntVariable(
		"r_ShadowMapHeight", 4096);
	FB_REGISTER_CVAR(r_ShadowMapHeight, r_ShadowMapHeight, CVAR_CATEGORY_CLIENT,
		"ShadowMap height");

	r_ShadowCamWidth = Console::GetInstance().GetRealVariable(
		"r_ShadowCamWidth", 150.f);
	FB_REGISTER_CVAR(r_ShadowCamWidth, r_ShadowCamWidth, CVAR_CATEGORY_CLIENT,
		"ShadowMap width");

	r_ShadowCamHeight = Console::GetInstance().GetRealVariable(
		"r_ShadowCamHeight", 150.f);
	FB_REGISTER_CVAR(r_ShadowCamHeight, r_ShadowCamHeight, CVAR_CATEGORY_CLIENT,
		"ShadowMap height");

	r_ShadowNear = Console::GetInstance().GetRealVariable(
		"r_ShadowNear", 0.0f);
	FB_REGISTER_CVAR(r_ShadowNear, r_ShadowNear, CVAR_CATEGORY_CLIENT,
		"Shadow camera near.");

	r_ShadowFar = Console::GetInstance().GetRealVariable(
		"r_ShadowFar", 200.0f);
	FB_REGISTER_CVAR(r_ShadowFar, r_ShadowFar, CVAR_CATEGORY_CLIENT,
		"Shadow camera far");

	r_ShadowCamDist = Console::GetInstance().GetRealVariable(
		"r_ShadowCamDist", 100.f);
	FB_REGISTER_CVAR(r_ShadowCamDist, r_ShadowCamDist, CVAR_CATEGORY_CLIENT,
		"Shadow camera far");

	r_UseShaderCache = Console::GetInstance().GetIntVariable("r_UseShaderCache", 1);
	FB_REGISTER_CVAR(r_UseShaderCache, r_UseShaderCache, CVAR_CATEGORY_CLIENT, "Use shader cache");

	r_GenerateShaderCache = Console::GetInstance().GetIntVariable("r_GenerateShaderCache", 1);
	FB_REGISTER_CVAR(r_GenerateShaderCache, r_GenerateShaderCache, CVAR_CATEGORY_CLIENT, "generate shader cache");

	r_numRenderTargets = Console::GetInstance().GetIntVariable("r_numRenderTargets", 0);
	FB_REGISTER_CVAR(r_numRenderTargets, r_numRenderTargets, CVAR_CATEGORY_CLIENT, "Log render targets");


	r_numParticleEmitters = Console::GetInstance().GetIntVariable("r_numParticleEmitters", 0);
	FB_REGISTER_CVAR(r_numParticleEmitters, r_numParticleEmitters, CVAR_CATEGORY_CLIENT, "Log number of particle emitters");

	r_debugDraw = Console::GetInstance().GetIntVariable("r_debugDraw", 1);
	FB_REGISTER_CVAR(r_debugDraw, r_debugDraw, CVAR_CATEGORY_CLIENT, "Debug draw");

	r_gameId = Console::GetInstance().GetIntVariable("r_gameId", 0);
	FB_REGISTER_CVAR(r_gameId, r_gameId, CVAR_CATEGORY_CLIENT, "Draw game id");

	r_resolution = Console::GetInstance().GetVec2IVariable("r_resolution", Vec2I(1600, 900));
	FB_REGISTER_CVAR(r_resolution, r_resolution, CVAR_CATEGORY_CLIENT, "Resolution");

	r_fullscreen = Console::GetInstance().GetIntVariable("r_fullscreen", 0);
	FB_REGISTER_CVAR(r_fullscreen, r_fullscreen, CVAR_CATEGORY_CLIENT, "fullscreen");

	r_noText = Console::GetInstance().GetIntVariable("r_noText", 0);
	FB_REGISTER_CVAR(r_noText, r_noText, CVAR_CATEGORY_CLIENT, "r_noText");
}

RendererOptions::~RendererOptions(){

}

bool RendererOptions::OnChangeCVar(CVarPtr pCVar){
	auto selfPtr = Renderer::GetInstance().GetRendererOptions();
	// name is always lower case
	if (strcmp(pCVar->mName.c_str(), "r_hdr") == 0)
	{
		auto rp = Renderer::GetInstance().GetResourceProvider();
		rp->DeleteTexture(ResourceTypes::Textures::ToneMap);
		rp->DeleteTexture(ResourceTypes::Textures::LuminanceMap);
		rp->DeleteShader(ResourceTypes::Shaders::ToneMappingPS);
		rp->DeleteShader(ResourceTypes::Shaders::BrightPassPS);
		rp->DeleteShader(ResourceTypes::Shaders::BloomPS);		
		return true;
	}
	else if (strcmp(pCVar->mName.c_str(), "r_hdrcpuluminance") == 0)
	{
		Logger::Log(FB_DEFAULT_LOG_ARG, "Not implemented");
	}
	else if (strcmp(pCVar->mName.c_str(), "r_hdrfilmic") == 0)
	{
		Renderer::GetInstance().SetFilmicToneMapping(pCVar->GetInt() != 0);
	}
	else if (strcmp(pCVar->mName.c_str(), "r_hdrmiddlegray") == 0)
	{
		Renderer::GetInstance().UpdateRareConstantsBuffer();
	}
	else if (strcmp(pCVar->mName.c_str(), "r_bloompower") == 0)
	{
		Renderer::GetInstance().UpdateRareConstantsBuffer();
	}
	else if (strcmp(pCVar->mName.c_str(), "r_starpower") == 0)
	{
		Renderer::GetInstance().UpdateRareConstantsBuffer();
	}
	else if (strcmp(pCVar->mName.c_str(), "r_shadowmapwidth") == 0 ||
		strcmp(pCVar->mName.c_str(), "r_shadowmapheight") == 0 ||
		strcmp(pCVar->mName.c_str(), "r_shadowcamwidth") == 0 ||
		strcmp(pCVar->mName.c_str(), "r_shadowcamheight") == 0 ||
		strcmp(pCVar->mName.c_str(), "r_shadownear") == 0 ||
		strcmp(pCVar->mName.c_str(), "r_shadowfar") == 0)
	{
		auto rt = Renderer::GetInstance().GetMainRenderTarget();
		if (rt){
			rt->OnRendererOptionChanged(selfPtr, pCVar->mName.c_str());
		}		
	}	
	else if (strcmp(pCVar->mName.c_str(), "r_resolution") == 0){
		auto resol = pCVar->GetVec2I();
		if (r_fullscreen == 1){
			Renderer::GetInstance().ChangeResolution(resol);
		}
		else if (r_fullscreen == 0){
			Renderer::GetInstance().ChangeWindowSizeAndResolution(resol);
		}
		else{
			 // cannot change resolution while in faked fullscreen mode.
		}
	}
	else if (strcmp(pCVar->mName.c_str(), "r_fullscreen") == 0){
		auto fullscreen = pCVar->GetInt();
		Renderer::GetInstance().ChangeFullscreenMode(fullscreen);
	}

	return false;
}

static void ReloadFonts(StringVector& arg){
	Renderer::GetInstance().ReloadFonts();
}