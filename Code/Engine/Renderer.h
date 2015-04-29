#pragma once
#include <Engine/IRenderer.h>
#include <Engine/IConsole.h>
#include <Engine/StarDef.h>

namespace fastbird
{
struct POINT_LIGHT_CONSTANTS;
class ILight;
class DebugHud;
class IFont;
class RenderToTexture;
class CloudManager;
class GeometryRenderer;
class PointLightMan;
#define FB_NUM_TONEMAP_TEXTURES  4
#define FB_NUM_TONEMAP_TEXTURES_NEW 5
#define FB_NUM_LUMINANCE_TEXTURES 3
#define FB_NUM_BLOOM_TEXTURES 3
#define FB_NUM_STAR_TEXTURES     12 
#define FB_MAX_SAMPLES           16      // Maximum number of texture grabs
class Renderer : public IRenderer, public ICVarListener
{
protected:
	unsigned				mWidth;
	unsigned				mHeight;
	Vec2I mCurRTSize;

	unsigned mCropWidth;
	unsigned mCropHeight;

	SmartPtr<ILight>		mDirectionalLight[2];
	SmartPtr<ILight>		mDirectionalLightOverride[2];
	SmartPtr<DebugHud>		mDebugHud;
	SmartPtr<GeometryRenderer> mGeomRenderer;
	SmartPtr<IFont> mFont;
	SmartPtr<IMaterial> mMaterials[DEFAULT_MATERIALS::COUNT];
	SmartPtr<IMaterial> mMissingMaterial;
	SmartPtr<IRasterizerState> mDefaultRasterizerState;
	SmartPtr<IRasterizerState> mFrontFaceCullRS;
	SmartPtr<IBlendState> mDefaultBlendState;
	SmartPtr<IBlendState> mAdditiveBlendState;
	SmartPtr<IBlendState> mAlphaBlendState;
	SmartPtr<IBlendState> mMaxBlendState;

	SmartPtr<IDepthStencilState> mDefaultDepthStencilState;
	SmartPtr<IDepthStencilState> mNoDepthStencilState;
	SmartPtr<IDepthStencilState> mNoDepthWriteLessEqualState;
	SmartPtr<IDepthStencilState> mLessEqualDepthState;
	bool mLockDepthStencil;

	// just temporal holder
	// dont need to delete. it will be deleted in the inherited class.
	ISamplerState* mDefaultSamplers[SAMPLERS::NUM];

	SmartPtr<ITexture> mEnvironmentTexture;
	SmartPtr<ITexture> mEnvironmentTextureOverride;
	std::vector< IRenderToTexture* > mRenderToTextures;

	std::vector<IRenderToTexture*> mRenderToTexturePool;

	Color mClearColor;
	float mDepthClear;
	UINT8 mStencilClear;
	bool					mForcedWireframe;
	bool					mDepthStencilCreated;

	ICamera*				mCamera;
	RENDERER_FRAME_PROFILER		mFrameProfiler;
	IShader* mBindedShader;
	IInputLayout* mBindedInputLayout;
	PRIMITIVE_TOPOLOGY mCurrentTopology;

	typedef std::map<INPUT_ELEMENT_DESCS, SmartPtr<IInputLayout> > INPUTLAYOUT_MAP;
	INPUTLAYOUT_MAP mInputLayouts;


	INPUT_ELEMENT_DESCS mInputLayoutDescs[DEFAULT_INPUTS::COUNT];
	const int DEFAULT_DYN_VERTEX_COUNTS;
	SmartPtr<IVertexBuffer> mDynVBs[DEFAULT_INPUTS::COUNT];

	typedef VectorMap< std::string, SmartPtr<ITexture> > TextureCache;
	TextureCache mTextureCache;

	typedef std::vector< SmartPtr<TextureAtlas> > TextureAtlasCache;
	TextureAtlasCache mTextureAtalsCache;

	typedef VectorMap< std::string, SmartPtr<IMaterial> > MaterialCache;
	MaterialCache mMaterialCache;
	SmartPtr<IShader> mFullscreenQuadVSNear;
	SmartPtr<IShader> mFullscreenQuadVSFar;

	SmartPtr<ITexture> mTempDepthBufferHalf; // for depth writing and clouds
	SmartPtr<ITexture> mTempDepthBuffer;

	// linear sampler
	SmartPtr<IShader> mCopyPS;
	SmartPtr<IShader> mCopyPSMS;

	// DepthPass Resources
	SmartPtr<ITexture> mDepthTarget;
	SmartPtr<IShader> mDepthWriteShader;
	SmartPtr<IShader> mCloudDepthWriteShader;
	SmartPtr<IBlendState> mMinBlendState;

