#include <Engine/StdAfx.h>
#include <Engine/RenderTarget.h>
#include <Engine/Camera.h>
#include <Engine/Renderer.h>
#include <Engine/Scene.h>
#include <Engine/Light.h>
#include <Engine/GaussianDist.h>
#include <Engine/StarDef.h>
#include <../es/shaders/Constants.h>

namespace fastbird
{

RenderTargetId RenderTarget::NextRenderTargetId = 1;

RenderTarget::RenderTarget()
	: mClearColor(0, 0, 0, 1)
	, mDepthClear(1.f)
	, mStencilClear(0)
	, mEnabled(true)
	, mSize(0, 0), mSizeCropped(0, 0)
	, mFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
	, mSRV(false)
	, mMiplevel(false)
	, mCubeMap(false)
	, mWillCreateDepth(false), mUsePool(true), mGlowSet(false)
	, mFrameLuminanceCalced(0)
	, mGaussianDistBlendGlow(0)
	, mGaussianDistBloom(0), mSceneOverride(0), mLockSceneOverride(false), mDrawOnEvent(false), mDrawEventTriggered(false)
{
	mId = NextRenderTargetId++;

	mCamera = FB_NEW(Camera);
	mRenderPipeline = FB_NEW(RenderPipeline);
	mRenderPipeline->SetMinimum();

}

RenderTarget::~RenderTarget()
{
	gFBEnv->_pInternalRenderer->OnRenderTargetDeleted(this);
	if (mGaussianDistBlendGlow)
	{
		FB_DELETE(mGaussianDistBlendGlow);
		mGaussianDistBlendGlow = 0;
	}
	if (mGaussianDistBloom)
	{
		FB_DELETE(mGaussianDistBloom);
		mGaussianDistBloom = 0;
	}
}

bool RenderTarget::CheckOptions(const RenderTargetParam& param)
{
	return param.mSize == mSize && param.mPixelFormat == mFormat && 
		param.mShaderResourceView == mSRV && 
		param.mMipmap == mMiplevel && param.mCubemap == mCubeMap && param.mWillCreateDepth == mWillCreateDepth&& param.mUsePool == mUsePool;
}

RenderPipeline& RenderTarget::GetRenderPipeline() const
{
	assert(mRenderPipeline);
	return *mRenderPipeline;
}

void RenderTarget::SetScene(IScene* scene)
{
	mScene = (Scene*)scene;
}

void RenderTarget::SetSceneOverride(IScene* scene)
{
	if (mLockSceneOverride)
		return;
	mSceneOverride = scene;
}

IScene* RenderTarget::GetScene() const
{
	return mSceneOverride ? mSceneOverride : mScene;
}

Scene* RenderTarget::GetSceneInternal() const
{
	return mSceneOverride ? (Scene*)mSceneOverride : (Scene*)mScene;
}

IScene* RenderTarget::CreateScene()
{
	if (mScene)
	{
		Log(FB_DEFAULT_DEBUG_ARG, "(info) original scene is replaced!");
	}
	mScene = FB_NEW(Scene);
	mScene->SetRttScene(true);
	return mScene;
}

ICamera* RenderTarget::GetCamera() const
{
	return mOverridingCam ? mOverridingCam : mCamera;
}

ICamera* RenderTarget::GetOrCreateOverridingCamera()
{
	if (!mOverridingCam)
	{
		mOverridingCam = FB_NEW(Camera);
	}
	return mOverridingCam;
}


void RenderTarget::RemoveOverridingCamera()
{
	mOverridingCam = 0;
}

const Vec2I& RenderTarget::GetSize() const
{
	return mSize;
}

void RenderTarget::SetClearValues(const Color& color, float z, UINT8 stencil)
{
	mClearColor = color;
	mDepthClear = z;
	mStencilClear = stencil;
}

void RenderTarget::SetClearColor(const Color& color)
{
	mClearColor = color;
}

void RenderTarget::SetClearDepthStencil(float z, UINT8 stencil)
{
	mDepthClear = z;
	mStencilClear = stencil;
}

void RenderTarget::Bind(size_t face)
{
	if (!mEnabled)
		return;

	auto const renderer = gFBEnv->_pInternalRenderer;
	renderer->SetCurRenderTarget(this);
	gFBEnv->mRenderTarget = this;

	if (mRenderTargetTexture)
		mRenderTargetTexture->Unbind();
	ITexture* rt[] = { mRenderTargetTexture };	
	size_t rtViewIndex[] = { face };
	// will update RenderTargetConstants
	renderer->SetRenderTarget(rt, rtViewIndex, 1, mDepthStencilTexture, face);
	renderer->SetViewports(&mViewport, 1);

	renderer->Clear(mClearColor.r(), mClearColor.g(), mClearColor.b(), mClearColor.a(),
		mDepthClear, mStencilClear);

	GetSceneInternal()->SetLightToRenderer();
	
	auto cam = GetCamera();
	renderer->SetCamera(cam);
	
	if (mRenderPipeline->GetStep(RenderSteps::ShadowMap))
	{
		UpdateLightCamera();
	}

	renderer->SetScene(GetScene()); // will update scene constants

	cam->ProcessInputData();	
	
	if (mEnvTexture)
	{
		renderer->SetEnvironmentTextureOverride(mEnvTexture);
	}
	renderer->RestoreRenderStates();
}

void RenderTarget::BindTargetOnly(bool hdr)
{
	if (hdr && mRenderPipeline->GetStep(RenderSteps::HDR) &&
			gFBEnv->pConsole->GetEngineCommand()->r_HDR ){
		SetHDRTarget();
	}
	else{
		auto const renderer = gFBEnv->pRenderer;
		if (mRenderTargetTexture)
			mRenderTargetTexture->Unbind();
		ITexture* rt[] = { mRenderTargetTexture };
		// we need to have 6 glow textures to support for cube map.
		// but don't need to.
		size_t rtViewIndex[] = { mFace };
		renderer->SetRenderTarget(rt, rtViewIndex, 1, mDepthStencilTexture, mFace);
		renderer->SetViewports(&mViewport, 1);
	}
}

bool RenderTarget::Render(size_t face)
{
	if (!mEnabled || !mScene)
		return false;

	if (mDrawOnEvent && !mDrawEventTriggered)
		return false;
	mDrawEventTriggered = false;

	mFace = face;
	auto renderer = gFBEnv->_pInternalRenderer;
	D3DEventMarker mark("RenderTarget");

	BindDepthTexture(false);
	SetCloudVolumeTexture(false);
	BindShadowMap(false);

	Bind(face);

	GetSceneInternal()->PreRender();
	
	auto const engineCmd = gFBEnv->pConsole->GetEngineCommand();
	if (mRenderPipeline->GetStep(RenderSteps::Glow))
	{
		SetGlowRenderTarget();
		renderer->Clear(0.f, 0.f, 0.f, 1.f);
		UnSetGlowRenderTarget();
	}

	{
		D3DEventMarker mark("Shadow pass");
		//UpdateLightCamera is performed in the Bind()
		BindShadowMap(false);
		PrepareShadowMapRendering();
		if (mRenderPipeline->GetStep(RenderSteps::ShadowMap))
		{
			GetSceneInternal()->Render();
		}
		EndShadowMapRendering();
	}
	

	if (mRenderPipeline->GetStep(RenderSteps::Depth))
	{
		D3DEventMarker mark("Depth pass");
		SetDepthRenderTarget(true);
		gFBEnv->mRenderPass = RENDER_PASS::PASS_DEPTH;
		GetSceneInternal()->Render();
		UnsetDepthRenderTarget();
		BindDepthTexture(true);
		gFBEnv->mRenderPass = RENDER_PASS::PASS_NORMAL;
	}

	if (mRenderPipeline->GetStep(RenderSteps::CloudVolume))
	{
		D3DEventMarker mark("Cloud Volumes");
		SetCloudVolumeTarget();
		gFBEnv->mRenderPass = RENDER_PASS::PASS_DEPTH;
		GetSceneInternal()->RenderCloudVolumes();
		BindTargetOnly(false);
		SetCloudVolumeTexture(true);
		//mRenderer->BindNoiseMap();
		gFBEnv->mRenderPass = RENDER_PASS::PASS_NORMAL;
	}

	// god ray pre pass
	if (engineCmd->r_GodRay && mRenderPipeline->GetStep(RenderSteps::GodRay))
	{
		D3DEventMarker mark("GodRay pre occlusion pass");
		SetGodRayRenderTarget();
		gFBEnv->mRenderPass = RENDER_PASS::PASS_GODRAY_OCC_PRE;
		GetSceneInternal()->Render();
		GodRay();
		gFBEnv->mRenderPass = RENDER_PASS::PASS_NORMAL;
	}

	if (engineCmd->r_HDR && mRenderPipeline->GetStep(RenderSteps::HDR))
	{
		// Set HDR RenderTarget
		SetHDRTarget();
		BindDepthTexture(true);
		SetCloudVolumeTexture(true);
		renderer->Clear(0.f, 0.f, 0.f, 1.f);
	}
	else
	{
		BindTargetOnly(false);
	}

	// RENDER
	{
		// Bind Shadow map
		BindShadowMap(true);
		gFBEnv->mSilouetteRendered = false;
		GetSceneInternal()->Render();
		if (gFBEnv->mSilouetteRendered)
			DrawSilouette();
	}

	if (mRenderPipeline->GetStep(RenderSteps::HDR) && engineCmd->r_HDR)
	{
		if (renderer->IsLuminanceOnCpu())
		{
			CalcLuminance();
		}

		if (engineCmd->r_Glow && mRenderPipeline->GetStep(RenderSteps::Glow))
			BlendGlow();
		if (engineCmd->r_GodRay && mRenderPipeline->GetStep(RenderSteps::GodRay))
			BlendGodRay();

		BindTargetOnly(false);
		MeasureLuminanceOfHDRTargetNew();

		BrightPass();

		if (engineCmd->r_Glow && mRenderPipeline->GetStep(RenderSteps::Glow))
		{
			BrightPassToStarSource();
			StarSourceToBloomSource();
			Bloom();
			RenderStarGlare();
		}
		
		ToneMapping();
	}
	Unbind();

	return true;
}

void RenderTarget::Unbind()
{
	auto const renderer = gFBEnv->pRenderer;	
	if (mEnvTexture)
	{
		gFBEnv->pRenderer->SetEnvironmentTextureOverride(0);
	}
}

void RenderTarget::OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	if (!pMouse || !pMouse->IsValid())
		return;
	auto cam = GetCamera();
	cam->SetCurrent(true);
	cam->OnInputFromEngine(pMouse, pKeyboard);
	cam->SetCurrent(false);
}

