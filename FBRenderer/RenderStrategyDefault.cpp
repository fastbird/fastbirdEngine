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
#include "RenderStrategyDefault.h"
#include "Texture.h"
#include "GaussianDistribution.h"
#include "RenderEventMarker.h"
#include "RenderTarget.h"
#include "Renderer.h"
#include "RenderParam.h"
#include "GaussianDistribution.h"
#include "StarDef.h"
#include "RendererOptions.h"
#include "SystemTextures.h"
#include "ResourceProvider.h"
#include "ResourceTypes.h"
#include "Camera.h"
#include "CascadedShadowsManager.h"
#include "FBSceneManager/IScene.h"
#include "FBSceneManager/DirectionalLight.h"
#include "FBStringLib/StringLib.h"
#include "FBSceneObjectFactory/SceneObjectType.h"
#include "FBSceneManager/ISpatialObject.h"
#include "FBMathLib/BoundingVolume.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

static const int FB_NUM_BLOOM_TEXTURES = 3;
static const int FB_NUM_STAR_TEXTURES = 12;
static const int starGlareMaxPasses = 3;
static const int starGlareSamples = 8;
static const Real starGlareChromaticAberration = 0.5f;
static const Real starGlareInclination = HALF_PI;

static Vec4 s_aaColor[starGlareMaxPasses][starGlareSamples];
class RenderStrategyDefault::Impl{
public:
	static const int FB_MAX_SAMPLES = 16;
	RenderStrategyDefaultWeakPtr mSelfPtr;
	ISceneWeakPtr mScene;
	RenderTargetWeakPtr mRenderTarget;
	Vec2I mSize; // Render target size.
	RenderTargetId mId;
	TexturePtr mDepthTarget;
	TexturePtr mGlowTarget;
	TexturePtr mGlowTexture[2];
	bool mGlowSet;
	bool mMain;
	//CameraPtr mLightCamera;
	//TexturePtr mShadowMap;
	TexturePtr mCloudVolumeDepth;
	ShaderPtr mCloudDepthWriteShader;
	TexturePtr mGodRayTarget[2]; // half resolution; could be shared.
	TexturePtr mNoMSDepthStencil;
	TexturePtr mHDRTarget;
	TexturePtr mSmallSilouetteBuffer;
	TexturePtr mBigSilouetteBuffer;
	unsigned mFrameLuminanceCalced;
	Real mLuminance;
	GaussianDistPtr mGaussianDistBlendGlow;
	GaussianDistPtr mGaussianDistBloom;
	TexturePtr mBrightPassTexture;
	TexturePtr mStarSourceTex;
	TexturePtr mBloomSourceTex;
	TexturePtr mBloomTexture[FB_NUM_BLOOM_TEXTURES];
	TexturePtr mStarTextures[FB_NUM_STAR_TEXTURES];
	size_t mRenderingFace;
	StarDef* mStarGlareDef;

	//-------------------------------------------------------------------
	Impl()
		:mGlowSet(false), mFrameLuminanceCalced(0), mLuminance(0.5f)
		, mRenderingFace(0)
		, mStarGlareDef(0)
		, mMain(false)
	{
		static bool s_aaColorCalced = false;
		if (!s_aaColorCalced)
		{
			s_aaColorCalced = true;
			CalcStarGlareConst(s_aaColor);
		}

		if (!mStarGlareDef){
			mStarGlareDef = FB_NEW(StarDef);
			mStarGlareDef->Initialize(STLT_VERTICAL);
		}
	}
	~Impl(){
		FB_DELETE(mStarGlareDef);
	}

	//-------------------------------------------------------------------
	// IRenderStrategy
	//-------------------------------------------------------------------
	void SetScene(IScenePtr scene){
		auto prevScene = mScene.lock();
		if (prevScene)
			prevScene->RemoveSceneObserver(ISceneObserver::Timing, mSelfPtr.lock());
		mScene = scene;
		if (scene) {
			scene->AddSceneObserver(ISceneObserver::Timing, mSelfPtr.lock());
		}
	}

	void SetRenderTarget(RenderTargetPtr renderTarget){
		mRenderTarget = renderTarget;
		mSize = renderTarget->GetSize();
		mId = renderTarget->GetId();		
	}

