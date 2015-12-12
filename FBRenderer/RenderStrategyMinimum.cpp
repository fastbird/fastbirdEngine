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
#include "RenderStrategyMinimum.h"
#include "RenderTarget.h"
#include "Renderer.h"
#include "RendererOptions.h"
#include "Texture.h"
#include "ResourceProvider.h"
#include "Camera.h"
#include "FBMathLib/BoundingVolume.h"
#include "FBTimer/Timer.h"
#include "FBSceneManager/IScene.h"
#include "FBSceneManager/ISpatialObject.h"
#include "FBSceneManager/DirectionalLight.h"
using namespace fb;
FB_IMPLEMENT_STATIC_CREATE(RenderStrategyMinimum);
class RenderStrategyMinimum::Impl{
public:
	ISceneWeakPtr mScene;
	RenderTargetWeakPtr mRenderTarget;
	Vec2I mSize; // Render target size.
	RenderTargetId mId;
	TexturePtr mDepthTarget;
	CameraPtr mLightCamera;
	TexturePtr mShadowMap;
	
	
	
	size_t mRenderingFace;

	//---------------------------------------------------------------------------
	Impl()
		: mRenderingFace(0)
	{

	}

	//-------------------------------------------------------------------
	// IRenderStrategy
	//-------------------------------------------------------------------
	void SetScene(IScenePtr scene){
		mScene = scene;
	}

	void SetRenderTarget(RenderTargetPtr renderTarget){
		mRenderTarget = renderTarget;
		mSize = renderTarget->GetSize();
		mId = renderTarget->GetId();
	}

	void UpdateLightCamera()
	{
		auto& renderer = Renderer::GetInstance();
		if (!mLightCamera)
		{
			mLightCamera = Camera::Create();
			mLightCamera->SetOrthogonal(true);
			auto cmd = renderer.GetRendererOptions();
			float width = std::min(cmd->r_ShadowCamWidth, mSize.x * (cmd->r_ShadowCamWidth / 1600.f));
			float height = std::min(cmd->r_ShadowCamHeight, mSize.y * (cmd->r_ShadowCamHeight / 900.f));
			width = std::max(16.f, width);
			height = std::max(16.f, height);
			Vec2 lightCamSize(width, height);

			Vec2 shadowMapSize(std::min((Real)cmd->r_ShadowMapWidth, (Real)(mSize.x / 1600.0f * cmd->r_ShadowMapWidth)),
				std::min((Real)cmd->r_ShadowMapHeight, (Real)(mSize.y / 900.0f * cmd->r_ShadowMapHeight))
				);
			shadowMapSize.x = (Real)CropSize8((int)shadowMapSize.x);
			shadowMapSize.y = (Real)CropSize8((int)shadowMapSize.y);
			shadowMapSize.x = std::max(16.0f, shadowMapSize.x);
			shadowMapSize.y = std::max(16.0f, shadowMapSize.y);

			Vec2 vWorldUnitsPerTexel;
			vWorldUnitsPerTexel = Vec2(lightCamSize.x, lightCamSize.y);
			vWorldUnitsPerTexel *= Vec2(1.0f / shadowMapSize.x, 1.0f / shadowMapSize.y);
			lightCamSize.x = std::floor(lightCamSize.x / vWorldUnitsPerTexel.x);
			lightCamSize.x *= vWorldUnitsPerTexel.x;
			lightCamSize.y = std::floor(lightCamSize.y / vWorldUnitsPerTexel.y);
			lightCamSize.y *= vWorldUnitsPerTexel.y;

			mLightCamera->SetWidth(lightCamSize.x);
			mLightCamera->SetHeight(lightCamSize.y);
			mLightCamera->SetNearFar(cmd->r_ShadowNear, cmd->r_ShadowFar);
		}

		auto cam = renderer.GetCamera();
		auto target = cam->GetTarget();
		float shadowCamDist = renderer.GetRendererOptions()->r_ShadowCamDist;
		auto scene = mScene.lock();
		if (scene){
			const auto& lightDir = scene->GetMainLightDirection();			
			if (target && target->GetBoundingVolumeWorld() && target->GetBoundingVolumeWorld()->GetRadius() < shadowCamDist)
			{
				mLightCamera->SetPosition(target->GetPosition() + cam->GetDirection() * 10.0f + lightDir *shadowCamDist);
			}
			else
			{
				mLightCamera->SetPosition(cam->GetPosition() + cam->GetDirection() * 10.0f + lightDir * shadowCamDist);
			}
			mLightCamera->SetDirection(-lightDir);
		}
	}

