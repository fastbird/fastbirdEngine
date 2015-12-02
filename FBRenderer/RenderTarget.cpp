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
#include "RenderTarget.h"
#include "Renderer.h"
#include "Texture.h"
#include "RenderStrategyDefault.h"
#include "RenderEventMarker.h"
#include "RendererOptions.h"
#include "RenderTargetParam.h"
#include "Camera.h"
#include "FBSceneManager/IScene.h"
#include "FBInputManager/IInputInjector.h"
#include "FBCommonHeaders/Observable.h"

namespace fb
{
RenderTargetId NextRenderTargetId = 1;

class RenderTarget::Impl{
public:
	RenderTargetWeakPtr mSelfPtr;
	CameraPtr mCamera;
	IRenderStrategyPtr mStrategy;
	RendererWeakPtr mRenderer;
	ISceneWeakPtr mScene;
	/// For RTT(render to texture)
	IScenePtr mSceneOwnerShip;
	RenderTargetId mId;

	bool mEnabled;
	bool mUsePool;
	Vec2I mSize;
	Vec2I mSizeCropped;
	Viewport mViewport;
	PIXEL_FORMAT mFormat;
	bool mSRV;
	bool mMiplevel;
	bool mCubeMap;
	bool mWillCreateDepth;
	TexturePtr mRenderTargetTexture;
	TexturePtr mDepthStencilTexture;
	Color mClearColor;
	Real mDepthClear;
	UINT8 mStencilClear;
	unsigned mFace;
	TexturePtr mEnvTexture;
	bool mDrawOnEvent;
	bool mDrawEventTriggered;

	Impl()
		: mClearColor(0, 0, 0, 1)
		, mDepthClear(1.f)
		, mStencilClear(0)
		, mEnabled(true)
		, mSize(0, 0), mSizeCropped(0, 0)
		, mFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
		, mSRV(false)
		, mMiplevel(false)
		, mCubeMap(false)
		, mWillCreateDepth(false), mUsePool(true)
		, mDrawOnEvent(false), mDrawEventTriggered(false)
		, mFace(0)
		, mId(NextRenderTargetId++)
		, mCamera(Camera::Create())
	{
		mStrategy = RenderStrategyDefault::Create();
	}

	void Initialize(){
		mStrategy->SetRenderTarget(mSelfPtr.lock());
	}

	//-------------------------------------------------------------------
	// Observable<IRenderTargetObserver>
	//-------------------------------------------------------------------
	void OnObserverAdded(IRenderTargetObserverPtr observer){
		auto& renderer = Renderer::GetInstance();
		HWindow hwnd = renderer.GetWindowHandle(mId);
		if (hwnd){
			observer->OnRenderTargetSizeChanged(mSize.x, mSize.y, hwnd);
		}
	}
	void OnObserverRemoved(IRenderTargetObserver* observer){
	}

	//-------------------------------------------------------------------

	bool CheckOptions(const RenderTargetParam& param)
	{
		return param.mSize == mSize && param.mPixelFormat == mFormat &&
			param.mShaderResourceView == mSRV &&
			param.mMipmap == mMiplevel && param.mCubemap == mCubeMap && param.mWillCreateDepth == mWillCreateDepth;
	}

	IRenderStrategyPtr SetRenderStrategy(IRenderStrategyPtr strategy){
		auto prev = mStrategy;
		mStrategy = strategy;
		return prev;
	}

	IScenePtr RegisterScene(IScenePtr scene)
	{
		auto prevScene = mScene;
		mScene = scene;
		auto mainRt = Renderer::GetInstance().GetMainRenderTarget();
		if (scene && mainRt == mSelfPtr.lock()){
			scene->AddSceneObserver(ISceneObserver::Timing, Renderer::GetInstancePtr());
		}
		mStrategy->SetScene(scene);
		return prevScene.lock();
	}

	void TakeOwnershipScene(IScenePtr scene){
		mScene = scene;
		mSceneOwnerShip = scene;
	}

	void SetCamera(CameraPtr cam){
		mCamera = cam;
	}

	CameraPtr ReplaceCamera(CameraPtr cam){
		auto prev = mCamera;
		mCamera = cam;
		return prev;
	}