void RenderTarget::SetEnvTexture(ITexture* texture)
{
	mEnvTexture = texture;
}

void RenderTarget::SetUsePool(bool usePool)
{
	mUsePool = usePool;
}

void RenderTarget::SetColorTexture(ITexture* pTexture)
{
	mRenderTargetTexture = pTexture;
	mSize = pTexture->GetSize();
	auto const renderer = gFBEnv->_pInternalRenderer;
	mSizeCropped = Vec2I(renderer->CropSize8(mSize.x), renderer->CropSize8(mSize.y));
	mFormat = pTexture->GetFormat();
	mViewport.mTopLeftX = 0;
	mViewport.mTopLeftY = 0;
	mViewport.mWidth = (float)mSize.x;
	mViewport.mHeight = (float)mSize.y;
	mViewport.mMinDepth = 0.f;
	mViewport.mMaxDepth = 1.0f;
	mCamera->SetWidth(mSize.x);
	mCamera->SetHeight(mSize.y);
	if (mSize.x < 100 || mSize.y < 100)
	{
		mRenderPipeline->SetStep(RenderSteps::HDR, false);
	}
}


//---------------------------------------------------------------------------
// Additional Targets
//---------------------------------------------------------------------------

void RenderTarget::SetDepthRenderTarget(bool clear)
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	int width = int(mSize.x*.5f);
	int height = int(mSize.y*.5f);
	if (!mDepthTarget)
	{ 
		mDepthTarget = renderer->CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	ITexture* rts[] = { mDepthTarget };
	size_t rtViewIndex[] = { 0 };
	
	auto depthBuffer = renderer->GetTemporalDepthBuffer(Vec2I(width, height));
	renderer->SetRenderTarget(rts, rtViewIndex, 1, depthBuffer, 0);
	Viewport vp = { 0, 0, (float)width, (float)height, 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	if (clear)
		renderer->Clear(1.f, 1.f, 1.f, 1.f, 1.f, 0);
	renderer->LockBlendState();
}

void RenderTarget::UnsetDepthRenderTarget()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	BindTargetOnly(false);
	renderer->UnlockBlendState();
}