	void Render(size_t face){
		auto scene = mScene.lock();
		if (!scene)
			return;
		auto renderTarget = mRenderTarget.lock();
		auto& renderer = Renderer::GetInstance();
		if (!scene || !renderTarget)
			return;

		auto provider = renderer.GetResourceProvider();
		renderer.SetSystemTexture(SystemTextures::GGXPrecalc, provider->GetTexture(ResourceTypes::Textures::GGX));

		RenderEventMarker marker("Rendering with default strategy.");
		RenderParam renderParam;
		RenderParamOut renderParamOut;
		memset(&renderParam, 0, sizeof(RenderParam));
		memset(&renderParamOut, 0, sizeof(RenderParamOut));
		ClearSilouetteBuffer();
		DepthTexture(false);
		CloudVolumeTexture(false);
		renderer.SetBindShadowMap(false);
		renderTarget->Bind(face);
		RenderParam param;
		memset(&param, 0, sizeof(RenderParam));
		param.mCamera = renderer.GetCamera().get();
		scene->MakeVisibleSet(param.mCamera);
		scene->PreRender(param, 0);
		mGlowSet = false;
		GlowTarget(true);
		auto& clearcolor = renderTarget->GetClearColor();
		renderer.Clear(clearcolor.r(), clearcolor.g(), clearcolor.b(), clearcolor.a());

		GlowTarget(false);
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
			//renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, 0);
			DepthTarget(false, false);
			DepthTexture(true);
		}
		{
			RenderEventMarker marker("Cloud Volume Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_DEPTH;
			CloudVolumeTarget(true);			
			renderer.SetLockDepthStencilState(true);
			provider->BindBlendState(ResourceTypes::BlendStates::BlueMask);
			renderParam.mCamera = renderer.GetCamera().get();
			//renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, 0);
			renderer.SetLockDepthStencilState(false);
			DepthWriteShaderCloud();
			provider->BindRasterizerState(ResourceTypes::RasterizerStates::CullFrontFace);
			provider->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthWrite_LessEqual, 0);
			provider->BindBlendState(ResourceTypes::BlendStates::GreenAlphaMaskAddMinus);
			scene->PreRenderCloudVolumes(renderParam, 0);
			scene->RenderCloudVolumes(renderParam, 0);
			renderer.RestoreRasterizerState();
			provider->BindBlendState(ResourceTypes::BlendStates::RedAlphaMaskAddAdd);
			scene->RenderCloudVolumes(renderParam, 0);
			renderer.RestoreRasterizerState();
			renderer.RestoreBlendState();
			renderer.RestoreDepthStencilState();
			CloudVolumeTarget(false);
		}
		{
			RenderEventMarker marker("GodRay Pre-occlusion Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			GodRayTarget(true);
			renderParam.mRenderPass = PASS_GODRAY_OCC_PRE;
			renderParam.mCamera = renderer.GetCamera().get();
			//renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, 0);
			GodRay();
		}
		auto forceWireframe = renderTarget->GetForceWireframe();
		{
			RenderEventMarker marker("Main Render Pass");
			memset(&renderParam, 0, sizeof(RenderParam));
			memset(&renderParamOut, 0, sizeof(RenderParamOut));
			renderParam.mRenderPass = PASS_NORMAL;
			HDRTarget(true);
			auto& clearcolor = renderTarget->GetClearColor();
			renderer.Clear(clearcolor.r(), clearcolor.g(), clearcolor.b(), clearcolor.a());
			DepthTexture(true);
			CloudVolumeTexture(true);
			renderer.SetBindShadowMap(true);
			renderParam.mCamera = renderer.GetCamera().get();
			
			if (forceWireframe) {
				renderer.GetResourceProvider()->BindRasterizerState(ResourceTypes::RasterizerStates::WireFrame);				
				RasterizerState::SetLock(true);
			}
			//renderParam.mLightCamera = mLightCamera.get();
			scene->Render(renderParam, &renderParamOut);
			if (forceWireframe) {
				RasterizerState::SetLock(false);
				renderer.GetResourceProvider()->BindRasterizerState(ResourceTypes::RasterizerStates::Default);
			}
			

			if (renderParamOut.mSilouetteRendered)
				Silouette();
		}

		if (forceWireframe)
		{
			RenderEventMarker marker("HDR simple copy");
			HDRTarget(false);
			SimpleToneMapping();
		}
		else {
			{
				RenderEventMarker marker("Blend Glow and God Ray");
				BlendGlow();
				BlendGodRay();
				HDRTarget(false);
			}
			{
				RenderEventMarker marker("HDR and Bloom");
				MeasureLuminanceOfHDRTarget();
				BrightPass();
				BrightPassToStarSource();
				StarSourceToBloomSource();
				Bloom();
				RenderStarGlare();
				ToneMapping();
			}
		}
		renderTarget->Unbind();
	}


