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