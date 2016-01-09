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
#include "RenderStrategyNull.h"
#include "RenderTarget.h"
#include "Renderer.h"
using namespace fb;
class RenderStrategyNull::Impl{
public:
	ISceneWeakPtr mScene;
	RenderTargetWeakPtr mRenderTarget;
	Vec2I mSize; // Render target size.
	RenderTargetId mId;
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
		auto renderTarget = mRenderTarget.lock();
		auto& renderer = Renderer::GetInstance();
		if (!renderTarget)
			return;
		renderTarget->Bind(face);
		renderer.Clear(0., 0., 0., 1., 1.f, 0);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(RenderStrategyNull);
RenderStrategyNull::RenderStrategyNull()
	: mImpl(new Impl){

}
RenderStrategyNull::~RenderStrategyNull(){
}

void RenderStrategyNull::SetMain(bool main){

}

void RenderStrategyNull::SetScene(IScenePtr scene){
	mImpl->SetScene(scene);
}

void RenderStrategyNull::SetRenderTarget(RenderTargetPtr renderTarget){
	mImpl->SetRenderTarget(renderTarget);
}

void RenderStrategyNull::UpdateLightCamera(){
	
}

void RenderStrategyNull::Render(size_t face){
	mImpl->Render(face);
}

bool RenderStrategyNull::IsHDR() const{
	return false;
}

bool RenderStrategyNull::IsGlowSupported(){
	return false;
}

CameraPtr RenderStrategyNull::GetLightCamera() const{

	return 0;
}

bool RenderStrategyNull::SetHDRTarget(){
	return false;
}

bool RenderStrategyNull::SetSmallSilouetteBuffer(){
	return false;
}

bool RenderStrategyNull::SetBigSilouetteBuffer(){
	return false;
}

void RenderStrategyNull::GlowRenderTarget(bool bind){
}

void RenderStrategyNull::DepthTexture(bool bind){
	
}

void RenderStrategyNull::OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName){
	
}

void RenderStrategyNull::OnRenderTargetSizeChanged(const Vec2I& size){

}

TexturePtr RenderStrategyNull::GetShadowMap(){
	return 0;
}