	bool SetSmallSilouetteBuffer(){
		auto& renderer = Renderer::GetInstance();
		RenderEventMarker mark("Rendering small silouette buffer");
		auto rt = mRenderTarget.lock();
		const auto& size = rt->GetSize();
		Vec2I halfSize((int)(size.x * 0.5f), (int)(size.y * 0.5f));
		if (!mSmallSilouetteBuffer)
		{
			mSmallSilouetteBuffer = renderer.CreateTexture(0, halfSize.x, halfSize.y, PIXEL_FORMAT_R16_FLOAT, 
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		auto halfDepthBuffer = renderer.GetTemporalDepthBuffer(halfSize, "SmallSilouette");
		TexturePtr rts[] = { mSmallSilouetteBuffer };
		unsigned index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, halfDepthBuffer, 0);
		Viewport vps = { 0, 0, (float)halfSize.x, (float)halfSize.y, 0.0f, 1.0f };
		renderer.SetViewports(&vps, 1);
		return true;
	}

	bool SetBigSilouetteBuffer(){
		auto& renderer = Renderer::GetInstance();
		RenderEventMarker mark("Rendering big silouette buffer");
		auto rt = mRenderTarget.lock();
		const auto& size = rt->GetSize();
		if (!mBigSilouetteBuffer)
		{
			mBigSilouetteBuffer = renderer.CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R16_FLOAT, 
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		auto depthBuffer = renderer.GetTemporalDepthBuffer(size, "BigSilouette");
		TexturePtr rts[] = { mBigSilouetteBuffer };
		unsigned index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, depthBuffer, 0);
		Viewport vps = { 0, 0, (float)size.x, (float)size.y, 0.0f, 1.0f };
		renderer.SetViewports(&vps, 1);
		return true;
	}

	//-------------------------------------------------------------------
	void ClearSilouetteBuffer(){
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		{
			Vec2I halfSize((int)(mSize.x * 0.5f), (int)(mSize.y * 0.5f));
			if (!mSmallSilouetteBuffer)
			{
				mSmallSilouetteBuffer = renderer.CreateTexture(0, halfSize.x, halfSize.y, PIXEL_FORMAT_R16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			}
			TexturePtr rts[] = { mSmallSilouetteBuffer };
			unsigned index[] = { 0 };
			auto halfDepthBuffer = renderer.GetTemporalDepthBuffer(halfSize, "SmallSilouette");
			renderer.SetRenderTarget(rts, index, 1, halfDepthBuffer, 0);
			Viewport vps = { 0, 0, (float)halfSize.x, (float)halfSize.y, 0.0f, 1.0f };
			renderer.SetViewports(&vps, 1);
			renderer.Clear(1, 1, 1, 1, 1, 0);
		}
		{
			if (!mBigSilouetteBuffer)
			{
				mBigSilouetteBuffer = renderer.CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			}
			TexturePtr rts[] = { mBigSilouetteBuffer };
			unsigned index[] = { 0 };
			auto depthBuffer = renderer.GetTemporalDepthBuffer(mSize, "BigSilouette");
			renderer.SetRenderTarget(rts, index, 1, depthBuffer, 0);
			Viewport vps = { 0, 0, (float)mSize.x, (float)mSize.y, 0.0f, 1.0f };
			renderer.SetViewports(&vps, 1);
			renderer.Clear(1, 1, 1, 1, 1, 0);
		}
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
				mDepthTarget = renderer.CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
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

	void GlowTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		const size_t secondSlot = 1;
		if (bind){
			if (!renderer.GetRendererOptions()->r_Glow){
				return;
			}

			if (mGlowSet)
				return;

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
				mGlowTarget = renderer.CreateTexture(0, mSize.x, mSize.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
				mGlowTarget->SetDebugName(FormatString("rt%u_%u_%u_GlowTarget", rt->GetId(), mSize.x, mSize.y).c_str());

				mGlowTexture[0] = renderer.CreateTexture(0, (int)(mSize.x * 0.25f), (int)(mSize.y * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
				mGlowTexture[0]->SetDebugName(FormatString("rt%u_%u_%u_GlowTexture0", rt->GetId(), mSize.x*0.25f, mSize.y*0.25f).c_str());				

				mGlowTexture[1] = renderer.CreateTexture(0, (int)(mSize.x * 0.25f), (int)(mSize.y * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
				mGlowTexture[1]->SetDebugName(FormatString("rt%u_%u_%u_GlowTexture1", rt->GetId(), mSize.x*0.25f, mSize.y*0.25f).c_str());
			}			
			renderer.SetRenderTargetAtSlot(mGlowTarget, mRenderingFace, secondSlot);
			mGlowSet = true;
		}
		else{
			if (!mGlowSet)
				return;
			mGlowSet = false;
			renderer.SetRenderTargetAtSlot(0, mRenderingFace, secondSlot);
		}
	}

	void DepthWriteShaderCloud(){
		auto& renderer = Renderer::GetInstance();
		renderer.SetDepthWriteShaderCloud();
	}

	void CloudVolumeTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		if (bind){

			const auto& size = mSize;
			if (!mCloudVolumeDepth)
			{
				mCloudVolumeDepth = renderer.CreateTexture(0, size.x / 2, size.y / 2, PIXEL_FORMAT_R16G16B16A16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			}
			auto depthBuffer = renderer.GetTemporalDepthBuffer(Vec2I(size.x / 2, size.y / 2), "any");
			assert(depthBuffer);
			TexturePtr rts[] = { mCloudVolumeDepth };
			size_t index[] = { 0 };
			// mTempDepthBufferHalf already filled with scene objects. while writing the depth texture;
			renderer.SetRenderTarget(rts, index, 1, depthBuffer, 0);
			Viewport vp = { 0, 0, size.x * .5f, size.y * .5f, 0, 1 };
			renderer.SetViewports(&vp, 1);
			renderer.Clear(0.0f, 0.0f, 0.0f, 0, 1, 0);
		}
		else{
			auto rt = mRenderTarget.lock();
			rt->BindTargetOnly(false);
		}
	}

	void CloudVolumeTexture(bool set)
	{
		auto& renderer = Renderer::GetInstance();
		if (set)
			renderer.SetSystemTexture(SystemTextures::CloudVolume, mCloudVolumeDepth);
		else
			renderer.SetSystemTexture(SystemTextures::CloudVolume, 0);
	}


	void GodRayTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		if (bind){
			const auto& size = mSize;
			if (!mGodRayTarget[0])
			{
				for (int i = 0; i < 2; i++)
				{
					mGodRayTarget[i] = renderer.CreateTexture(0, size.x / 2, size.y / 2, PIXEL_FORMAT_R8G8B8A8_UNORM,
						1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
				}
				mNoMSDepthStencil = renderer.CreateTexture(0, size.x / 2, size.y / 2, PIXEL_FORMAT_D24_UNORM_S8_UINT,
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);
			}


			TexturePtr rts[] = { mGodRayTarget[1] };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, mNoMSDepthStencil, 0);
			Viewport vp = { 0, 0, size.x*.5f, size.y*.5f, 0, 1 };
			renderer.SetViewports(&vp, 1);
			renderer.Clear(0, 0, 0, 1, 1, 0);
		}
		else{

		}
	}

	void GodRay()
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		auto scene = mScene.lock();
		if (scene){
			const auto& lightDir = scene->GetMainLightDirection();
			auto camera = rt->GetCamera();
			Vec4 lightPos(camera->GetPosition() + lightDir, 1);
			lightPos = camera->GetMatrix(Camera::ViewProj) * lightPos; // only x,y nee
			lightPos.x = lightPos.x*.5f + .5f;
			lightPos.y = .5f - lightPos.y*.5f;

			TexturePtr rts[] = { mGodRayTarget[0] };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, 0, 0);
			mGodRayTarget[1]->Bind(SHADER_TYPE_PS, 0);
			Vec4f* pData = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pData)
			{
				*pData = camera->GetMatrix(Camera::ViewProj) * lightPos; // only x,y needed.
				pData->x = (float)lightPos.x;
				pData->y = (float)lightPos.y;
				auto pEC = renderer.GetRendererOptions();

				pData->z = pEC->r_GodRayDensity; // density
				pData->w = pEC->r_GodRayDecay; // decay
				++pData;
				pData->x = pEC->r_GodRayWeight; // weight
				pData->y = pEC->r_GodRayExposure; // exposure
				renderer.UnmapShaderConstantsBuffer();
			}
			auto provider = renderer.GetResourceProvider();
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::GodRayPS), false);
		}
		rt->BindTargetOnly(false);
	}