	// HDR resources.
	SmartPtr<ITexture> mHDRTarget;
	SmartPtr<ITexture> mToneMap[FB_NUM_TONEMAP_TEXTURES_NEW];
	SmartPtr<ITexture> mLuminanceMap[FB_NUM_LUMINANCE_TEXTURES];
	int mLuminanceIndex;
	SmartPtr<IShader> mDownScale2x2LumPS;
	SmartPtr<IShader> mSampleLumInitialShader;
	SmartPtr<IShader> mSampleLumIterativeShader;
	SmartPtr<IShader> mSampleLumFinalShader;
	SmartPtr<IShader> mCalcAdaptedLumShader;
	SmartPtr<IShader> mDownScale3x3PS;
	SmartPtr<IShader> mToneMappingPS;
	SmartPtr<IShader> mLuminanceAvgPS;
	SmartPtr<ITexture> mBrightPassTexture;
	SmartPtr<ITexture> mBloomSourceTex;
	SmartPtr<ITexture> mBloomTexture[FB_NUM_BLOOM_TEXTURES];
	SmartPtr<IShader> mBrightPassPS;
	SmartPtr<IShader> mBloomPS;

	// 1/4

	// x, y,    offset, weight;
	VectorMap< std::pair<DWORD, DWORD>, std::pair<std::vector<Vec4>, std::vector<Vec4> > > mGauss5x5;

	Vec4 mGaussianOffsetsDownScale2x2[4];
	bool mGaussianDownScale2x2Calculated;

	// Star
	StarDef mStarDef;
	SmartPtr<ITexture> mStarSourceTex;
	SmartPtr<ITexture> mStarTextures[FB_NUM_STAR_TEXTURES];
	SmartPtr<IShader> mBlur5x5;
	SmartPtr<IShader> mStarGlareShader;
	SmartPtr<IShader> mMergeTexture2;
	float m_fChromaticAberration;
	float m_fStarInclination;

	// GodRay resources.
	SmartPtr<ITexture> mGodRayTarget[2]; // half resolution; could be shared.
	SmartPtr<IShader> mOccPrePassShader;
	SmartPtr<IShader> mOccPrePassGSShader;
	SmartPtr<IShader> mGodRayPS;
	SmartPtr<ITexture> mNoMSDepthStencil;
	SmartPtr<IInputLayout> mPositionInputLayout;
	// for bloom
	bool mGaussianDistCalculated; // should recalculate when the screen resolution changed
	Vec4 mGaussianDistOffsetX[15];
	Vec4 mGaussianDistWeightX[15];
	Vec4 mGaussianDistOffsetY[15];
	Vec4 mGaussianDistWeightY[15];

	// Glow
	SmartPtr<ITexture> mGlowTarget;
	SmartPtr<ITexture> mGlowTexture[2];
	SmartPtr<IShader> mGlowPS;
	bool mGlowSet;

	// for glow
	bool mGaussianDistGlowCalculated; // should recalculate when the screen resolution changed
	Vec4 mGaussianDistGlowOffsetX[15];
	Vec4 mGaussianDistGlowWeightX[15];
	Vec4 mGaussianDistGlowOffsetY[15];
	Vec4 mGaussianDistGlowWeightY[15];

	// for Cloud Volumes;
	SmartPtr<ITexture> mCloudVolumeDepth;
	SmartPtr<IBlendState> mRedAlphaMaskBlend;

	SmartPtr<IBlendState> mGreenAlphaMaskBlend;
	SmartPtr<IBlendState> mGreenAlphaMaskMaxBlend;

	SmartPtr<IBlendState> mRedAlphaMaskAddMinusBlend;
	SmartPtr<IBlendState> mGreenAlphaMaskAddAddBlend;

	SmartPtr<IBlendState> mRedAlphaMaskAddAddBlend;
	SmartPtr<IBlendState> mGreenAlphaMaskAddMinusBlend;


	SmartPtr<IBlendState> mGreenMaskBlend;
	SmartPtr<IBlendState> mBlueMaskBlend;
	SmartPtr<IRasterizerState> mRSCloudFar;
	SmartPtr<IRasterizerState> mRSCloudNear;
	bool mCloudRendering;

	SmartPtr<ITexture> mNoiseMap;

	ISkySphere* mNextEnvUpdateSkySphere;

	CloudManager* mCloudManager;
	bool mLockBlendState;

	// shadow
	SmartPtr<ITexture> mShadowMap;
	SmartPtr<IShader> mShadowMapShader;
	ICamera* mCameraBackup;

