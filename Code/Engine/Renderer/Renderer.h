#pragma once
#include <Engine/IRenderer.h>
#include <Engine/IConsole.h>

namespace fastbird
{

class ILight;
class DebugHud;
class IFont;
class RenderToTexture;
class CloudManager;
#define FB_NUM_TONEMAP_TEXTURES  5 
#define FB_NUM_LUMINANCE_TEXTURES 3
#define FB_NUM_BLOOM_TEXTURES 2
class Renderer : public IRenderer, public ICVarListener
{
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
	//virtual void SetWireframe(bool enable); // see RendererD3D11
	virtual bool GetWireframe() const { return mForcedWireframe; }
	virtual void SetClearColor(float r, float g, float b, float a=1.f);
	virtual void SetClearDepthStencil(float z, UINT8 stencil);
	virtual void SetCamera(ICamera* pCamera);
	virtual ICamera* GetCamera() const;
	virtual void InitFrameProfiler(float dt);
	virtual const RENDERER_FRAME_PROFILER& GetFrameProfiler() const;
	virtual void DrawText(const Vec2I& pos, WCHAR* text, const Color& color);
	virtual void DrawText(const Vec2I& pos, const char* text, const Color& color);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
		const Color& color);
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1);
	virtual void DrawLine(const Vec2I& start, const Vec2I& end, 
		const Color& color0, const Color& color1);
	virtual void RenderDebugHud(); 
	virtual inline IFont* GetFont() const;
	
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

	virtual IMaterial* CreateMaterial(const char* file);
	virtual IMaterial* GetMissingMaterial();

	virtual void SetDirectionalLight(ILight* pLight);
	virtual ILight* GetDirectionalLight() const;

	virtual int BindFullscreenQuadUV_VB(bool farSide);

	virtual void SetEnvironmentTexture(ITexture* pTexture);


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

	// raster
	virtual void SetFrontFaceCullRS();

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

	void SetHDRTarget();
	void MeasureLuminanceOfHDRTarget();
	void Bloom();
	void ToneMapping();
	bool GetSampleOffsets_Bloom(DWORD dwTexSize,
		float afTexCoordOffset[15],
		Vec4* avColorWeight,
		float fDeviation, float fMultiplier);

	ITexture* FindRenderTarget(const Vec2I& size);

	void SetDepthRenderTarget(bool clear);
	void UnsetDepthRenderTarget();

	void SetGodRayRenderTarget();
	void GodRay();
	void BlendGodRay();
	
	virtual void SetGlowRenderTarget();
	void BlendGlow();

	void SetDepthTexture(bool set);
	void SetCloudVolumeTarget();
	void SetCloudVolumeTexture(bool set);
	virtual void SetCloudRendering(bool rendering);

	void BindNoiseMap();



protected:
	unsigned				mWidth;
	unsigned				mHeight;

	SmartPtr<ILight>		mDirectionalLight;
	SmartPtr<ILight>		mDirectionalLightOverride;
	SmartPtr<IVertexBuffer>	mVBQuadUV_Far;
	SmartPtr<IVertexBuffer>	mVBQuadUV_Near;
	SmartPtr<DebugHud>		mDebugHud;
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
	SmartPtr<ITexture> mEnvironmentTexture;
	std::vector< SmartPtr<RenderToTexture> > mRenderToTextures;

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
	SmartPtr<IInputLayout> mQuadInputLayout;
	SmartPtr<IShader> mFullscreenQuadVS;
	SmartPtr<IShader> mCopyPS;
	SmartPtr<IShader> mCopyPSMS;

	// DepthPass Resources
	SmartPtr<ITexture> mDepthTarget;
	SmartPtr<ITexture> mTempDepthBuffer; // for depth writing and clouds
	SmartPtr<IShader> mDepthWriteShader;
	SmartPtr<IShader> mCloudDepthWriteShader;
	SmartPtr<IBlendState> mMinBlendState;

	// HDR resources.
	SmartPtr<ITexture> mHDRTarget;
	SmartPtr<ITexture> mToneMap[FB_NUM_TONEMAP_TEXTURES];
	SmartPtr<ITexture> mLuminanceMap[FB_NUM_LUMINANCE_TEXTURES];
	int mLuminanceIndex;	
	SmartPtr<IShader> mDownScale2x2PS;
	SmartPtr<IShader> mDownScale3x3PS;
	SmartPtr<IShader> mToneMappingPS;
	SmartPtr<IShader> mLuminanceAvgPS;	
	SmartPtr<ITexture> mBrightPassTexture;
	SmartPtr<ITexture> mBloomTexture[FB_NUM_BLOOM_TEXTURES];
	SmartPtr<IShader> mBrightPassPS;
	SmartPtr<IShader> mBloomPS;
	
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
	
};

inline bool operator < (const INPUT_ELEMENT_DESCS& left, const INPUT_ELEMENT_DESCS& right)
{
	if (left.size() < right.size())
		return true;
	else if (left.size() == right.size())
	{
		DWORD size = left.size();
		for (DWORD i=0; i<size; i++)
		{
			if (left[i].mInputSlot < right[i].mInputSlot)
				return true;
			else if (left[i].mInputSlot == right[i].mInputSlot)
			{
				if (left[i] < right[i])
					return true;
			}
		}
	}
	
	return false;
}

}