	//---------------------------------------------------------------------------
	void HDRTarget(bool bind)
	{
		auto& renderer = Renderer::GetInstance();
		const auto& size = mSize;
		if (!mHDRTarget)
		{
			mHDRTarget = renderer.CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, 
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
			if (!mHDRTarget)
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Cannot create HDR RenderTarget.");
				return;
			}
			mHDRTarget->SetDebugName(FormatString("rt%u_%u_%u_HDRTargetTexture", mId, size.x, size.y).c_str());
		}

		TexturePtr rts[] = { mHDRTarget };
		size_t index[] = { 0 };
		auto rt = mRenderTarget.lock();
		renderer.SetRenderTarget(rts, index, 1, rt->GetDepthStencilTexture(), 0);
		renderer.SetViewports(&rt->GetViewport(), 1);
		mGlowSet = false;
	}

	void Silouette()
	{
		auto rt = mRenderTarget.lock();
		rt->BindTargetOnly(true);
		auto& renderer = Renderer::GetInstance();

		TexturePtr ts[] = { mSmallSilouetteBuffer, mBigSilouetteBuffer, mDepthTarget };
		renderer.SetTextures(ts, 3, SHADER_TYPE_PS, 0);
		auto provider = renderer.GetResourceProvider();
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::SilouettePS), false);
	}

	//---------------------------------------------------------------------------
	void BlendGlow()
	{
		if (!mGlowTexture[0])
		{
			return;
		}

		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		const auto& oriSize = mSize;
		Vec2I size((int)(oriSize.x*0.25f), (int)(oriSize.y*0.25f));
		{
			RenderEventMarker marker("Glowing");
			assert(mGlowTarget);
			TexturePtr rt[] = { mGlowTexture[1] };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rt, index, 1, 0, 0);
			mGlowTarget->Bind(SHADER_TYPE_PS, 0);
			renderer.RestoreBlendState();
			Vec2I resol = mGlowTexture[0]->GetSize();
			Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0, 1 };
			renderer.SetViewports(&vp, 1);

			if (!mGaussianDistBlendGlow)
			{
				mGaussianDistBlendGlow = GaussianDistPtr(FB_NEW(GaussianDist), [](GaussianDist* obj){ delete obj; });
				mGaussianDistBlendGlow->Calc(oriSize.x, oriSize.y, 3, 1.25f);
			}

			// Horizontal Blur		
			BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
			Vec4f* avSampleOffsets = pData->gSampleOffsets;
			Vec4f* avSampleWeights = pData->gSampleWeights;
			memcpy(avSampleOffsets, mGaussianDistBlendGlow->mGaussianDistOffsetX, sizeof(Vec4f) * 15);
			memcpy(avSampleWeights, mGaussianDistBlendGlow->mGaussianDistWeightX, sizeof(Vec4f) * 15);
			renderer.UnmapBigBuffer();
			// mGlowPS is same with BloomPS except it has the _MULTI_SAMPLE shader define.
			renderer.DrawFullscreenQuad(renderer.GetResourceProvider()->GetShader(ResourceTypes::Shaders::GlowPS), false);

			// Vertical Blur
			pData = (BIG_BUFFER*)renderer.MapBigBuffer();
			avSampleOffsets = pData->gSampleOffsets;
			avSampleWeights = pData->gSampleWeights;
			memcpy(avSampleOffsets, mGaussianDistBlendGlow->mGaussianDistOffsetY, sizeof(Vec4f) * 15);
			memcpy(avSampleWeights, mGaussianDistBlendGlow->mGaussianDistWeightY, sizeof(Vec4f) * 15);
			renderer.UnmapBigBuffer();
			rt[0] = mGlowTexture[0];
			renderer.SetRenderTarget(rt, index, 1, 0, 0);
			mGlowTexture[1]->Bind(SHADER_TYPE_PS, 0);
			renderer.DrawFullscreenQuad(renderer.GetResourceProvider()->GetShader(ResourceTypes::Shaders::GlowPS), false);
		}

	{
		RenderEventMarker marker("Glow Blend");
		rt->BindTargetOnly(true);
		mGlowTexture[0]->Bind(SHADER_TYPE_PS, 0);
		auto provider = renderer.GetResourceProvider();
		provider->BindBlendState(ResourceTypes::BlendStates::AdditiveKeepAlpha);
		//provider->BindBlendState(ResourceTypes::BlendStates::Additive);
		provider->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthStencil, 0);
		if (renderer.GetMultiSampleCount() == 1)
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::CopyPS), false);
		else
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::CopyPSMS), false);
		renderer.RestoreBlendState();
	}
	}

	////---------------------------------------------------------------------------
	void BlendGodRay()
	{
		RenderEventMarker marker("GodRay Blending");
		assert(mGodRayTarget[0]);
		auto& renderer = Renderer::GetInstance();
		mGodRayTarget[0]->Bind(SHADER_TYPE_PS, 0);
		auto provider = renderer.GetResourceProvider();
		provider->BindBlendState(ResourceTypes::BlendStates::AdditiveKeepAlpha);
		//provider->BindBlendState(ResourceTypes::BlendStates::Additive);
		provider->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthStencil, 0);
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::CopyPS), false);
		renderer.RestoreBlendState();
	}

	////---------------------------------------------------------------------------
	void MeasureLuminanceOfHDRTarget()
	{
		auto& renderer = Renderer::GetInstance();
		renderer.RestoreBlendState();
		auto provider = renderer.GetResourceProvider();
		provider->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthStencil, 0);

		RenderEventMarker mark("Luminance");
		assert(mHDRTarget);
		int dwCurTexture = provider->GetNumToneMaps() - 1;
		TexturePtr renderTarget = provider->GetTexture(ResourceTypes::Textures::ToneMap, dwCurTexture);
		TexturePtr rts[] = { renderTarget };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		mHDRTarget->Bind(SHADER_TYPE_PS, 0);
		bool msaa = renderer.GetMultiSampleCount() > 1;
		if (msaa)
		{
			Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pDest)
			{
				pDest->x = (float)mHDRTarget->GetWidth();
				pDest->y = (float)mHDRTarget->GetHeight();
				renderer.UnmapShaderConstantsBuffer();
			}
		}

		const Vec2I& resol = renderTarget->GetSize();
		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0, 1 };
		renderer.SetViewports(&vp, 1);
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::SampleLumInitialPS), false);
		--dwCurTexture;

		while (dwCurTexture > 0)
		{
			TexturePtr src = provider->GetTexture(ResourceTypes::Textures::ToneMap, dwCurTexture + 1);
			TexturePtr renderTarget = provider->GetTexture(ResourceTypes::Textures::ToneMap, dwCurTexture);

			TexturePtr rts[] = { renderTarget };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
			src->Bind(SHADER_TYPE_PS, 0);

			const Vec2I& resol = renderTarget->GetSize();
			Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0, 1 };
			renderer.SetViewports(&vp, 1);
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::SampleLumIterativePS), false);
			--dwCurTexture;
		}

		// Perform the final pass of the average luminance calculation.
	{
		TexturePtr src = provider->GetTexture(ResourceTypes::Textures::ToneMap, dwCurTexture + 1);
		TexturePtr renderTarget = provider->GetTexture(ResourceTypes::Textures::ToneMap, dwCurTexture);
		TexturePtr rts[] = { renderTarget };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		src->Bind(SHADER_TYPE_PS, 0);
		const Vec2I& resol = renderTarget->GetSize();
		{Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0, 1 };
		renderer.SetViewports(&vp, 1); }

		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::SampleLumFinalPS), false);
	}

	// AdaptedLum
	{
		provider->SwapTexture(ResourceTypes::Textures::LuminanceMap, 0, 1);
		auto luminanceMap0 = provider->GetTexture(ResourceTypes::Textures::LuminanceMap, 0);
		auto luminanceMap1 = provider->GetTexture(ResourceTypes::Textures::LuminanceMap, 1);
		TexturePtr rts[] = { luminanceMap0 };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		luminanceMap1->Bind(SHADER_TYPE_PS, 0);
		auto tonemap0 = provider->GetTexture(ResourceTypes::Textures::ToneMap, 0);
		tonemap0->Bind(SHADER_TYPE_PS, 1);
		Viewport vp = { 0, 0, (float)luminanceMap0->GetWidth(), (float)luminanceMap0->GetHeight(), 0, 1 };
		renderer.SetViewports(&vp, 1);
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::CalcAdaptedLumPS), false);
	}
	}

	//---------------------------------------------------------------------------
	void BrightPass()
	{
		auto& renderer = Renderer::GetInstance();
		auto rt = mRenderTarget.lock();
		const auto& size = mSize;
		if (!mBrightPassTexture)
		{
			Vec2I size(
				CropSize8((int)(size.x* 0.25f)),
				CropSize8((int)(size.y * 0.25f))
				);
			mBrightPassTexture = renderer.CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			mBrightPassTexture->SetDebugName(FormatString("rt%u_%u_%u_BrightPass", rt->GetId(), size.x, size.y).c_str());
		}

		const Vec2I& resol = mBrightPassTexture->GetSize();
		{
			RenderEventMarker mark("Bloom - BrightPass");
			// brightpass
			TexturePtr rts[] = { mBrightPassTexture };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
			auto provider = renderer.GetResourceProvider();
			TexturePtr rvs[] = { mHDRTarget, provider->GetTexture(ResourceTypes::Textures::LuminanceMap, 0) };
			renderer.SetTextures(rvs, 2, SHADER_TYPE_PS, 0);

			Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0, 1 };
			renderer.SetViewports(&vp, 1);

			if (renderer.GetMultiSampleCount() != 1)
			{
				Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
				if (pDest)
				{
					pDest->x = (float)mHDRTarget->GetWidth();
					pDest->y = (float)mHDRTarget->GetHeight();
					renderer.UnmapShaderConstantsBuffer();
				}
			}

			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::BrightPassPS), false);
		}
	}

	//---------------------------------------------------------------------------
	void BrightPassToStarSource()
	{
		auto& renderer = Renderer::GetInstance();
		const auto& size = mSize;
		if (!mStarSourceTex)
		{
			Vec2I starSourceSize(CropSize8((int)(size.x*0.25f)),
				CropSize8((int)(size.y*0.25f))
				);
			mStarSourceTex = renderer.CreateTexture(0, starSourceSize.x, starSourceSize.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		RenderEventMarker mark("BrightPassToStarSource");

		Vec4f* pOffsets = 0;
		Vec4f* pWeights = 0;
		renderer.GetSampleOffsets_GaussianBlur5x5(mGlowTarget->GetWidth(), mGlowTarget->GetHeight(),
			&pOffsets, &pWeights, 1.0f);
		assert(pOffsets && pWeights);

		BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
		memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4f) * 13);
		memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4f) * 13);
		renderer.UnmapBigBuffer();

		TexturePtr rts[] = { mStarSourceTex };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0);
		Viewport vp = { 0, 0, (float)mStarSourceTex->GetWidth(), (float)mStarSourceTex->GetHeight(), 0, 1 };
		renderer.SetViewports(&vp, 1);
		renderer.Clear(0, 0, 0, 1);
		mGlowTarget->Bind(SHADER_TYPE_PS, 0);
		if (renderer.GetMultiSampleCount() != 1)
		{
			Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pDest)
			{
				pDest->x = (float)mGlowTarget->GetWidth();
				pDest->y = (float)mGlowTarget->GetHeight();
				renderer.UnmapShaderConstantsBuffer();
			}
		}
		auto provider = renderer.GetResourceProvider();
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::Blur5x5PS), false);
	}

	//---------------------------------------------------------------------------
	void StarSourceToBloomSource()
	{
		RenderEventMarker mark("StarSourceToBloomSource");
		auto& renderer = Renderer::GetInstance();
		const auto& size = mSize;
		if (!mBloomSourceTex)
		{
			Vec2I bloomSourceSize(CropSize8(size.x / 8), CropSize8(size.y / 8));
			mBloomSourceTex = renderer.CreateTexture(0, bloomSourceSize.x, bloomSourceSize.y, PIXEL_FORMAT_R8G8B8A8_UNORM, 
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		auto provider = renderer.GetResourceProvider();
		auto blur5x5 = provider->GetShader(ResourceTypes::Shaders::Blur5x5PS);
		assert(blur5x5);
		Vec4f* pOffsets = 0;
		Vec4f* pWeights = 0;
		renderer.GetSampleOffsets_GaussianBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
			&pOffsets, &pWeights, 1.0f);
		assert(pOffsets && pWeights);

		BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
		memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4f) * 13);
		memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4f) * 13);
		renderer.UnmapBigBuffer();

		TexturePtr rts[] = { mBloomSourceTex };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0);

		Viewport vp = { 0, 0, (float)mBloomSourceTex->GetWidth(), (float)mBloomSourceTex->GetHeight(), 0, 1 };
		renderer.SetViewports(&vp, 1);

		mBrightPassTexture->Bind(SHADER_TYPE_PS, 0);

		if (renderer.GetMultiSampleCount() != 1)
		{
			Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pDest)
			{
				pDest->x = (float)mBrightPassTexture->GetWidth();
				pDest->y = (float)mBrightPassTexture->GetHeight();
				renderer.UnmapShaderConstantsBuffer();
			}
		}

		renderer.DrawFullscreenQuad(blur5x5, false);
	}

	//---------------------------------------------------------------------------
	void Bloom()
	{
		auto& renderer = Renderer::GetInstance();
		if (!mBloomTexture[0])
		{
			Vec2I size(CropSize8(mSize.x / 8),
				CropSize8(mSize.y / 8));
			for (int i = 0; i < FB_NUM_BLOOM_TEXTURES; i++)
			{
				mBloomTexture[i] = renderer.CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM,
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
				char buff[255];
				sprintf_s(buff, "rt%u_%u_%u_Bloom(%d)", mId, size.x, size.y, i);
				mBloomTexture[i]->SetDebugName(buff);
			}
		}

		// blur
		RenderEventMarker mark("Bloom - Blur");
		assert(mBloomSourceTex);
		TexturePtr rts[] = { mBloomTexture[2] };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0);
		Viewport vp = { 0, 0, (float)mBloomTexture[2]->GetWidth(), (float)mBloomTexture[2]->GetHeight(), 0, 1 };
		renderer.SetViewports(&vp, 1);

		mBloomSourceTex->Bind(SHADER_TYPE_PS, 0);

		Vec4f* pOffsets = 0;
		Vec4f* pWeights = 0;
		renderer.GetSampleOffsets_GaussianBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
			&pOffsets, &pWeights, 1.0f);
		assert(pOffsets && pWeights);

		BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
		memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4f) * 13);
		memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4f) * 13);
		renderer.UnmapBigBuffer();

		if (renderer.GetMultiSampleCount() != 1)
		{
			Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pDest)
			{
				pDest->x = (float)mBloomSourceTex->GetWidth();
				pDest->y = (float)mBloomSourceTex->GetHeight();
				renderer.UnmapShaderConstantsBuffer();
			}
		}
		auto provider = renderer.GetResourceProvider();
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::Blur5x5PS), false);

		const Vec2I& resol = mBloomTexture[2]->GetSize();
		// Horizontal Blur
		{
			RenderEventMarker mark("Bloom - Apply hori gaussian filter");
			BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
			Vec4f* avSampleOffsets = pData->gSampleOffsets;
			Vec4f* avSampleWeights = pData->gSampleWeights;
			if (!mGaussianDistBloom)
			{
				mGaussianDistBloom = GaussianDistPtr(FB_NEW(GaussianDist), [](GaussianDist* obj){ delete obj; });
				mGaussianDistBloom->Calc(resol.x, resol.y, 3, 1.25f);
			}
			memcpy(avSampleOffsets, mGaussianDistBloom->mGaussianDistOffsetX, sizeof(Vec4f) * 15);
			memcpy(avSampleWeights, mGaussianDistBloom->mGaussianDistWeightX, sizeof(Vec4f) * 15);
			renderer.UnmapBigBuffer();

			TexturePtr rts[] = { mBloomTexture[1] };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, 0, 0);
			mBloomTexture[2]->Bind(SHADER_TYPE_PS, 0);
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::BloomPS), false);
		}

		// Vertical Blur
	{
		RenderEventMarker mark("Bloom - Apply verti gaussian filter");
		BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
		Vec4f* avSampleOffsets = pData->gSampleOffsets;
		Vec4f* avSampleWeights = pData->gSampleWeights;
		memcpy(avSampleOffsets, mGaussianDistBloom->mGaussianDistOffsetY, sizeof(Vec4f) * 15);
		memcpy(avSampleWeights, mGaussianDistBloom->mGaussianDistWeightY, sizeof(Vec4f) * 15);
		renderer.UnmapBigBuffer();
		renderer.UnbindTexture(SHADER_TYPE_PS, 0);
		renderer.UnbindTexture(SHADER_TYPE_PS, 1);
		renderer.UnbindTexture(SHADER_TYPE_PS, 2);
		rts[0] = mBloomTexture[0];
		renderer.SetRenderTarget(rts, index, 1, 0, 0);
		mBloomTexture[1]->Bind(SHADER_TYPE_PS, 0);

		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::BloomPS), false);
	}
	}


	//static  function
	void ReleaseStarDef()
	{
		
	}
	void CalcStarGlareConst(Vec4 s_aaColor[starGlareMaxPasses][starGlareSamples])
	{
		static const Color s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f);
		for (int p = 0; p < starGlareMaxPasses; ++p)
		{
			Real ratio;
			ratio = (Real)(p + 1) / (Real)starGlareMaxPasses;

			for (int s = 0; s < starGlareSamples; s++)
			{
				Color chromaticAberrColor = Lerp(StarDef::GetChromaticAberrationColor(s), s_colorWhite, ratio);
				s_aaColor[p][s] = Lerp(s_colorWhite, chromaticAberrColor, starGlareChromaticAberration).GetVec4();
			}
		}
		StarDef::InitializeStatic();
	}

	//---------------------------------------------------------------------------
	void RenderStarGlare()
	{
		auto& renderer = Renderer::GetInstance();
		if (mStarTextures[0] == 0)
		{
			Vec2I size(CropSize8((int)(mSize.x * 0.25f)), CropSize8((int)(mSize.y*0.25f)));
			for (int i = 0; i < FB_NUM_STAR_TEXTURES; ++i)
			{
				mStarTextures[i] = renderer.CreateTexture(0, size.x, size.y, PIXEL_FORMAT_R16G16B16A16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			}
		}

		TexturePtr rts[] = { mStarTextures[0] };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, 0, 0);
		Viewport vp = { 0, 0, (float)mStarTextures[0]->GetWidth(), (float)mStarTextures[0]->GetHeight(), 0, 1 };
		renderer.SetViewports(&vp, 1);
		renderer.Clear(0, 0, 0, 1);

		auto rt = mRenderTarget.lock();
		const Real fTanFoV = rt->GetCamera()->GetFOV();
		const Color vWhite(1.0f, 1.0f, 1.0f, 1.0f);
		

		Vec4f avSampleWeights[FB_MAX_SAMPLES];
		Vec4f avSampleOffsets[FB_MAX_SAMPLES];

		//mAdditiveBlendState->Bind();

		Real srcW = (Real)mStarSourceTex->GetWidth();
		Real srcH = (Real)mStarSourceTex->GetHeight();


		Real radOffset;
		radOffset = starGlareInclination + mStarGlareDef->m_fInclination;

		TexturePtr pSrcTexture = 0;
		TexturePtr pDestTexture = 0;

		auto provider = renderer.GetResourceProvider();
		// Direction loop
		for (int d = 0; d < mStarGlareDef->m_nStarLines; ++d)
		{
			const STARLINE& starLine = mStarGlareDef->m_pStarLine[d];

			pSrcTexture = mStarSourceTex;

			Real rad = radOffset + starLine.fInclination;
			Real sn = std::sin(rad), cs = std::cos(rad);
			Vec2 vtStepUV(sn / srcW * starLine.fSampleLength,
				cs / srcH * starLine.fSampleLength);

			Real attnPowScale;
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
					Real lum;
					lum = std::pow(starLine.fAttenuation, attnPowScale * i);

					avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] *
						lum * (p + 1.0f) * 0.5f;

					// Offset of sampling coordinate
					avSampleOffsets[i].x = (float)vtStepUV.x * i;
					avSampleOffsets[i].y = (float)vtStepUV.y * i;
					if (fabs(avSampleOffsets[i].x) >= 0.9f ||
						fabs(avSampleOffsets[i].y) >= 0.9f)
					{
						avSampleOffsets[i].x = 0.0f;
						avSampleOffsets[i].y = 0.0f;
						avSampleWeights[i] *= 0.0f;
					}

				}

				BIG_BUFFER* pData = (BIG_BUFFER*)renderer.MapBigBuffer();
				memcpy(pData->gSampleOffsets, avSampleOffsets, sizeof(Vec4f)* FB_MAX_SAMPLES);
				memcpy(pData->gSampleWeights, avSampleWeights, sizeof(Vec4f)* FB_MAX_SAMPLES);
				renderer.UnmapBigBuffer();

				TexturePtr rts[] = { pDestTexture };
				size_t index[] = { 0 };
				renderer.SetRenderTarget(rts, index, 1, 0, 0);
				pSrcTexture->Bind(SHADER_TYPE_PS, 0);

				renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::StarGlarePS), false);

				// Setup next expansion
				vtStepUV *= (Real)starGlareSamples;
				attnPowScale *= (Real)starGlareSamples;

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

		std::vector<TexturePtr> textures;
		textures.reserve(mStarGlareDef->m_nStarLines);
		for (int i = 0; i < mStarGlareDef->m_nStarLines; i++)
		{
			textures.push_back(mStarTextures[i + 4]);
			avSampleWeights[i] = vWhite.GetVec4() * (1.0f / (Real)mStarGlareDef->m_nStarLines);
		}

		{
			TexturePtr rts[] = { pDestTexture };
			size_t index[] = { 0 };
			renderer.SetRenderTarget(rts, index, 1, 0, 0);
		}
		renderer.SetTextures(&textures[0], textures.size(), SHADER_TYPE_PS, 0);

		switch (mStarGlareDef->m_nStarLines)
		{
		case 2:
			renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::MergeTextures2PS), false);
			break;

		default:
			assert(0);
		}

		textures.clear();
		for (int i = 0; i < mStarGlareDef->m_nStarLines; i++)
		{
			textures.push_back(0);
		}
		renderer.SetTextures(&textures[0], mStarGlareDef->m_nStarLines, SHADER_TYPE_PS, 0);
	}

	//---------------------------------------------------------------------------
	void ToneMapping()
	{
		RenderEventMarker mark("ToneMapping");
		auto& renderer = Renderer::GetInstance();
		auto provider = renderer.GetResourceProvider();
		TexturePtr textures[] = { mHDRTarget, provider->GetTexture(ResourceTypes::Textures::LuminanceMap, 0), mBloomTexture[0], mStarTextures[0] };
		auto rt = mRenderTarget.lock();
		rt->BindTargetOnly(false);
		renderer.SetTextures(textures, 4, SHADER_TYPE_PS, 0);
		provider->BindBlendState(ResourceTypes::BlendStates::Default);		
		if (renderer.IsLuminanceOnCpu())
		{
			Vec4f* pData = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pData)
			{
				*pData = Vec4f(mLuminance, 0, 0, 0);
				renderer.UnmapShaderConstantsBuffer();
			}
		}

		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::ToneMappingPS), false);
		renderer.RestoreDepthStencilState();
		renderer.UnbindTexture(SHADER_TYPE_PS, 3);			
		renderer.UnbindTexture(SHADER_TYPE_PS, 0);		
	}	

	void SimpleToneMapping()
	{
		auto& renderer = Renderer::GetInstance();				
		auto rt = mRenderTarget.lock();
		rt->BindTargetOnly(false);
		mHDRTarget->Bind(SHADER_TYPE_PS, 0);		
		auto& provider = renderer.GetResourceProvider();
		provider->BindBlendState(ResourceTypes::BlendStates::Default);		
		renderer.DrawFullscreenQuad(provider->GetShader(ResourceTypes::Shaders::SimpleToneMappingPS), false);
		renderer.RestoreDepthStencilState();
		renderer.UnbindTexture(SHADER_TYPE_PS, 3);
		renderer.UnbindTexture(SHADER_TYPE_PS, 0);

	}

	void OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName){
	
	}

	void OnRenderTargetSizeChanged(const Vec2I& size){
		mSize = size;
		mDepthTarget.reset();
		mGlowTarget.reset();
		for (int i = 0; i < 2; ++i)
			mGlowTexture[i].reset();

		//mShadowMap.reset();
		mCloudVolumeDepth.reset();

		for (int i = 0; i < 2; ++i)
			mGodRayTarget[i].reset(); // half resolution; could be shared.
		mNoMSDepthStencil.reset();
		mHDRTarget.reset();
		mSmallSilouetteBuffer.reset();
		mBigSilouetteBuffer.reset();

		mBrightPassTexture.reset();
		mStarSourceTex.reset();
		mBloomSourceTex.reset();
		for (int i = 0; i < FB_NUM_BLOOM_TEXTURES; ++i)
			mBloomTexture[i].reset();
		for (int i = 0; i < FB_NUM_STAR_TEXTURES; ++i)
			mStarTextures[i].reset();

		//mLightCamera.reset();
		mGaussianDistBlendGlow.reset();
		mGaussianDistBloom.reset();
	}
};

