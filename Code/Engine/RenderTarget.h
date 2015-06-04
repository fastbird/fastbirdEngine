#pragma once
#include <Engine/IRenderTarget.h>
#include <Engine/ICamera.h>

#define FB_NUM_BLOOM_TEXTURES 3
#define FB_NUM_STAR_TEXTURES     12 

namespace fastbird
{
	class RenderPipeline;
	struct GaussianDist;
	class Scene;

	class RenderTarget : public IRenderTarget
	{
	protected:
		static RenderTargetId NextRenderTargetId;
		RenderTargetId mId;
		Vec2I mSize;
		Vec2I mSizeCropped;
		PIXEL_FORMAT mFormat;
		SmartPtr<RenderPipeline> mRenderPipeline;
		bool mSRV;
		bool mMiplevel;
		bool mCubeMap;
		bool mWillCreateDepth;
		SmartPtr<ITexture> mRenderTargetTexture;
		SmartPtr<ITexture> mDepthStencilTexture;
		SmartPtr<Scene> mScene;
		IScene* mSceneOverride;
		bool mLockSceneOverride;
		SmartPtr<ICamera> mCamera;
		SmartPtr<ICamera> mOverridingCam;		
		
		Color mClearColor;
		float mDepthClear;
		UINT8 mStencilClear;
		bool mEnabled;
		bool mUsePool;
		Viewport mViewport;
		unsigned mFace;

		// Additional Targets
		// Depth target
		SmartPtr<ITexture> mDepthTarget;

		// Glow
		SmartPtr<ITexture> mGlowTarget;
		SmartPtr<ITexture> mGlowTexture[2];
		bool mGlowSet;

		// Shadow
		SmartPtr<ICamera> mLightCamera;
		SmartPtr<ITexture> mShadowMap;

		// Cloud
		SmartPtr<ITexture> mCloudVolumeDepth;

		SmartPtr<ITexture> mGodRayTarget[2]; // half resolution; could be shared.
		SmartPtr<ITexture> mNoMSDepthStencil;
		SmartPtr<ITexture> mHDRTarget;

		SmartPtr<ITexture> mSmallSilouetteBuffer;
		SmartPtr<ITexture> mBigSilouetteBuffer;

		unsigned mFrameLuminanceCalced;
		float mLuminance;

		GaussianDist* mGaussianDistBlendGlow;
		GaussianDist* mGaussianDistBloom;

		SmartPtr<ITexture> mBrightPassTexture;
		SmartPtr<ITexture> mStarSourceTex;
		SmartPtr<ITexture> mBloomSourceTex;
		SmartPtr<ITexture> mBloomTexture[FB_NUM_BLOOM_TEXTURES];
		SmartPtr<ITexture> mStarTextures[FB_NUM_STAR_TEXTURES];

		SmartPtr<ITexture> mEnvTexture;

		bool mDrawOnEvent;
		bool mDrawEventTriggered;

	private:


	public:

		RenderTarget();
		virtual ~RenderTarget();

	protected:
		virtual void FinishSmartPtr();

	public:

		const Vec2I& GetSize() const;
		virtual bool CheckOptions(const RenderTargetParam& param);
		virtual RenderPipeline& GetRenderPipeline() const;

		virtual void SetScene(IScene* scene);
		virtual IScene* GetScene() const;
		Scene* GetSceneInternal() const;
		virtual void SetSceneOverride(IScene* scene);
		virtual IScene* GetSceneOverride() const { return mSceneOverride; }
		virtual void LockSceneOverride(bool lock) { mLockSceneOverride = lock; }

		virtual IScene* CreateScene();
		virtual ICamera* GetCamera() const;
		virtual ICamera* GetOrCreateOverridingCamera();
		virtual void RemoveOverridingCamera();

		virtual ITexture* GetRenderTargetTexture() { return mRenderTargetTexture; }
		virtual ITexture* GetDepthStencilTexture() { return mDepthStencilTexture; }
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil);
		virtual void SetClearColor(const Color& color);
		virtual void SetClearDepthStencil(float z, UINT8 stencil);

		virtual void Bind(size_t face = 0);
		virtual void BindTargetOnly(bool hdr);
		virtual bool Render(size_t face=0);
		virtual void Unbind();

		virtual void SetEnable(bool enable){ mEnabled = enable; }
		virtual bool GetEnable() const { return mEnabled; }
		virtual void SetEnvTexture(ITexture* texture);
		virtual void SetUsePool(bool usePool);
		virtual bool GetUsePool() const { return mUsePool; }

		virtual void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);

		void SetColorTexture(ITexture* pTexture);
		virtual ICamera* GetLightCamera() const { return mLightCamera; }

		//-------------------------------------------------------------------
		// Post processors
		//-------------------------------------------------------------------
		void SetDepthRenderTarget(bool clear);
		void UnsetDepthRenderTarget();
		virtual void BindDepthTexture(bool set);

		virtual void SetGlowRenderTarget();
		virtual void UnSetGlowRenderTarget();

		void PrepareShadowMapRendering();
		void EndShadowMapRendering();
		void BindShadowMap(bool bind);

		void SetCloudVolumeTarget();
		void SetCloudVolumeTexture(bool set);

		void SetGodRayRenderTarget();
		void GodRay();
		
		virtual void SetHDRTarget();
		void DrawSilouette();
		
		virtual void SetSmallSilouetteBuffer();
		virtual void SetBigSilouetteBuffer();

		void CalcLuminance();

		void BlendGlow();
		void BlendGodRay();
		void MeasureLuminanceOfHDRTargetNew();
		void BrightPass();
		void BrightPassToStarSource();
		void StarSourceToBloomSource();
		void Bloom();
		void RenderStarGlare();
		void ToneMapping();

		// internal access only
		ITexture* GetShadowMap() const { return mShadowMap; }
		void DeleteShadowMap();

		void SetLightCamWidth(float width);
		void SetLightCamHeight(float height);
		void SetLightCamNear(float n);
		void SetLightCamFar(float f);
		void UpdateLightCamera();

		static void ReleaseStarDef();

		virtual void DrawOnEvent(bool set);
		virtual void TriggerDrawEvent();
	};
}