	void Render(size_t face){
		auto scene = mScene.lock();
		auto renderTarget = mRenderTarget.lock();
		if (!renderTarget)
			return;
		if (!scene){
			scene = renderTarget->GetScene();
			mScene = scene;
			if (!scene){
				Logger::Log(FB_FRAME_TIME, FB_ERROR_LOG_ARG, "RenderStrategy cannot find a scene.");
				return;
			}
		}		
		auto& renderer = Renderer::GetInstance();

		RenderEventMarker marker("Rendering with minimum strategy.");
		RenderParam renderParam;
		RenderParamOut renderParamOut;
		memset(&renderParam, 0, sizeof(RenderParam));
		memset(&renderParamOut, 0, sizeof(RenderParamOut));		
		DepthTexture(false);		
		ShadowMap(false);
		renderTarget->Bind(face);
		RenderParam param;
		memset(&param, 0, sizeof(RenderParam));
		param.mCamera = renderer.GetCamera().get();
		param.mLightCamera = mLightCamera.get();
		scene->PreRender(param, 0);


		{
			RenderEventMarker marker("Shadow Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_SHADOW;
			ShadowMap(false);
			ShadowTarget(true);
			renderParam.mCamera = renderer.GetCamera().get();
			renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, 0);
			ShadowTarget(false);
		}
		{
			RenderEventMarker marker("Depth Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_DEPTH;
			DepthTarget(true, true);
			renderParam.mCamera = renderer.GetCamera().get();
			renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, 0);
			DepthTarget(false, false);
			DepthTexture(true);
		}
		{
			RenderEventMarker marker("Main Render Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_NORMAL;
			renderTarget->BindTargetOnly(false);
			renderer.Clear(0., 0., 0., 1.);
			DepthTexture(true);			
			ShadowMap(true);
			renderParam.mCamera = renderer.GetCamera().get();
			renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, &renderParamOut);			
		}

		renderTarget->Unbind();
	}

