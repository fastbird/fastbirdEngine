#pragma once
#include <Engine/IRenderer.h>
#include <Engine/IConsole.h>
#include <Engine/UI3DObj.h>
#include <Engine/StarDef.h>

namespace fastbird
{
struct POINT_LIGHT_CONSTANTS;
class ILight;
class DebugHud;
class IFont;
class RenderTarget;
class CloudManager;
class GeometryRenderer;
class PointLightMan;
class IShader;
class IScene;
#define FB_NUM_TONEMAP_TEXTURES  4
#define FB_NUM_TONEMAP_TEXTURES_NEW 5
#define FB_NUM_LUMINANCE_TEXTURES 3
#define FB_MAX_SAMPLES           16      // Maximum number of texture grabs
class Renderer : public IRenderer, public ICVarListener
{
protected:
	VectorMap<HWND_ID, unsigned> mWidth;
	VectorMap<HWND_ID, unsigned> mHeight;
	VectorMap<HWND_ID, SmartPtr<RenderTarget>> mSwapChainRenderTargets;
	IRenderTarget* mCurRenderTarget;
	std::vector<IRenderTarget*> mRenderTargetPool;

	// every frame render targets
	std::vector< IRenderTarget* > mRenderTargets;
	IScene* mCurProcessingScene;

	Vec2I mCurRTSize;

	SmartPtr<ILight>		mDirectionalLight[2];
	SmartPtr<DebugHud>		mDebugHud;
	SmartPtr<GeometryRenderer> mGeomRenderer;
	SmartPtr<IFont> mFont;
	SmartPtr<IMaterial> mMaterials[DEFAULT_MATERIALS::COUNT];
	SmartPtr<IMaterial> mMissingMaterial;
	SmartPtr<IRasterizerState> mDefaultRasterizerState;
	SmartPtr<IRasterizerState> mFrontFaceCullRS;
	SmartPtr<IRasterizerState> mOneBiasedDepthRS;
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

	bool					mForcedWireframe;

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

	VectorMap<Vec2I, SmartPtr<ITexture>> mTempDepthBuffers;

	// linear sampler
	SmartPtr<IShader> mCopyPS;
	SmartPtr<IShader> mCopyPSMS;

	// DepthPass Resources
	SmartPtr<ITexture> mDepthTarget;
	SmartPtr<IShader> mDepthWriteShader;
	SmartPtr<IShader> mCloudDepthWriteShader;
	SmartPtr<IShader> mDepthOnlyShader;
	SmartPtr<IBlendState> mMinBlendState;

	// HDR resources.
	
	SmartPtr<ITexture> mToneMap[FB_NUM_TONEMAP_TEXTURES_NEW];
	SmartPtr<ITexture> mLuminanceMap[FB_NUM_LUMINANCE_TEXTURES];

	SmartPtr<IShader> mSampleLumInitialShader;
	SmartPtr<IShader> mSampleLumIterativeShader;
	SmartPtr<IShader> mSampleLumFinalShader;
	SmartPtr<IShader> mCalcAdaptedLumShader;
	SmartPtr<IShader> mToneMappingPS;
	SmartPtr<IShader> mBrightPassPS;
	SmartPtr<IShader> mBloomPS;

	// 1/4
	// x, y,    offset, weight;
	VectorMap< std::pair<DWORD, DWORD>, std::pair<std::vector<Vec4>, std::vector<Vec4> > > mGauss5x5;

	// Star
	SmartPtr<IShader> mBlur5x5;
	SmartPtr<IShader> mStarGlareShader;
	SmartPtr<IShader> mMergeTexture2;

	// GodRay resources.
	SmartPtr<IShader> mOccPrePassShader;
	SmartPtr<IShader> mOccPrePassGSShader;
	SmartPtr<IShader> mGodRayPS;
	SmartPtr<IInputLayout> mPositionInputLayout;
	SmartPtr<IShader> mGlowPS;


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
	SmartPtr<IBlendState> mNoColorWriteBlend;
	SmartPtr<IRasterizerState> mRSCloudFar;
	SmartPtr<IRasterizerState> mRSCloudNear;

	SmartPtr<ITexture> mNoiseMap;

	ISkySphere* mNextEnvUpdateSkySphere;

	CloudManager* mCloudManager;
	bool mLockBlendState;

	// shadow
	SmartPtr<IShader> mShadowMapShader;
	ICamera* mCameraBackup;

	
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

	typedef VectorMap<HWND_ID, std::vector<IUIObject*> > UI_OBJECTS;
	UI_OBJECTS mUIObjectsToRender;