	void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap){
		mSize = { width, height };
		mSizeCropped = { CropSize8(width), CropSize8(height) };
		mFormat = format;
		mSRV = srv;
		mMiplevel = miplevel;
		mCubeMap = cubeMap;
		int type;
		type = srv ? TEXTURE_TYPE_RENDER_TARGET_SRV : TEXTURE_TYPE_RENDER_TARGET;
		if (miplevel)
			type |= TEXTURE_TYPE_MIPS;
		if (cubeMap)
			type |= TEXTURE_TYPE_CUBE_MAP;
		auto& renderer = Renderer::GetInstance();		
		mRenderTargetTexture = renderer.CreateTexture(0, width, height, format,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, type);
		if (mCamera){
			mCamera->SetWidth((float)width);
			mCamera->SetHeight((float)height);
		}
		mViewport.mTopLeftX = 0;
		mViewport.mTopLeftY = 0;
		mViewport.mWidth = (float)mSize.x;
		mViewport.mHeight = (float)mSize.y;
		mViewport.mMinDepth = 0.f;
		mViewport.mMaxDepth = 1.0f;

		if (mStrategy){
			mStrategy->SetRenderTarget(mSelfPtr.lock());
		}
	}

	void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap){
		int type;
		type = srv ? TEXTURE_TYPE_DEPTH_STENCIL_SRV : TEXTURE_TYPE_DEPTH_STENCIL;
		if (cubeMap)
			type |= TEXTURE_TYPE_CUBE_MAP;
		auto& renderer = Renderer::GetInstance();		
		mDepthStencilTexture = renderer.CreateTexture(0, width, height, format,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, type);
		mWillCreateDepth = true;
	}

	void Bind(size_t face)
	{
		if (!mEnabled)
			return;
		auto& renderer = Renderer::GetInstance();
		renderer.SetCurrentRenderTarget(mSelfPtr.lock());

		Renderer::GetInstance().UnbindRenderTarget(mRenderTargetTexture);
		
		TexturePtr rt[] = { mRenderTargetTexture };
		size_t rtViewIndex[] = { face };
		renderer.SetRenderTarget(rt, rtViewIndex, 1, mDepthStencilTexture, face);
		renderer.SetViewports(&mViewport, 1);
		renderer.Clear(mClearColor.r(), mClearColor.g(), mClearColor.b(), mClearColor.a(),
			mDepthClear, mStencilClear);
		auto scene = mScene.lock();
		if (scene){
			DirectionalLightInfo lightInfo;
			scene->GetDirectionalLightInfo(DirectionalLightIndex::Main, lightInfo);
			renderer.SetDirectionalLightInfo(0, lightInfo);
			scene->GetDirectionalLightInfo(DirectionalLightIndex::Sub, lightInfo);
			renderer.SetDirectionalLightInfo(1, lightInfo);			
		}

		renderer.SetCamera(mCamera);
		if (mCamera)
			mCamera->ProcessInputData();
		
		renderer.SetCurrentScene(scene);
		mStrategy->SetScene(scene);
		mStrategy->UpdateLightCamera();

		if (mEnvTexture)
			renderer.SetEnvironmentTextureOverride(mEnvTexture);
		renderer.RestoreRenderStates();
	}

	void BindTargetOnly(bool hdr)
	{
		auto& renderer = Renderer::GetInstance();
		renderer.SetCurrentRenderTarget(mSelfPtr.lock());
		if (hdr &&  mStrategy->IsHDR() && renderer.GetRendererOptions()->r_HDR){
			mStrategy->SetHDRTarget();
		}
		else{
			if (mRenderTargetTexture)
				mRenderTargetTexture->Unbind();
			TexturePtr rt[] = { mRenderTargetTexture };
			// we need to have 6 glow textures to support for cube map.
			// but don't need to.
			size_t rtViewIndex[] = { mFace };
			renderer.SetRenderTarget(rt, rtViewIndex, 1, mDepthStencilTexture, mFace);
			renderer.SetViewports(&mViewport, 1);
		}
	}

	void BindDepthTexture(bool bind){
		mStrategy->DepthTexture(bind);		
	}

	bool Render(size_t face)
	{
		if (!mEnabled || !mScene.lock())
			return false;

		if (mDrawOnEvent && !mDrawEventTriggered)
			return false;
		mDrawEventTriggered = false;

		mFace = face;
		auto& renderer = Renderer::GetInstance();
		RenderEventMarker mark("RenderTarget");
		mStrategy->Render(face);
		
		return true;
	}

	CameraPtr GetLightCamera() const {
		return mStrategy->GetLightCamera();
	}

	bool IsGlowSupported() const{
		return mStrategy->IsGlowSupported();
	}

	bool SetSmallSilouetteBuffer(){
		return mStrategy->SetSmallSilouetteBuffer();
	}

	bool SetBigSilouetteBuffer(){
		return mStrategy->SetBigSilouetteBuffer();
	}

	void OnRendererOptionChanged(RendererOptionsPtr options, const char* name){
		return mStrategy->OnRendererOptionChanged(options, name);
	}

	TexturePtr GetShadowMap() const{
		return mStrategy->GetShadowMap();
	}

	//---------------------------------------------------------------------------
	// Additional Targets
	//---------------------------------------------------------------------------
	void SetLightCamWidth(Real width)
	{
		auto lightCam = mStrategy->GetLightCamera();
		if (lightCam){
			lightCam->SetWidth(std::min(width, mSize.x * (width / 1600)));
		}
	}

	void SetLightCamHeight(Real height)
	{
		auto lightCam = mStrategy->GetLightCamera();
		if (lightCam){
			lightCam->SetHeight(std::min(height, mSize.y * (height / 900)));
		}
	}

	void SetLightCamNear(Real n)
	{
		auto lightCam = mStrategy->GetLightCamera();
		if (lightCam){
			Real on, of;
			lightCam->GetNearFar(on, of);
			lightCam->SetNearFar(n, of);
		}
	}
	void SetLightCamFar(Real f)
	{
		Real on, of;
		auto lightCam = mStrategy->GetLightCamera();
		if (lightCam){
			lightCam->GetNearFar(on, of);
			lightCam->SetNearFar(on, f);
		}
	}

	void DrawOnEvent(bool set)
	{
		mDrawOnEvent = set;
	}

	void TriggerDrawEvent()
	{
		mDrawEventTriggered = true;
	}

	void Unbind()
	{
		auto& renderer = Renderer::GetInstance();
		if (mEnvTexture)
		{
			renderer.SetEnvironmentTextureOverride(0);
		}
	}

	void GlowRenderTarget(bool bind){
		mStrategy->GlowRenderTarget(true);
	}

	void ConsumeInput(IInputInjectorPtr injector)
	{
		if (!injector->IsValid(InputDevice::Mouse))
			return;			
		return;
		mCamera->SetCurrent(true);
		mCamera->ConsumeInput(injector);
		mCamera->SetCurrent(false);
	}

	void SetColorTexture(TexturePtr pTexture)
	{
		mRenderTargetTexture = pTexture;
		mSize = pTexture->GetSize();
		auto& renderer = Renderer::GetInstance();
		mSizeCropped = Vec2I(CropSize8(mSize.x), CropSize8(mSize.y));
		mFormat = pTexture->GetFormat();
		mViewport.mTopLeftX = 0;
		mViewport.mTopLeftY = 0;
		mViewport.mWidth = (float)mSize.x;
		mViewport.mHeight = (float)mSize.y;
		mViewport.mMinDepth = 0.f;
		mViewport.mMaxDepth = 1.0f;
		mCamera->SetWidth((Real)mSize.x);
		mCamera->SetHeight((Real)mSize.y);
		/*if (mSize.x < 100 || mSize.y < 100)
		{
			mRenderPipeline->SetStep(RenderSteps::HDR, false);
		}*/

		if (mStrategy){
			mStrategy->SetRenderTarget(mSelfPtr.lock());
		}
	}

	void SetDepthTexture(TexturePtr texture){
		mDepthStencilTexture = texture;
	}
};