	void DepthTarget(bool bind, bool clear)
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		if (bind){
			int width = int(mSize.x);
			int height = int(mSize.y);
			if (!mDepthTarget)
			{
				mDepthTarget = renderer.CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
					BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			}
			TexturePtr rts[] = { mDepthTarget };
			size_t rtViewIndex[] = { 0 };

			auto depthBuffer = renderer.GetTemporalDepthBuffer(Vec2I(width, height), "DepthTarget");
			renderer.SetRenderTarget(rts, rtViewIndex, 1, depthBuffer, 0);
			Viewport vp = { 0, 0, (float)width, (float)height, 0, 1 };
			renderer.SetViewports(&vp, 1);
			if (clear)
				renderer.Clear(1, 1, 1, 1, 1, 0);
			renderer.SetLockBlendState(true);
		}
		else{
			rt->BindTargetOnly(false);
			renderer.SetLockBlendState(false);
		}
	}

	void DepthTexture(bool bind) {
		auto& renderer = Renderer::GetInstance();
		if (bind){
			renderer.SetSystemTexture(SystemTextures::Depth, mDepthTarget);
		}
		else{
			renderer.SetSystemTexture(SystemTextures::Depth, 0);
		}
	}

	void OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName){
		if (strcmp(optionName, "r_shadowmapwidth") == 0 ||
			strcmp(optionName, "r_shadowmapheight") == 0)
		{
			mShadowMap.reset();
		}
		else if ((strcmp(optionName, "r_shadowcamwidth") == 0 ||
			strcmp(optionName, "r_shadowcamheight") == 0))
		{
			if (mLightCamera){
				mLightCamera->SetWidth(options->r_ShadowCamWidth);
				mLightCamera->SetHeight(options->r_ShadowCamHeight);
			}
		}
		else if ((strcmp(optionName, "r_shadownear") == 0 ||
			strcmp(optionName, "r_shadowfar") == 0))
		{
			if (mLightCamera){
				mLightCamera->SetNearFar(options->r_ShadowNear, options->r_ShadowFar);
			}
		}
	}

	void OnRenderTargetSizeChanged(const Vec2I& size){
		mSize = size;
		mDepthTarget.reset();
		mLightCamera.reset();
		mShadowMap.reset();
	}

	void ShadowTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		if (bind){

			auto cmd = renderer.GetRendererOptions();
			if (!mShadowMap)
			{
				const auto& size = mSize;
				int width = (int)std::min(cmd->r_ShadowMapWidth, (int)(size.x / 1600 * cmd->r_ShadowMapWidth));
				int height = (int)std::min(cmd->r_ShadowMapHeight, (int)(size.y / 900 * cmd->r_ShadowMapHeight));
				width = CropSize8(width);
				height = CropSize8(height);

				width = std::max(16, width);
				height = std::max(16, height);

				mShadowMap = renderer.CreateTexture(0,
					width, height,
					PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
					BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL_SRV);
				assert(mShadowMap);
				mShadowMap->SetDebugName(FormatString("rt%u_%u_%u_ShadowMap", rt->GetId(), width, height).c_str());
			}

			TexturePtr rts[] = { 0 };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, mShadowMap, 0);
			auto provider = renderer.GetResourceProvider();
			provider->BindShader(ResourceTypes::Shaders::ShadowMapVSPS);
			const auto& size = mShadowMap->GetSize();
			Viewport vp = { 0, 0,
				(float)size.x,
				(float)size.y,
				0, 1 };
			renderer.SetViewports(&vp, 1);
			renderer.SetCamera(mLightCamera);
			renderer.Clear(0, 0, 0, 0, 1.0f, 0);
		}
		else{
			assert(mShadowMap);
			rt->BindTargetOnly(false);
			renderer.SetCamera(rt->GetCamera());
		}
	}

	void ShadowMap(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		if (bind && mShadowMap)
			renderer.SetSystemTexture(SystemTextures::ShadowMap, mShadowMap);
		else
			renderer.SetSystemTexture(SystemTextures::ShadowMap, 0);
	}

	void DepthWriteShaderCloud(){

	}

	TexturePtr GetShadowMap() const{
		return mShadowMap;
	}

	void DeleteShadowMap()
	{
		mShadowMap = 0;
	}
};

//---------------------------------------------------------------------------
RenderStrategyMinimum::RenderStrategyMinimum()
	: mImpl(new Impl){

}
RenderStrategyMinimum::~RenderStrategyMinimum(){
}

void RenderStrategyMinimum::SetScene(IScenePtr scene){
	mImpl->SetScene(scene);
}

void RenderStrategyMinimum::SetRenderTarget(RenderTargetPtr renderTarget){
	mImpl->SetRenderTarget(renderTarget);
}

void RenderStrategyMinimum::UpdateLightCamera(){
	mImpl->UpdateLightCamera();
}

void RenderStrategyMinimum::Render(size_t face){
	mImpl->Render(face);
}

bool RenderStrategyMinimum::IsHDR() const{
	return false;
}

bool RenderStrategyMinimum::IsGlowSupported(){
	return false;
}

CameraPtr RenderStrategyMinimum::GetLightCamera() const{

	return mImpl->mLightCamera;
}

bool RenderStrategyMinimum::SetHDRTarget(){
	return false;
}

bool RenderStrategyMinimum::SetSmallSilouetteBuffer(){
	return false;
}

bool RenderStrategyMinimum::SetBigSilouetteBuffer(){
	return false;
}

void RenderStrategyMinimum::GlowRenderTarget(bool bind){
}

void RenderStrategyMinimum::DepthTexture(bool bind){
	mImpl->DepthTexture(bind);
}

void RenderStrategyMinimum::OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName){
	mImpl->OnRendererOptionChanged(options, optionName);
}

void RenderStrategyMinimum::OnRenderTargetSizeChanged(const Vec2I& size){
	mImpl->OnRenderTargetSizeChanged(size);
}

TexturePtr RenderStrategyMinimum::GetShadowMap(){
	return mImpl->mShadowMap;
}