	typedef VectorMap< std::pair<HWND_ID, std::string>, std::vector<IUIObject*>> UI_3DOBJECTS;
	UI_3DOBJECTS mUI3DObjects;
	VectorMap<std::string, IRenderTarget*> mUI3DObjectsRTs;
	VectorMap<std::string, UI3DObj*> mUI3DRenderObjs;

	bool m3DUIEnabled;

	// todo: generalize. layer 1~4.
	std::vector<IObject*> mMarkObjects;
	std::vector<IObject*> mHPBarObjects;

	std::vector<IRenderListener*> mRenderListeners;

	std::vector<IVideoPlayer*> mVideoPlayers;

	SmartPtr<IShader> mGGXGenShader;
	SmartPtr<ITexture> mGGXGenTarget;

	struct OutputInfo
	{
		WCHAR mDeviceName[32];
		RECT mRect;
	};
	std::vector<OutputInfo> mOutputInfos;
	bool mTakeScreenShot;

public:
	Renderer();
	virtual ~Renderer();

protected:
	virtual void FinishSmartPtr();

public:
	virtual bool Init(int threadPool);
	virtual void Deinit();
	virtual bool InitSwapChain(HWND_ID id, int width, int height) = 0;
	virtual void ReleaseSwapChain(HWND_ID id) = 0;
	// for windowed
	virtual void ChangeWindowSize(HWND_ID id, const Vec2I& resol);
	// for full-screen
	virtual void ChangeResolution(HWND_ID id, const Vec2I& resol) = 0;
	virtual void OnSizeChanged(HWND_ID id, const Vec2I& resol) = 0;

	void CleanDepthWriteResources();
	void CleanGlowResources();
	void CleanHDRResources();
	void CleanGodRayResources();

	IRenderTarget* GetMainRenderTarget() const;
	virtual IRenderTarget* GetRenderTarget(HWND_ID id) const;
	virtual IScene* GetMainScene() const;
	virtual IScene* GetScene() const;
	const Vec2I& GetMainRTSize() const;

	bool OnPrepared();
	void OnSwapchainCreated(HWND_ID id);
	
	virtual void ProcessRenderTarget();

	virtual Vec2I ToSreenPos(HWND_ID id, const Vec3& ndcPos) const;
	virtual Vec2 ToNdcPos(HWND_ID id, const Vec2I& screenPos) const;
	/*virtual unsigned GetWidth(HWND_ID id) const;
	virtual unsigned GetHeight(HWND_ID id) const;
	virtual unsigned GetWidth(HWND hWnd) const;
	virtual unsigned GetHeight(HWND hWnd) const;
	virtual unsigned GetCropWidth(HWND hWnd) const;
	virtual unsigned GetCropHeight(HWND hWnd) const;*/
	//virtual void SetWireframe(bool enable); // see RendererD3D11
	virtual bool GetWireframe() const { return mForcedWireframe; }
	virtual void SetClearColor(HWND_ID id, const Color& color);
	virtual void SetClearDepthStencil(HWND_ID id, float z, UINT8 stencil);
	virtual void SetCamera(ICamera* pCamera);
	virtual ICamera* GetCamera() const;
	virtual ICamera* GetMainCamera() const;
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
	virtual void ClearDurationTexts();
	// without depth culling
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1);
	virtual void DrawQuadLater(const Vec2I& pos, const Vec2I& size, const Color& color);
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

