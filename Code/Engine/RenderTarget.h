#pragma once
#include <Engine/IRenderTarget.h>
#include <Engine/ICamera.h>

#define FB_NUM_BLOOM_TEXTURES 3
#define FB_NUM_STAR_TEXTURES     12 

namespace fastbird
{
	class RenderPipeline;
	struct GaussianDist;

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
		bool mHasDepth;
		SmartPtr<ITexture> mRenderTargetTexture;
		SmartPtr<ITexture> mDepthStencilTexture;
		SmartPtr<IScene> mScene;
		SmartPtr<ICamera> mCamera;
		SmartPtr<ICamera> mOverridingCam;
		SmartPtr<ICamera> mLightCamera;
		SmartPtr<ILight> mLight[2];
		SmartPtr<ITexture> mEnvTexture;
		Color mClearColor;
		float mDepthClear;
		UINT8 mStencilClear;
		bool mEnabled;
		bool mUsePool;
		Viewport mViewport;
		unsigned mFace = 0;

		// Additional Targets
		// Depth target
		SmartPtr<ITexture> mDepthTarget;

		// Glow
		SmartPtr<ITexture> mGlowTarget;
		SmartPtr<ITexture> mGlowTexture[2];
		bool mGlowSet;

		// Shadow
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

	private:

		void UpdateLightCamera();


	public:

		RenderTarget();
		virtual ~RenderTarget();
		
		const Vec2I& GetSize() const;
		virtual bool CheckOptions(const RenderTargetParam& param);
		virtual void SetRenderPipeline(RenderPipeline* pipeline);

		virtual void SetScene(IScene* scene);
		virtual IScene* GetScene() const{ return mScene; }
		virtual IScene* CreateScene();
		virtual ICamera* GetCamera() const;
		virtual ICamera* GetOrCreateOverridingCamera();
		virtual void RemoveOverridingCamera();
		virtual ILight* GetLight(int idx);

		virtual ITexture* GetRenderTargetTexture() { return mRenderTargetTexture; }
		virtual ITexture* GetDepthStencilTexture() { return mDepthStencilTexture; }
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil);

		virtual void Bind(size_t face = 0);
		virtual void BindTargetOnly();
		virtual void Render(size_t face=0);
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


	};
}