//-------------------------------------------------------------------
RenderTargetPtr RenderTarget::Create(){
	auto p = RenderTargetPtr(FB_NEW(RenderTarget), [](RenderTarget* obj){ FB_DELETE(obj); });
	p->mImpl->mSelfPtr = p;
	p->mImpl->Initialize();
	return p;
}

RenderTarget::RenderTarget()
	:mImpl(new Impl)
{
}

RenderTarget::~RenderTarget()
{
}

//-------------------------------------------------------------------
// Observable<IRenderTargetObserver>
//-------------------------------------------------------------------
void RenderTarget::OnObserverAdded(IRenderTargetObserverPtr observer){
	mImpl->OnObserverAdded(observer);
}

//-------------------------------------------------------------------
RenderTargetId RenderTarget::GetId() const{
	return mImpl->mId;
}

bool RenderTarget::CheckOptions(const RenderTargetParam& param)
{
	return mImpl->CheckOptions(param);
}

IRenderStrategyPtr RenderTarget::SetRenderStrategy(IRenderStrategyPtr strategy) {
	return mImpl->SetRenderStrategy(strategy);
}

IScenePtr RenderTarget::RegisterScene(IScenePtr scene)
{
	return mImpl->RegisterScene(scene);
}

void RenderTarget::TakeOwnershipScene(IScenePtr scene){
	return mImpl->TakeOwnershipScene(scene);
}