void RenderTarget::BindDepthTexture(bool set)
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (set)
	{
		renderer->SetTexture(mDepthTarget, BINDING_SHADER_PS, 5);
	}
	else 
		renderer->SetTexture(0, BINDING_SHADER_PS, 5);
}

void RenderTarget::SetGlowRenderTarget()
{
	if (!mRenderPipeline->GetStep(RenderSteps::Glow))
	{
		BindTargetOnly(true);
		return;
	}
	if (mGlowSet)
		return;

	auto const renderer = gFBEnv->pRenderer;
	if (!mGlowTarget)
	{
		/*
		A pixel shader can be used to render to at least 8 separate render targets,
		all of which must be the same type (buffer, Texture1D, Texture1DArray, etc...).
		Furthermore, all render targets must have the same size in all dimensions (width, height, depth, array size, sample counts).
		Each render target may have a different data format.
		
		You may use any combination of render targets slots (up to 8). However, a resource view cannot be bound to
		multiple render-target-slots simultaneously. A view may be reused as long as the resources are not used simultaneously.
		
		*/
		mGlowTarget = renderer->CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		const char* szStr = FormatString("rt%u_%u_%u_GlowTarget", mId, mSize.x, mSize.y);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTarget, szStr);
		
		mGlowTexture[0] = renderer->CreateTexture(0, (int)(mSize.x * 0.25f), (int)(mSize.y * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		szStr = FormatString("rt%u_%u_%u_GlowTexture0", mId, mSize.x, mSize.y);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTexture[0], szStr);
		
		mGlowTexture[1] = renderer->CreateTexture(0, (int)(mSize.x * 0.25f), (int)(mSize.y * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		szStr = FormatString("rt%u_%u_%u_GlowTexture1", mId, mSize.x, mSize.y);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTexture[1], szStr);
	}

	ITexture* rt[] = { mRenderTargetTexture, mGlowTarget };
	if (mHDRTarget && 
		mRenderPipeline->GetStep(RenderSteps::HDR) && gFBEnv->pConsole->GetEngineCommand()->r_HDR)
	{
		rt[0] = mHDRTarget;
	}
	// we need to have 6 glow textures to support for cube map.
	// but don't need to.
	assert(mFace == 0);
	size_t rtViewIndex[] = { mFace, 0 }; 
	renderer->SetRenderTarget(rt, rtViewIndex, 2, mDepthStencilTexture, mFace);
	Viewport viewports[] = { mViewport, mViewport };
	renderer->SetViewports(viewports, 2);
	mGlowSet = true;

}

void RenderTarget::UnSetGlowRenderTarget()
{
	if (!mRenderPipeline->GetStep(RenderSteps::Glow))
		return;
	if (!mGlowSet)
		return;
	auto const renderer = gFBEnv->pRenderer;

	mGlowSet = false;

	BindTargetOnly(true);
}


void RenderTarget::PrepareShadowMapRendering()
{
	auto cmd = gFBEnv->pConsole->GetEngineCommand();
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mShadowMap)
	{
		int width = (int)std::min(cmd->r_ShadowMapWidth, (int)(mSize.x / 1600.f * cmd->r_ShadowMapWidth));
		int height = (int)std::min(cmd->r_ShadowMapHeight, (int)(mSize.y / 900.f * cmd->r_ShadowMapHeight));
		width = renderer->CropSize8(width);
		height = renderer->CropSize8(height);
		
		width = std::max(16, width);
		height = std::max(16, height);

		mShadowMap = renderer->CreateTexture(0, 
			width, height, 
			PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL_SRV);
		assert(mShadowMap);
		const char* szStr = FormatString("rt%u_%u_%u_ShadowMap", mId, width, height);
		FB_SET_DEVICE_DEBUG_NAME(mShadowMap, szStr);
	}
	
	ITexture* rts[] = { 0 };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, mShadowMap, 0);

	gFBEnv->mRenderPass = PASS_SHADOW;

	renderer->SetShadowMapShader();
	const auto& size = mShadowMap->GetSize();
	Viewport vp = { 0, 0,
		(float)size.x,
		(float)size.y,
		0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->SetCamera(mLightCamera);
	renderer->Clear(0, 0, 0, 0, 1.0f, 0);
}

void RenderTarget::EndShadowMapRendering()
{
	assert(mShadowMap);
	auto const renderer = gFBEnv->pRenderer;
	BindTargetOnly(false);
	gFBEnv->mRenderPass = PASS_NORMAL;
	renderer->SetCamera(GetCamera());
}

void RenderTarget::BindShadowMap(bool bind)
{
	auto const renderer = gFBEnv->pRenderer;
	if (bind && mShadowMap)
		renderer->SetTexture(mShadowMap, BINDING_SHADER_PS, 8);
	else
		renderer->SetTexture(0, BINDING_SHADER_PS, 8);
}