RenderStrategyDefaultPtr RenderStrategyDefault::Create() {
	auto p = RenderStrategyDefaultPtr(new RenderStrategyDefault, [](RenderStrategyDefault* obj) {delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}

//---------------------------------------------------------------------------
RenderStrategyDefault::RenderStrategyDefault()
	: mImpl(new Impl){

}
RenderStrategyDefault::~RenderStrategyDefault(){
}

void RenderStrategyDefault::SetScene(IScenePtr scene){
	mImpl->SetScene(scene);
}

void RenderStrategyDefault::SetRenderTarget(RenderTargetPtr renderTarget){
	mImpl->SetRenderTarget(renderTarget);
}

void RenderStrategyDefault::Render(size_t face){
	mImpl->Render(face);
}

bool RenderStrategyDefault::IsHDR() const{
	return true;
}

bool RenderStrategyDefault::IsGlowSupported(){
	return true;
}

bool RenderStrategyDefault::SetHDRTarget(){
	mImpl->HDRTarget(true);
	return true;
}

bool RenderStrategyDefault::SetSmallSilouetteBuffer(){
	return mImpl->SetSmallSilouetteBuffer();
}

bool RenderStrategyDefault::SetBigSilouetteBuffer(){
	return mImpl->SetBigSilouetteBuffer();
}

void RenderStrategyDefault::GlowRenderTarget(bool bind){
	return mImpl->GlowTarget(bind);
}

void RenderStrategyDefault::DepthTexture(bool bind){
	return mImpl->DepthTexture(bind);
}

void RenderStrategyDefault::OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName){
	mImpl->OnRendererOptionChanged(options, optionName);
}

void RenderStrategyDefault::OnRenderTargetSizeChanged(const Vec2I& size){
	mImpl->OnRenderTargetSizeChanged(size);
}

void RenderStrategyDefault::OnAfterMakeVisibleSet(IScene* scene) {

}

void RenderStrategyDefault::OnBeforeRenderingOpaques(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {

}

void RenderStrategyDefault::OnBeforeRenderingOpaquesRenderStates(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	if (renderParam.mRenderPass == RENDER_PASS::PASS_NORMAL) {
		RenderStates::SetForceIncrementalStencilState(true);
	}
}

void RenderStrategyDefault::OnAfterRenderingOpaquesRenderStates(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	if (renderParam.mRenderPass == RENDER_PASS::PASS_NORMAL) {
		RenderStates::SetForceIncrementalStencilState(false);
	}
}
void RenderStrategyDefault::OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {

}