#include "stdafx.h"
#include "RenderStrategyNull.h"
#include "RenderTarget.h"
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

TexturePtr RenderStrategyNull::GetShadowMap(){
	return 0;
}