void RenderTarget::SetCloudVolumeTarget()
{
	auto const renderer = gFBEnv->pRenderer;
	if (!mCloudVolumeDepth)
	{
		mCloudVolumeDepth = renderer->CreateTexture(0, mSize.x / 2, mSize .y / 2, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	auto depthBuffer = renderer->GetTemporalDepthBuffer(Vec2I(mSize.x / 2, mSize.y / 2));
	assert(depthBuffer);
	ITexture* rts[] = { mCloudVolumeDepth };
	size_t index[] = { 0 };
	// mTempDepthBufferHalf already filled with scene objects. while writing the depth texture;
	renderer->SetRenderTarget(rts, index, 1, depthBuffer, 0);
	Viewport vp = { 0, 0, mSize.x * .5f, mSize.y * .5f, 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->Clear(0.0f, 0.0f, 0.0f, 0.f, 1.f, 0);
}

void RenderTarget::SetCloudVolumeTexture(bool set)
{
	auto const renderer = gFBEnv->pRenderer;
	if (set)
		renderer->SetTexture(mCloudVolumeDepth, BINDING_SHADER_PS, 6);
	else
		renderer->SetTexture(0, BINDING_SHADER_PS, 6);
}


void RenderTarget::SetGodRayRenderTarget()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mGodRayTarget[0])
	{
		for (int i = 0; i < 2; i++)
		{
			mGodRayTarget[i] = renderer->CreateTexture(0, mSize.x / 2, mSize.y / 2, PIXEL_FORMAT_R8G8B8A8_UNORM,
				BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		mNoMSDepthStencil = renderer->CreateTexture(0, mSize.x / 2, mSize.y / 2, PIXEL_FORMAT_D24_UNORM_S8_UINT,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);
		renderer->GetGodRayPS();
	}

	
	ITexture* rts[] = { mGodRayTarget[1] };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, mNoMSDepthStencil, 0);
	Viewport vp = { 0, 0, (float)mSize.x*.5f, (float)mSize.y*.5f, 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->Clear(0.f, 0.f, 0.f, 1.f, 1.f, 0);
}

void RenderTarget::GodRay()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	ILight* pLight = renderer->GetDirectionalLight(0);
	assert(pLight);
	Vec4 lightPos(GetCamera()->GetPos() + pLight->GetPosition(), 1.f);
	lightPos = GetCamera()->GetViewProjMat() * lightPos; // only x,y nee
	lightPos.x = lightPos.x*.5f + .5f;
	lightPos.y = .5f - lightPos.y*.5f;
	
	ITexture* rts[] = { mGodRayTarget[0] };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0);
	renderer->SetTexture(mGodRayTarget[1], BINDING_SHADER_PS, 0);
	Vec4* pData = (Vec4*)renderer->MapMaterialParameterBuffer();
	if (pData)
	{
		*pData = GetCamera()->GetViewProjMat() * lightPos; // only x,y needed.
		pData->x = lightPos.x;
		pData->y = lightPos.y;
		EngineCommand* pEC = gFBEnv->pConsole->GetEngineCommand();

		pData->z = pEC->r_GodRayDensity; // density
		pData->w = pEC->r_GodRayDecay; // decay
		++pData;
		pData->x = pEC->r_GodRayWeight; // weight
		pData->y =pEC->r_GodRayExposure; // exposure
		renderer->UnmapMaterialParameterBuffer();
	}
	renderer->DrawFullscreenQuad(renderer->GetGodRayPS(), false);

	BindTargetOnly(false);
}


//---------------------------------------------------------------------------
void RenderTarget::SetHDRTarget()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mHDRTarget)
	{
		mHDRTarget = renderer->CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE,
			TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		if (!mHDRTarget)
		{
			Error("Cannot create HDR RenderTarget.");
			return;
		}
		const char* szStr = FormatString("rt%u_%u_%u_HDRTargetTexture", mId, mSize.x, mSize.y);
		FB_SET_DEVICE_DEBUG_NAME(mHDRTarget, szStr);
	}

	ITexture* rts[] = { mHDRTarget };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, mDepthStencilTexture, 0);
	renderer->SetViewports(&mViewport, 1);
}

void RenderTarget::SetSmallSilouetteBuffer()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	Vec2I halfSize((int)(mSize.x * 0.5f), (int)(mSize.y * 0.5f));
	if (!mSmallSilouetteBuffer)
	{
		mSmallSilouetteBuffer = renderer->CreateTexture(0, halfSize.x, halfSize.y, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	auto halfDepthBuffer = renderer->GetTemporalDepthBuffer(halfSize);
	ITexture* rts[] = { mSmallSilouetteBuffer };
	unsigned index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, halfDepthBuffer, 0);
	Viewport vps = { 0.f, 0.f, (float)halfSize.x, (float)halfSize.y, 0.0f, 1.0f };
	renderer->SetViewports(&vps, 1);
	if (!gFBEnv->mSilouetteRendered)
		renderer->Clear(1, 1, 1, 1, 1.f, 0);
}

void RenderTarget::SetBigSilouetteBuffer()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mBigSilouetteBuffer)
	{
		mBigSilouetteBuffer = renderer->CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	auto depthBuffer = renderer->GetTemporalDepthBuffer(mSize);
	ITexture* rts[] = { mBigSilouetteBuffer };
	unsigned index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, depthBuffer, 0);
	Viewport vps = { 0.f, 0.f, (float)mSize.x, (float)mSize.y, 0.0f, 1.0f };
	renderer->SetViewports(&vps, 1);
	if (!gFBEnv->mSilouetteRendered)
		renderer->Clear(1, 1, 1, 1, 1.f, 0);
}

void RenderTarget::DrawSilouette()
{

	BindTargetOnly(true);

	auto const renderer = gFBEnv->_pInternalRenderer;
	
	

	ITexture* ts[] = { mSmallSilouetteBuffer, mBigSilouetteBuffer, mDepthTarget };
	renderer->SetTextures(ts, 3, BINDING_SHADER_PS, 0);
	
	renderer->DrawFullscreenQuad(renderer->GetSilouetteShader(), false);
}


void RenderTarget::CalcLuminance()
{
	if (mFrameLuminanceCalced == gFBEnv->mFrameCounter ||
		gFBEnv->pConsole->GetEngineCommand()->r_HDR == 0)
	{
		return;
	}
	auto const renderer = gFBEnv->pRenderer;
	mFrameLuminanceCalced = gFBEnv->mFrameCounter;
	// todo buffer the stage texture
	SmartPtr<ITexture> stage = renderer->CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_STAGING,
		BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);
	mHDRTarget->CopyToStaging(stage, 0, 0, 0, 0, 0, 0);
	auto mapped = stage->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	if (mapped.pData)
	{
		float accumulated = 0.f;
		Vec3 luminance(0.2125f, 0.7154f, 0.0721f);
		BYTE* data = (BYTE*)mapped.pData;
		unsigned numPixels = mSize.x * mSize.y;
		for (unsigned i = 0; i < numPixels; i++)
		{
			Vec3 target;
			Halfp2Singles(&target, data, 3);
			// Reference : 
			// Reinhard, Erik, Greg Ward, Sumanta Pattanaik, and Paul Debeve,
			// Photographic Tone Reproduction for Digital Images, 
			// ACM Transactions on Graphics, vol. 21, no. 3, pp.267-276, July 2002.
			//accumulated +=  log(target.Dot(luminance)+0.0001f);

			float l = target.Dot(luminance);
			//accumulated += std::max(l, 0.2f);
			accumulated += l;

			data += 8;
		}
		//mLuminance = exp(accumulated / numPixels);
		mLuminance = (accumulated / numPixels);
		Log("Luminance = %f", mLuminance);
	}
}