	SmartPtr<ITexture> mSmallSilouetteBuffer;
	SmartPtr<ITexture> mBigSilouetteBuffer;
	SmartPtr<IShader> mSilouetteShader;

	float mMiddleGray;
	float mStarPower;
	float mBloomPower;

	struct DebugRenderTarget
	{
		Vec2 mPos;
		Vec2 mSize;

		SmartPtr<ITexture> mTexture;
	};
	static const unsigned MaxDebugRenderTargets = 4;
	DebugRenderTarget mDebugRenderTargets[MaxDebugRenderTargets];

	PointLightMan* mPointLightMan;
	bool mRefreshPointLight;
	bool mLuminanceOnCpu;
	bool mUseFilmicToneMapping;
	float mLuminance;
	unsigned mFrameLuminanceCalced;
	float mFadeAlpha;

public:
	Renderer();
	virtual ~Renderer();
	virtual void Deinit();
	void CleanDepthWriteResources();
	void CleanGlowResources();
	void CleanHDRResources();
	void CleanGodRayResources();

	bool OnPrepared();
	
	virtual void ProcessRenderToTexture();

	virtual Vec2I ToSreenPos(const Vec3& ndcPos) const;
	virtual Vec2 ToNdcPos(const Vec2I& screenPos) const;
	virtual unsigned GetWidth() const { return mWidth; }
	virtual unsigned GetHeight() const { return mHeight; }
	virtual unsigned GetCropWidth() const { return mCropWidth; }
	virtual unsigned GetCropHeight() const { return mCropHeight; }
	//virtual void SetWireframe(bool enable); // see RendererD3D11
	virtual bool GetWireframe() const { return mForcedWireframe; }
	virtual void SetClearColor(float r, float g, float b, float a=1.f);
	virtual void SetClearDepthStencil(float z, UINT8 stencil);
	virtual void SetCamera(ICamera* pCamera);
	virtual ICamera* GetCamera() const;
	virtual void InitFrameProfiler(float dt);
	virtual const RENDERER_FRAME_PROFILER& GetFrameProfiler() const;
	virtual void DrawText(const Vec2I& pos, WCHAR* text, const Color& color, float size = 24);
	virtual void DrawText(const Vec2I& pos, const char* text, const Color& color, float size = 24);
	virtual void Draw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, float size = 24);
	virtual void Draw3DText(const Vec3& worldpos, const char* text, const Color& color, float size = 24);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color, float size = 24);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
		const Color& color, float size = 24);
	// without depth culling
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1);
	virtual void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
		const Color& color0, const Color& color1);
	virtual void DrawLine(const Vec2I& start, const Vec2I& end, 
		const Color& color0, const Color& color1);
	// with depth culling
	virtual void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
		const char* texture, bool textureFlow);
	virtual void DrawSphere(const Vec3& pos, float radius, const Color& color);
	virtual void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha);
	virtual void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha);
	virtual void RenderDebugHud(); 
	virtual void RenderGeoms();
	virtual inline IFont* GetFont() const;

	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num,
		ITexture* pDepthStencil, size_t dsIndex);
	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num);
	virtual const Vec2I& GetRenderTargetSize() const;
	virtual void RestoreRenderTarget();
	
	virtual const INPUT_ELEMENT_DESCS& GetInputElementDesc(
		DEFAULT_INPUTS::Enum e);

	// use this if you are sure there is instance of the descs.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs);

	// use these if you are not sure.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IMaterial* material) = 0;
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IShader* shader) = 0;
	// auxiliary
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IMaterial* material);
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IShader* shader);

	virtual TextureAtlas* GetTextureAtlas(const char* path);
	virtual TextureAtlasRegion* GetTextureAtlasRegion(const char* path, const char* region);
	bool ReloadTextureAtlas(const char* path);

	virtual IMaterial* CreateMaterial(const char* file);
	virtual IMaterial* GetMissingMaterial();

	virtual void SetDirectionalLight(ILight* pLight, int idx);
	virtual ILight* GetDirectionalLight(int idx) const;

	virtual void SetEnvironmentTexture(ITexture* pTexture);
	virtual void SetEnvironmentTextureOverride(ITexture* texture);

	// Render to Texture Pool handling
	virtual IRenderToTexture* CreateRenderToTexture(const RenderToTextureParam& param);
	virtual void DeleteRenderToTexture(IRenderToTexture*);


	//-------------------------------------------------------------------------
	// Render States
	virtual void RestoreRasterizerState();
	virtual void RestoreBlendState();
	virtual void RestoreDepthStencilState();
	virtual void LockDepthStencilState();
	virtual void UnlockDepthStencilState();

	// blend
	virtual void SetAlphaBlendState();
	virtual void SetAdditiveBlendState();
	virtual void SetMaxBlendState();
	virtual void SetRedAlphaMask();	
	virtual void SetGreenAlphaMask();
	virtual void SetGreenAlphaMaskMax();
	virtual void SetGreenAlphaMaskAddAddBlend();
	virtual void SetRedAlphaMaskAddMinusBlend();
	virtual void SetGreenAlphaMaskAddMinusBlend();
	virtual void SetRedAlphaMaskAddAddBlend();
	virtual void SetGreenMask();
	virtual void SetBlueMask();


	void LockBlendState();
	void UnlockBlendState();

	// depth
	virtual void SetNoDepthWriteLessEqual();
	virtual void SetLessEqualDepth();
	virtual void SetNoDepthStencil();

	// raster
	virtual void SetFrontFaceCullRS();

	// sampler
	virtual void SetSamplerState(SAMPLERS::Enum s, BINDING_SHADER shader, int slot);

	virtual void SetDepthWriteShader();
	
	

	virtual void SetPositionInputLayout();
	virtual void SetOccPreShader();
	virtual void SetOccPreGSShader();

	virtual void InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds);
	virtual void CleanCloud();
	void UpdateCloud(float dt);

	// ICVarListener
	virtual bool OnChangeCVar(CVar* pCVar);
	virtual void UpdateEnvMapInNextFrame(ISkySphere* sky);
	
	void Update(float dt);
	void UpdateLights(float dt);

	virtual void SetHDRTarget();
	void MeasureLuminanceOfHDRTarget();
	void MeasureLuminanceOfHDRTargetNew();
	void BrightPass();
	void BrightPassToStarSource();
	void StarSourceToBloomSource();
	void Bloom();
	void RenderStarGlare();
	void ToneMapping();
	bool GetSampleOffsets_Bloom(DWORD dwTexSize,
		float afTexCoordOffset[15],
		Vec4* avColorWeight,
		float fDeviation, float fMultiplier);
	void GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight, Vec4** avTexCoordOffset, Vec4** avSampleWeight, float fMultiplier);
	void GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4* avTexCoordOffset);

	ITexture* FindRenderTarget(const Vec2I& size);

	void SetDepthRenderTarget(bool clear);
	void UnsetDepthRenderTarget();

	void SetGodRayRenderTarget();
	void GodRay();
	void BlendGodRay();
	
	virtual void SetGlowRenderTarget();
	void BlendGlow();

	virtual void BindDepthTexture(bool set);
	void SetCloudVolumeTarget();
	void SetCloudVolumeTexture(bool set);
	virtual void SetCloudRendering(bool rendering);

	void BindNoiseMap();

	// shadow
	void PrepareShadowMapRendering();
	void EndShadowMapRendering();
	void BindShadowMap(bool bind);
	virtual void SetShadowMapShader();

	virtual void SetSilouetteShader();
	virtual void SetSamllSilouetteBuffer();
	virtual void SetBigSilouetteBuffer();
	virtual void DrawSilouette();

	void RenderDebugRenderTargets();
	virtual void SetDebugRenderTarget(unsigned idx, const char* textureName);

	virtual void GatherPointLightData(BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst);
	virtual void RefreshPointLight();
	virtual bool NeedToRefreshPointLight() const { return mRefreshPointLight; }

	virtual IPointLight* CreatePointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime,
		bool manualDeletion);
	virtual void DeletePointLight(IPointLight* pointLight);

	void CalcLuminance();
	void ToneMapLuminanceOnCpu(bool oncpu);
	bool IsLuminanceOnCpu() const { return mLuminanceOnCpu; }
	void UseFilmicToneMapping(bool filmic);
	void CreateToneMappingShader();

	virtual void SetFadeAlpha(float alpha);
	void RenderFade();

	PointLightMan* GetPointLightMan() const { return mPointLightMan; }
	virtual IMaterial* GetMaterial(DEFAULT_MATERIALS::Enum type);

};

inline bool operator < (const INPUT_ELEMENT_DESCS& left, const INPUT_ELEMENT_DESCS& right)
{
	if (left.size() < right.size())
		return true;
	else if (left.size() == right.size())
	{
		DWORD size = left.size();

		int cmp = memcmp(&left[0], &right[0], sizeof(INPUT_ELEMENT_DESC)*size);
		if (cmp < 0)
			return true;
	}
	return false;
}

}