IScenePtr RenderTarget::GetScene() const
{
	return mImpl->mScene.lock();
}

void RenderTarget::SetCamera(CameraPtr cam){
	mImpl->SetCamera(cam);
}

CameraPtr RenderTarget::ReplaceCamera(CameraPtr cam){
	return mImpl->ReplaceCamera(cam);
}

CameraPtr RenderTarget::GetCamera() const
{
	return mImpl->mCamera;
}

TexturePtr RenderTarget::GetRenderTargetTexture() const{
	return mImpl->mRenderTargetTexture;
}

TexturePtr RenderTarget::GetDepthStencilTexture() const{
	return mImpl->mDepthStencilTexture;
}

const Vec2I& RenderTarget::GetSize() const
{
	return mImpl->mSize;
}

void RenderTarget::DeleteBuffers(){
	mImpl->mRenderTargetTexture = 0;
	mImpl->mDepthStencilTexture = 0;
}

void RenderTarget::SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap){
	mImpl->SetColorTextureDesc(width, height, format, srv, miplevel, cubeMap);
}
void RenderTarget::SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap){
	mImpl->SetDepthStencilDesc(width, height, format, srv, cubeMap);
}

void RenderTarget::SetClearValues(const Color& color, Real z, UINT8 stencil)
{
	mImpl->mClearColor = color;
	mImpl->mDepthClear = z;
	mImpl->mStencilClear = stencil;
}

void RenderTarget::SetClearColor(const Color& color)
{
	mImpl->mClearColor = color;
}

void RenderTarget::SetClearDepthStencil(Real z, UINT8 stencil)
{
	mImpl->mDepthClear = z;
	mImpl->mStencilClear = stencil;
}

void RenderTarget::Bind(size_t face)
{
	mImpl->Bind(face);	
}

void RenderTarget::BindTargetOnly(bool hdr)
{	
	mImpl->BindTargetOnly(hdr);
}

void RenderTarget::BindDepthTexture(bool bind){
	mImpl->BindDepthTexture(bind);
}

bool RenderTarget::Render(size_t face)
{
	return mImpl->Render(face);
}

void RenderTarget::Unbind()
{
	mImpl->Unbind();
}

void RenderTarget::GlowRenderTarget(bool bind){
	mImpl->GlowRenderTarget(bind);
}

void RenderTarget::SetEnable(bool enable) {
	mImpl->mEnabled = enable;
}
bool RenderTarget::GetEnable() const {
	return mImpl->mEnabled;
}

void RenderTarget::ConsumeInput(IInputInjectorPtr injector)
{
	mImpl->ConsumeInput(injector);	
}

void RenderTarget::SetEnvTexture(TexturePtr texture)
{
	mImpl->mEnvTexture = texture;
}

void RenderTarget::SetUsePool(bool usePool)
{
	mImpl->mUsePool = usePool;
}

bool RenderTarget::GetUsePool() const{
	return mImpl->mUsePool;
}

void RenderTarget::SetColorTexture(TexturePtr pTexture)
{
	mImpl->SetColorTexture(pTexture);
}

void RenderTarget::SetDepthTexture(TexturePtr pTexture){
	mImpl->SetDepthTexture(pTexture);
}

CameraPtr RenderTarget::GetLightCamera() const {
	return mImpl->GetLightCamera();
}

const Viewport& RenderTarget::GetViewport() const{
	return mImpl->mViewport;
}

bool RenderTarget::IsGlowSupported() const{
	return mImpl->IsGlowSupported();
}

void RenderTarget::DrawOnEvent(bool set)
{
	mImpl->mDrawOnEvent = set;
}

void RenderTarget::TriggerDrawEvent()
{
	mImpl->mDrawEventTriggered = true;
}

bool RenderTarget::SetSmallSilouetteBuffer(){
	return mImpl->SetSmallSilouetteBuffer();
}

bool RenderTarget::SetBigSilouetteBuffer(){
	return mImpl->SetBigSilouetteBuffer();
}

void RenderTarget::OnRendererOptionChanged(RendererOptionsPtr options, const char* name){
	return mImpl->OnRendererOptionChanged(options, name);
}

TexturePtr RenderTarget::GetShadowMap() const{
	return mImpl->GetShadowMap();
}
}