//---------------------------------------------------------------------------
void RenderTarget::BlendGlow()
{
	if (!mGlowTexture[0])
	{
		return;
	}

	auto const renderer = gFBEnv->_pInternalRenderer;
	Vec2I size((int)(mSize.x*0.25f), (int)(mSize.y*0.25f));
	

	{
		D3DEventMarker mark("Glowing");
		assert(mGlowTarget);
		ITexture* rt[] = { mGlowTexture[1] };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rt, index, 1, 0, 0);
		renderer->SetTexture(mGlowTarget, BINDING_SHADER_PS, 0);		
		renderer->RestoreBlendState();
		Vec2I resol = mGlowTexture[0]->GetSize();
		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		renderer->SetViewports(&vp, 1);

		if (!mGaussianDistBlendGlow)
		{
			mGaussianDistBlendGlow = FB_NEW(GaussianDist);
			mGaussianDistBlendGlow->Calc(mSize.x, mSize.y, 3.f, 1.25f);
		}
		
		// Horizontal Blur		
		BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		memcpy(avSampleOffsets, mGaussianDistBlendGlow->mGaussianDistOffsetX, sizeof(Vec4) * 15);
		memcpy(avSampleWeights, mGaussianDistBlendGlow->mGaussianDistWeightX, sizeof(Vec4) * 15);
		renderer->UnmapBigBuffer();
		// mGlowPS is same with BloomPS except it has the _MULTI_SAMPLE shader define.
		renderer->DrawFullscreenQuad(renderer->GetGlowShader(), false);

		// Vertical Blur
		pData = (BIG_BUFFER*)renderer->MapBigBuffer();
		avSampleOffsets = pData->gSampleOffsets;
		avSampleWeights = pData->gSampleWeights;
		memcpy(avSampleOffsets, mGaussianDistBlendGlow->mGaussianDistOffsetY, sizeof(Vec4) * 15);
		memcpy(avSampleWeights, mGaussianDistBlendGlow->mGaussianDistWeightY, sizeof(Vec4) * 15);
		renderer->UnmapBigBuffer();
		rt[0] = mGlowTexture[0];
		renderer->SetRenderTarget(rt, index, 1, 0, 0);
		renderer->SetTexture(mGlowTexture[1], BINDING_SHADER_PS, 0);
		renderer->DrawFullscreenQuad(renderer->GetGlowShader(), false);
	}

	{
		D3DEventMarker mark("Glow Blend");
		BindTargetOnly(true);
		renderer->SetTexture(mGlowTexture[0], BINDING_SHADER_PS, 0);
		renderer->SetAdditiveBlendState();
		renderer->SetNoDepthStencil();		
		if (renderer->GetMultiSampleCount() == 1)
			renderer->DrawFullscreenQuad(renderer->GetCopyPS(), false);
		else
			renderer->DrawFullscreenQuad(renderer->GetCopyPSMS(), false);
		renderer->RestoreBlendState();
	}
}

////---------------------------------------------------------------------------
void RenderTarget::BlendGodRay()
{
	D3DEventMarker mark("GodRay Blending");
	assert(mGodRayTarget[0]);
	auto const renderer = gFBEnv->_pInternalRenderer;
	renderer->SetTexture(mGodRayTarget[0], BINDING_SHADER_PS, 0);
	renderer->SetAdditiveBlendState();
	renderer->SetNoDepthStencil();
	renderer->DrawFullscreenQuad(renderer->GetCopyPS(), false);
	renderer->RestoreBlendState();
}

////---------------------------------------------------------------------------
void RenderTarget::MeasureLuminanceOfHDRTargetNew()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	renderer->RestoreBlendState();
	renderer->SetNoDepthStencil();
	

	D3DEventMarker mark("Luminance");
	assert(mHDRTarget);
	int dwCurTexture = FB_NUM_TONEMAP_TEXTURES_NEW - 1;
	ITexture* renderTarget = renderer->GetToneMap(dwCurTexture);
	ITexture* rts[] = { renderTarget };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
	renderer->SetTexture(mHDRTarget, BINDING_SHADER_PS, 0);
	bool msaa = renderer->GetMultiSampleCount() > 1;
	if (msaa)
	{
		Vec4* pDest = (Vec4*)renderer->MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mHDRTarget->GetWidth();
			pDest->y = (float)mHDRTarget->GetHeight();
			renderer->UnmapMaterialParameterBuffer();
		}
	}

	const Vec2I& resol = renderTarget->GetSize();
	Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->DrawFullscreenQuad(renderer->GetSampleLumInitialShader(), false);
	--dwCurTexture;

	while (dwCurTexture>0)
	{
		ITexture* src = renderer->GetToneMap(dwCurTexture + 1);
		ITexture* renderTarget = renderer->GetToneMap(dwCurTexture);

		ITexture* rts[] = { renderTarget };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		renderer->SetTexture(src, BINDING_SHADER_PS, 0);

		const Vec2I& resol = renderTarget->GetSize();
		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		renderer->SetViewports(&vp, 1);
		renderer->DrawFullscreenQuad(renderer->GetSampleLumIterativeShader(), false);
		--dwCurTexture;
	}

	// Perform the final pass of the average luminance calculation.
	{
		ITexture* src = renderer->GetToneMap(dwCurTexture + 1);
		ITexture* renderTarget = renderer->GetToneMap(dwCurTexture);
		ITexture* rts[] = { renderTarget };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		renderer->SetTexture(src, BINDING_SHADER_PS, 0);
		const Vec2I& resol = renderTarget->GetSize();
		{Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		renderer->SetViewports(&vp, 1); }

		renderer->DrawFullscreenQuad(renderer->GetSampleLumFinalShader(), false);
	}

	// AdaptedLum
	{
		renderer->SwapLuminanceMap();
		auto luminanceMap0 = renderer->GetLuminanceMap(0);
		ITexture* rts[] = { luminanceMap0 };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		renderer->SetTexture(renderer->GetLuminanceMap(1), BINDING_SHADER_PS, 0);
		renderer->SetTexture(renderer->GetToneMap(0), BINDING_SHADER_PS, 1);
		Viewport vp = { 0, 0, (float)luminanceMap0->GetWidth(), (float)luminanceMap0->GetHeight(), 0.f, 1.f };
		renderer->SetViewports(&vp, 1);
		renderer->DrawFullscreenQuad(renderer->GetCalcAdapedLumShader(), false);
	}
}

