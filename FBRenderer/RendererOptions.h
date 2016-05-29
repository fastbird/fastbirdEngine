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

#pragma once
#include "FBMathLib/Vec2I.h"
#include "FBCommonHeaders/Types.h"
#include "FBConsole/ICVarObserver.h"
namespace fb{
	FB_DECLARE_SMART_PTR_STRUCT(CVar);
	FB_DECLARE_SMART_PTR(RendererOptions);
	class RendererOptions : public ICVarObserver{
		RendererOptions();
		~RendererOptions();		

	public:

		// ICVarObserver
		bool OnChangeCVar(CVarPtr pCVar);
		static RendererOptionsPtr Create();

		int r_noObjectConstants;
		int r_noMesh;
		int r_noSky;
		int r_noParticleDraw;
		int r_particleProfile;
		int r_HDR;
		float r_HDRMiddleGray;
		int r_HDRCpuLuminance;
		int r_HDRFilmic;
		float r_BloomPower;
		float r_StarPower;
		int r_GodRay;
		float r_GodRayWeight;
		float r_GodRayDecay;
		float r_GodRayDensity;
		float r_GodRayExposure;
		int r_Glow;
		int r_ReportDeviceObjectLeak;
		int r_Shadow;
		int r_ShadowCascadeLevels;
		float r_ShadowCascadeBlendArea;
		int r_ShadowMapPCFBlurSize;
		int r_ShadowMapSize;
		int r_LightFrustum;
		float r_ShadowCamWidth;
		float r_ShadowCamHeight;
		float r_ShadowNear;
		float r_ShadowFar;
		float r_ShadowCamDist;
		int r_UseShaderCache;
		int r_GenerateShaderCache;
		int r_numRenderTargets;
		int r_numParticleEmitters;
		int r_debugDraw;
		int r_debugCam;
		int r_gameId;
		Vec2I r_resolution;
		int r_fullscreen; // 0 : window, 1 : full-screen, 2 : fake full-screen
		int r_noText;
		int r_renderFrustum;
		int r_renderAxis;
	};
}