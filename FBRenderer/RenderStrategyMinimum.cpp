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
#include "CascadedShadowsManager.h"
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
		renderer.SetBindShadowMap(false);
		renderTarget->Bind(face);
		RenderParam param;
		memset(&param, 0, sizeof(RenderParam));
		param.mCamera = renderer.GetCamera().get();		
		scene->MakeVisibleSet(param.mCamera);
		scene->PreRender(param, 0);

		{
			renderer.RenderShadows();			
		}
		{
			RenderEventMarker marker("Depth Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_DEPTH;
			DepthTarget(true, true);
			renderParam.mCamera = renderer.GetCamera().get();			
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
			renderer.SetBindShadowMap(true);
			renderParam.mCamera = renderer.GetCamera().get();			
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
				mDepthTarget = renderer.CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, 1,
					BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
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

	}

	void OnRenderTargetSizeChanged(const Vec2I& size){
		mSize = size;
		mDepthTarget.reset();
	}

	void ShadowTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		if (bind){
			// empty
		}
		else{
			rt->BindTargetOnly(false);
			renderer.SetCamera(rt->GetCamera());
		}
	}

	void DepthWriteShaderCloud(){

	}
};

//---------------------------------------------------------------------------
RenderStrategyMinimum::RenderStrategyMinimum()
	: mImpl(new Impl){

}
RenderStrategyMinimum::~RenderStrategyMinimum(){
}

void RenderStrategyMinimum::SetMain(bool main){

}

void RenderStrategyMinimum::SetScene(IScenePtr scene){
	mImpl->SetScene(scene);
}

void RenderStrategyMinimum::SetRenderTarget(RenderTargetPtr renderTarget){
	mImpl->SetRenderTarget(renderTarget);
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