//---------------------------------------------------------------------------
void RenderTarget::BrightPass()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mBrightPassTexture)
	{
		Vec2I size( 
			renderer->CropSize8((int)(mSize.x* 0.25f)), 
			renderer->CropSize8((int)(mSize.y * 0.25f))
			);
		mBrightPassTexture = renderer->CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		const char* szStr = FormatString("rt%u_%u_%u_BrightPass", mId, size.x, size.y);
		FB_SET_DEVICE_DEBUG_NAME(mBrightPassTexture, szStr);
	}

	const Vec2I& resol = mBrightPassTexture->GetSize();
	{
		D3DEventMarker mark("Bloom - BrightPass");
		// brightpass
		ITexture* rts[] = { mBrightPassTexture };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;

		ITexture* rvs[] = { mHDRTarget, renderer->GetLuminanceMap(0) };
		renderer->SetTextures(rvs, 2, BINDING_SHADER_PS, 0);

		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		renderer->SetViewports(&vp, 1);

		if (renderer->GetMultiSampleCount() != 1)
		{
			Vec4* pDest = (Vec4*)renderer->MapMaterialParameterBuffer();
			if (pDest)
			{
				pDest->x = (float)mHDRTarget->GetWidth();
				pDest->y = (float)mHDRTarget->GetHeight();
				renderer->UnmapMaterialParameterBuffer();
			}
		}

		renderer->DrawFullscreenQuad(renderer->GetBrightPassPS(), false);
	}
}

//---------------------------------------------------------------------------
void RenderTarget::BrightPassToStarSource()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mStarSourceTex)
	{
		Vec2I starSourceSize(renderer->CropSize8( (int)(mSize.x*0.25f) ),
			renderer->CropSize8( (int)(mSize.y*0.25f) )
			);
		mStarSourceTex = renderer->CreateTexture(0, starSourceSize.x, starSourceSize.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	D3DEventMarker mark("BrightPassToStarSource");

	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	renderer->GetSampleOffsets_GaussBlur5x5(mGlowTarget->GetWidth(), mGlowTarget->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4)* 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4)* 13);
	renderer->UnmapBigBuffer();

	ITexture* rts[] = { mStarSourceTex };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mStarSourceTex->GetWidth(), (float)mStarSourceTex->GetHeight(), 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->Clear(0.f, 0.f, 0.f, 1.f);
	renderer->SetTexture(mGlowTarget, BINDING_SHADER_PS, 0);
	if (renderer->GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)renderer->MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mGlowTarget->GetWidth();
			pDest->y = (float)mGlowTarget->GetHeight();
			renderer->UnmapMaterialParameterBuffer();
		}
	}

	renderer->DrawFullscreenQuad(renderer->GetBlur5x5PS(), false);
}

//---------------------------------------------------------------------------
void RenderTarget::StarSourceToBloomSource()
{
	D3DEventMarker mark("StarSourceToBloomSource");
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mBloomSourceTex)
	{
		Vec2I bloomSourceSize(renderer->CropSize8(mSize.x/8),
			renderer->CropSize8(mSize.y/8));
		mBloomSourceTex = renderer->CreateTexture(0, bloomSourceSize.x, bloomSourceSize.y, PIXEL_FORMAT_R8G8B8A8_UNORM, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}
	auto blur5x5 = renderer->GetBlur5x5PS();
	assert(blur5x5);
	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	renderer->GetSampleOffsets_GaussBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4)* 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4)* 13);
	renderer->UnmapBigBuffer();	

	ITexture* rts[] = { mBloomSourceTex };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0);
	
	Viewport vp = { 0, 0, (float)mBloomSourceTex->GetWidth(), (float)mBloomSourceTex->GetHeight(), 0.f, 1.f };
	renderer->SetViewports(&vp, 1);

	renderer->SetTexture(mBrightPassTexture, BINDING_SHADER_PS, 0);	

	if (renderer->GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)renderer->MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mBrightPassTexture->GetWidth();
			pDest->y = (float)mBrightPassTexture->GetHeight();
			renderer->UnmapMaterialParameterBuffer();
		}
	}

	renderer->DrawFullscreenQuad(blur5x5, false);
}