	virtual void SetCurRenderTarget(IRenderTarget* renderTarget);
	virtual bool IsMainRenderTarget() const;
	virtual IRenderTarget* GetCurRenderTarget() const;
	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num,
		ITexture* pDepthStencil, size_t dsViewIndex);
	virtual const Vec2I& GetRenderTargetSize(HWND_ID id = INVALID_HWND_ID) const;
	virtual const Vec2I& GetRenderTargetSize(HWND hwnd = 0) const;
	
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
	virtual ILight* GetMainDirectionalLight(int idx) const;

	virtual void SetEnvironmentTexture(ITexture* pTexture);
	virtual void SetEnvironmentTextureOverride(ITexture* texture);

	// Render to Texture Pool handling
	virtual IRenderTarget* CreateRenderTarget(const RenderTargetParam& param);
	virtual void DeleteRenderTarget(IRenderTarget*);


	//-------------------------------------------------------------------------
	// Render States
	virtual void RestoreRenderStates();
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
	virtual void SetNoColorWriteState();


	void LockBlendState();
	void UnlockBlendState();

	// depth
	virtual void SetNoDepthWriteLessEqual();
	virtual void SetLessEqualDepth();
	virtual void SetNoDepthStencil();

	// raster
	virtual void SetFrontFaceCullRS();
	virtual void SetOneBiasedDepthRS();

	// sampler
	virtual void SetSamplerState(SAMPLERS::Enum s, BINDING_SHADER shader, int slot);

	// write depth to render target
	virtual void SetDepthWriteShader();
	virtual void SetDepthWriteShaderCloud();

	// write depth to depth buffer
	virtual void SetDepthOnlyShader();

	virtual ITexture* GetTemporalDepthBuffer(const Vec2I& size);	

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

	bool GetSampleOffsets_Bloom(DWORD dwTexSize,
		float afTexCoordOffset[15],
		Vec4* avColorWeight,
		float fDeviation, float fMultiplier);
	void GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight, Vec4** avTexCoordOffset, Vec4** avSampleWeight, float fMultiplier);
	void GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4* avTexCoordOffset);

	ITexture* FindRenderTarget(const Vec2I& size);
	IShader* GetGodRayPS();
	IShader* GetGlowShader();
	void SetShadowMapShader();
	IShader* GetSilouetteShader();
	IShader* GetCopyPS();
	IShader* GetCopyPSMS();
	ITexture* GetToneMap(unsigned idx);
	IShader* GetSampleLumInitialShader();
	IShader* GetSampleLumIterativeShader();
	IShader* GetSampleLumFinalShader();
	void SwapLuminanceMap();
	ITexture* GetLuminanceMap(unsigned idx);
	IShader* GetCalcAdapedLumShader();
	IShader* GetBrightPassPS();
	IShader* GetBlur5x5PS();
	IShader* GetBloomPS();
	IShader* GetStarGlareShader();
	IShader* GetMergeTexturePS();
	IShader* GetToneMappingPS();
	OBJECT_CONSTANTS mObjConst;

	void Render(float dt);
	/*void SetGodRayRenderTarget();
	void GodRay();
	void BlendGodRay();*/
	
	

	//void BlendGlow();
	

	virtual void BindDepthTexture(bool set);
	/*void SetCloudVolumeTarget();
	void SetCloudVolumeTexture(bool set);
	virtual void SetCloudRendering(bool rendering);*/

	void BindNoiseMap();
	bool IsLuminanceOnCpu() const { return mLuminanceOnCpu; }
	// shadow
	
	/*void PrepareShadowMapRendering();
	void EndShadowMapRendering();
	void BindShadowMap(bool bind);*/

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
	
	void UseFilmicToneMapping(bool filmic);
	void CreateToneMappingShader();

	virtual void SetFadeAlpha(float alpha);
	void RenderFade();

	PointLightMan* GetPointLightMan() const { return mPointLightMan; }
	virtual IMaterial* GetMaterial(DEFAULT_MATERIALS::Enum type);

	void ProcessInputData();
	void OnInputFromEngineForCamera(IMouse* mouse, IKeyboard* keyboard);

	virtual int CropSize8(int size) const;
	void Render3DUIsToTexture();
	void RenderMarks();
	void RenderUI(HWND_ID hwndId);
	void RenderFrameProfiler();

	virtual void RegisterUIs(HWND_ID hwndId, std::vector<IUIObject*>& uiobj);
	virtual void UnregisterUIs(HWND_ID hwndId);
	virtual void Register3DUIs(HWND_ID hwndId, const char* name, std::vector<IUIObject*>& objects);
	virtual void Unregister3DUIs(const char* name);
	virtual void Set3DUIPosSize(const char* name, const Vec3& pos, const Vec2& sizeInWorld);
	virtual void Reset3DUI(const char* name);
	virtual void SetEnable3DUIs(bool enable);


	virtual void AddMarkObject(IObject* mark);
	virtual void RemoveMarkObject(IObject* mark);
	virtual void AddHPBarObject(IObject* hpBar);
	virtual void RemoveHPBarObject(IObject* hpBar);

	void OnRenderTargetDeleted(RenderTarget* renderTarget);

	void SetScene(IScene* scene);

	virtual void UpdateFrameConstantsBuffer() = 0;
	virtual void UpdateMaterialConstantsBuffer(void* pData) = 0;
	virtual void UpdateCameraConstantsBuffer() = 0;
	virtual void UpdateRenderTargetConstantsBuffer() = 0;
	virtual void UpdateSceneConstantsBuffer() = 0;
	virtual void UpdateRareConstantsBuffer() = 0;
	virtual void UpdateRadConstantsBuffer(void* pData) = 0;

	virtual void AddRenderListener(IRenderListener* listener);
	virtual void RemoveRenderListener(IRenderListener* listener);

	virtual void RegisterVideoPlayer(IVideoPlayer* player);
	virtual void UnregisterVideoPlayer(IVideoPlayer* player);
	virtual void GenGGX();
	virtual void ChangeFullscreenMode(int mode) = 0;
	virtual void TakeScreenshot();

	const char* GetNextScreenshotFile();
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