//---------------------------------------------------------------------------
void RenderTarget::Bloom()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (!mBloomTexture[0])
	{
		Vec2I size(renderer->CropSize8(mSize.x/8),
			renderer->CropSize8(mSize.y/8));
		for (int i = 0; i < FB_NUM_BLOOM_TEXTURES; i++)
		{
			mBloomTexture[i] = renderer->CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
				BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			char buff[255];
			sprintf_s(buff, "rt%u_%u_%u_Bloom(%d) %u", mId, size.x, size.y, i);
			FB_SET_DEVICE_DEBUG_NAME(mBloomTexture[i], buff);
		}
	}

	// blur
	D3DEventMarker mark("Bloom - Blur");
	assert(mBloomSourceTex);
	ITexture* rts[] = { mBloomTexture[2] };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mBloomTexture[2]->GetWidth(), (float)mBloomTexture[2]->GetHeight(), 0.f, 1.f };
	renderer->SetViewports(&vp, 1);

	renderer->SetTexture(mBloomSourceTex, BINDING_SHADER_PS, 0);

	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	renderer->GetSampleOffsets_GaussBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4) * 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4) * 13);
	renderer->UnmapBigBuffer();

	if (renderer->GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)renderer->MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mBloomSourceTex->GetWidth();
			pDest->y = (float)mBloomSourceTex->GetHeight();
			renderer->UnmapMaterialParameterBuffer();
		}
	}

	renderer->DrawFullscreenQuad(renderer->GetBlur5x5PS(), false);

	const Vec2I& resol = mBloomTexture[2]->GetSize();
	// Horizontal Blur
	{
		D3DEventMarker mark("Bloom - Apply hori gaussian filter");
		BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		if (!mGaussianDistBloom)
		{
			mGaussianDistBloom = FB_NEW(GaussianDist);
			mGaussianDistBloom->Calc(resol.x, resol.y, 3.f, 1.25f);
		}
		memcpy(avSampleOffsets, mGaussianDistBloom->mGaussianDistOffsetX, sizeof(Vec4) * 15);
		memcpy(avSampleOffsets, mGaussianDistBloom->mGaussianDistWeightX, sizeof(Vec4) * 15);
		renderer->UnmapBigBuffer();

		ITexture* rts[] = { mBloomTexture[1] };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0);
		renderer->SetTexture(mBloomTexture[2], BINDING_SHADER_PS, 0);
		renderer->DrawFullscreenQuad(renderer->GetBloomPS(), false);
	}

	// Vertical Blur
	{
		D3DEventMarker mark("Bloom - Apply verti gaussian filter");
		BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		memcpy(avSampleOffsets, mGaussianDistBloom->mGaussianDistOffsetY, sizeof(Vec4) * 15);
		memcpy(avSampleWeights, mGaussianDistBloom->mGaussianDistWeightY, sizeof(Vec4) * 15);
		renderer->UnmapBigBuffer();

		renderer->SetTexture(0, BINDING_SHADER_PS, 0);
		renderer->SetTexture(0, BINDING_SHADER_PS, 1);
		renderer->SetTexture(0, BINDING_SHADER_PS, 2);
		rts[0] = mBloomTexture[0];
		renderer->SetRenderTarget(rts, index, 1, 0, 0);
		renderer->SetTexture(mBloomTexture[1], BINDING_SHADER_PS, 0);

		renderer->DrawFullscreenQuad(renderer->GetBloomPS(), false);
	}
}

static const int starGlareMaxPasses = 3;
static const int starGlareSamples = 8;
static const float starGlareChromaticAberration = 0.5f;
static const float starGlareInclination = HALF_PI;
static StarDef* starGlareDef = 0;
//static  function
void RenderTarget::ReleaseStarDef()
{
	FB_DELETE(starGlareDef);
}
void CalcStarGlareConst(Vec4 s_aaColor[starGlareMaxPasses][starGlareSamples])
{
	static const Color s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f);
	for (int p = 0; p < starGlareMaxPasses; ++p)
	{
		float ratio;
		ratio = (float)(p + 1) / (float)starGlareMaxPasses;

		for (int s = 0; s < starGlareSamples; s++)
		{
			Color chromaticAberrColor = Lerp(StarDef::GetChromaticAberrationColor(s), s_colorWhite, ratio);
			s_aaColor[p][s] = Lerp(s_colorWhite, chromaticAberrColor, starGlareChromaticAberration).GetVec4();
		}
	}
	StarDef::InitializeStatic();
	starGlareDef = FB_NEW(StarDef);
	starGlareDef->Initialize(STLT_VERTICAL);
}

//---------------------------------------------------------------------------
void RenderTarget::RenderStarGlare()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	if (mStarTextures[0] == 0)
	{
		Vec2I size( renderer->CropSize8((int)(mSize.x * 0.25f)), renderer->CropSize8((int)(mSize.y*0.25f)) );
		for (int i = 0; i < FB_NUM_STAR_TEXTURES; ++i)
		{
			mStarTextures[i] = renderer->CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
	}

	ITexture* rts[] = { mStarTextures[0] };
	size_t index[] = { 0 };
	renderer->SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mStarTextures[0]->GetWidth(), (float)mStarTextures[0]->GetHeight(), 0.f, 1.f };
	renderer->SetViewports(&vp, 1);
	renderer->Clear(0.f, 0.f, 0.f, 1.f);

	const float fTanFoV = GetCamera()->GetFOV();
	const Color vWhite(1.0f, 1.0f, 1.0f, 1.0f);	
	static Vec4 s_aaColor[starGlareMaxPasses][starGlareSamples];
	static bool s_aaColorCalced = false;
	if (!s_aaColorCalced)
	{
		s_aaColorCalced = true;
		CalcStarGlareConst(s_aaColor);

	}

	Vec4 avSampleWeights[FB_MAX_SAMPLES];
	Vec4 avSampleOffsets[FB_MAX_SAMPLES];

	//mAdditiveBlendState->Bind();

	float srcW = (float)mStarSourceTex->GetWidth();
	float srcH = (float)mStarSourceTex->GetHeight();

	
	float radOffset;
	radOffset = starGlareInclination + starGlareDef->m_fInclination;
	
	ITexture* pSrcTexture = 0;
	ITexture* pDestTexture = 0;

	// Direction loop
	for (int d = 0; d < starGlareDef->m_nStarLines; ++d)
	{
		CONST STARLINE& starLine = starGlareDef->m_pStarLine[d];

		pSrcTexture = mStarSourceTex;

		float rad = radOffset + starLine.fInclination;
		float sn = sinf(rad), cs = cosf(rad);
		Vec2 vtStepUV(sn / srcW * starLine.fSampleLength,
			cs / srcH * starLine.fSampleLength);

		float attnPowScale;
		attnPowScale = (fTanFoV + 0.1f) * 1.0f *
			(160.0f + 120.0f) / (srcW + srcH) * 1.2f;

		// 1 direction expansion loop
		int iWorkTexture;
		iWorkTexture = 1;
		for (int p = 0; p < starLine.nPasses; p++)
		{
			if (p == starLine.nPasses - 1)
			{
				// Last pass move to other work buffer
				pDestTexture = mStarTextures[d + 4];
			}
			else
			{
				pDestTexture = mStarTextures[iWorkTexture];
			}

			// Sampling configration for each stage
			for (int i = 0; i < starGlareSamples; ++i)
			{
				float lum;
				lum = powf(starLine.fAttenuation, attnPowScale * i);

				avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] *
					lum * (p + 1.0f) * 0.5f;


				// Offset of sampling coordinate
				avSampleOffsets[i].x = vtStepUV.x * i;
				avSampleOffsets[i].y = vtStepUV.y * i;
				if (fabs(avSampleOffsets[i].x) >= 0.9f ||
					fabs(avSampleOffsets[i].y) >= 0.9f)
				{
					avSampleOffsets[i].x = 0.0f;
					avSampleOffsets[i].y = 0.0f;
					avSampleWeights[i] *= 0.0f;
				}

			}
			BIG_BUFFER* pData = (BIG_BUFFER*)renderer->MapBigBuffer();
			memcpy(pData->gSampleOffsets, avSampleOffsets, sizeof(Vec4)* FB_MAX_SAMPLES);
			memcpy(pData->gSampleWeights, avSampleWeights, sizeof(Vec4)* FB_MAX_SAMPLES);
			renderer->UnmapBigBuffer();

			ITexture* rts[] = { pDestTexture };
			size_t index[] = { 0 };
			renderer->SetRenderTarget(rts, index, 1, 0, 0);
			renderer->SetTexture(pSrcTexture, BINDING_SHADER_PS, 0);
			renderer->DrawFullscreenQuad(renderer->GetStarGlareShader(), false);

			// Setup next expansion
			vtStepUV *= starGlareSamples;
			attnPowScale *= starGlareSamples;

			// Set the work drawn just before to next texture source.
			pSrcTexture = mStarTextures[iWorkTexture];

			iWorkTexture += 1;
			if (iWorkTexture > 2)
			{
				iWorkTexture = 1;
			}
		}
	}

	pDestTexture = mStarTextures[0];

	std::vector<ITexture*> textures;
	textures.reserve(starGlareDef->m_nStarLines);
	for (int i = 0; i < starGlareDef->m_nStarLines; i++)
	{
		textures.push_back(mStarTextures[i + 4]);
		avSampleWeights[i] = vWhite.GetVec4() * (1.0f / (FLOAT)starGlareDef->m_nStarLines);
	}

	{
		ITexture* rts[] = { pDestTexture };
		size_t index[] = { 0 };
		renderer->SetRenderTarget(rts, index, 1, 0, 0); 
	}
	renderer->SetTextures(&textures[0], textures.size(), BINDING_SHADER_PS, 0);
	
	switch (starGlareDef->m_nStarLines)
	{
	case 2:
		renderer->DrawFullscreenQuad(renderer->GetMergeTexturePS(), false);
		break;

	default:
		assert(0);
	}

	textures.clear();
	for (int i = 0; i < starGlareDef->m_nStarLines; i++)
	{
		textures.push_back(0);
	}
	renderer->SetTextures(&textures[0], starGlareDef->m_nStarLines, BINDING_SHADER_PS, 0);
}

//---------------------------------------------------------------------------
void RenderTarget::ToneMapping()
{
	D3DEventMarker mark("ToneMapping");
	auto const renderer = gFBEnv->_pInternalRenderer;
	ITexture* textures[] = { mHDRTarget, renderer->GetLuminanceMap(0), mBloomTexture[0], mStarTextures[0] };
	BindTargetOnly(false);
	renderer->SetTextures(textures, 4, BINDING_SHADER_PS, 0); 
	if (renderer->IsLuminanceOnCpu())
	{
		Vec4* pData = (Vec4*)renderer->MapMaterialParameterBuffer();
		if (pData)
		{
			*pData = float4(mLuminance, 0, 0, 0);
			renderer->UnmapMaterialParameterBuffer();
		}
	}	

	renderer->DrawFullscreenQuad(renderer->GetToneMappingPS(), false);
	renderer->RestoreDepthStencilState();
	renderer->SetTexture(0, BINDING_SHADER_PS, 3);
}

void RenderTarget::DeleteShadowMap()
{
	mShadowMap = 0;
}

void RenderTarget::UpdateLightCamera()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	auto cam = renderer->GetCamera();
	if (!mLightCamera)
	{
		mLightCamera = FB_NEW(Camera);
		mLightCamera->SetOrthogonal(true);
		auto cmd = gFBEnv->pConsole->GetEngineCommand();
		int width = (int)std::min(cmd->r_ShadowCamWidth, mSize.x * (cmd->r_ShadowCamWidth / 1600.f));
		int height = (int)std::min(cmd->r_ShadowCamHeight, mSize.y * (cmd->r_ShadowCamHeight / 900.f));
		width = std::max(16, width);
		height = std::max(16, height);
		mLightCamera->SetWidth(width);
		mLightCamera->SetHeight(height);
		mLightCamera->SetNearFar(cmd->r_ShadowNear, cmd->r_ShadowFar);
	}

	auto target = cam->GetTarget();
	float shadowCamDist =
		gFBEnv->pConsole->GetEngineCommand()->r_ShadowCamDist;
	auto light = GetSceneInternal()->GetLight(0);
	assert(light);
	if (target && target->GetBoundingVolume()->GetRadius() < shadowCamDist)
	{
		mLightCamera->SetPos(target->GetPos() + light->GetPosition() *shadowCamDist);
	}
	else
	{
		mLightCamera->SetPos(cam->GetPos() + light->GetPosition() * shadowCamDist);
	}
	mLightCamera->SetDir(-light->GetPosition());
}

void RenderTarget::SetLightCamWidth(float width)
{
	if (mLightCamera)
	{
		mLightCamera->SetWidth((int)std::min(width, mSize.x * (width / 1600)));
	}
}

void RenderTarget::SetLightCamHeight(float height)
{
	if (mLightCamera)
	{
		mLightCamera->SetHeight((int)std::min(height, mSize.y * (height / 900)));
	}
}

void RenderTarget::SetLightCamNear(float n)
{
	if (mLightCamera)
	{
		float on, of;
		mLightCamera->GetNearFar(on, of);
		mLightCamera->SetNearFar(n, of);
	}
}

void RenderTarget::SetLightCamFar(float f)
{
	float on, of;
	mLightCamera->GetNearFar(on, of);
	mLightCamera->SetNearFar(on, f);
}

void RenderTarget::DrawOnEvent(bool set)
{
	mDrawOnEvent = set;
}

void RenderTarget::TriggerDrawEvent()
{
	mDrawEventTriggered = true;
}


}