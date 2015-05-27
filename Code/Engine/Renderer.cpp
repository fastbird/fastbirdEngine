#include <Engine/StdAfx.h>
#include <Engine/Renderer.h>
#include <Engine/Font.h>
#include <Engine/Material.h>
#include <Engine/DebugHud.h>
#include <Engine/GeometryRenderer.h>
#include <Engine/RenderTarget.h>
#include <Engine/ILight.h>
#include <Engine/SkySphere.h>
#include <Engine/IConsole.h>
#include <Engine/MeshObject.h>
#include <Engine/IScriptSystem.h>
#include <Engine/VolumetricCloud.h>
#include <Engine/CloudManager.h>
#include <Engine/PointLightMan.h>
#include <Engine/RenderPipeline.h>
#include <Engine/UIObject.h>
#include <Engine/ParticleManager.h>
#include <Engine/IRenderListener.h>
#include <CommonLib/Unicode.h>

namespace fastbird
{

	extern STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES];
	extern int                      s_nLibStarDefs;
//----------------------------------------------------------------------------
	Renderer::Renderer()
		: DEFAULT_DYN_VERTEX_COUNTS(100)
		, mCurRTSize(100, 100)
		, mCurProcessingScene(0)
		, mCamera(0)
		, mForcedWireframe(false)
		, mNextEnvUpdateSkySphere(0)
		, mLockBlendState(false)
		, mLockDepthStencil(false)
		, mCameraBackup(0)
		, mPointLightMan(0)
		, mRefreshPointLight(true)
		, mLuminanceOnCpu(false)
		, mFrameLuminanceCalced(0)
		, mLuminance(0.5f)
		, mUseFilmicToneMapping(true)
		, mFadeAlpha(0.0f)
		, m3DUIEnabled(true)
{
	assert(gFBEnv->pConsole);
	gFBEnv->pConsole->AddListener(this);
	mCloudManager = FB_NEW(CloudManager);

	for (int i = 0; i < SAMPLERS::NUM; ++i)
	{
		mDefaultSamplers[i] = 0;
	}

	float y = 0.0f;
	for (int i = 0; i < MaxDebugRenderTargets; i++)
	{
		mDebugRenderTargets[i].mPos = Vec2(0, y);
		mDebugRenderTargets[i].mSize = Vec2(0.25, 0.24f);
		y += 0.25f;
	}
	mPointLightMan = FB_NEW(PointLightMan);
}
Renderer::~Renderer()
{
	FB_DELETE(mPointLightMan);
	StarDef::FinalizeStatic();
	if (gFBEnv->pConsole)
		gFBEnv->pConsole->RemoveListener(this);
}

// called from inherited classes.
void Renderer::Deinit()
{
	RenderTarget::ReleaseStarDef();
	for (auto it : mUI3DObjectsRTs)
	{
		gFBEnv->pRenderer->DeleteRenderTarget(it.second);
	}

	for (auto it : mUI3DRenderObjs)
	{
		FB_DELETE(it.second);
	}

	mMergeTexture2 = 0;
	mStarGlareShader = 0;
	mSwapChainRenderTargets.clear();
	mBlur5x5 = 0;
	mSilouetteShader = 0;
	mShadowMapShader = 0;
	mCloudDepthWriteShader = 0;
	mNoiseMap = 0;
	mRedAlphaMaskAddMinusBlend = 0;
	mGreenAlphaMaskAddAddBlend = 0;
	mRedAlphaMaskAddAddBlend = 0;
	mGreenAlphaMaskAddMinusBlend = 0;
	mRedAlphaMaskBlend = 0;
	mGreenAlphaMaskBlend = 0;
	mGreenMaskBlend = 0;
	mRSCloudFar = 0;
	mRSCloudNear = 0;
	mCloudVolumeDepth = 0;
	mFrontFaceCullRS = 0;
	mOneBiasedDepthRS = 0;
	FB_SAFE_DEL(mCloudManager);
	// All release operation should be done here.
	SkySphere::DeleteSharedEnvRT();
	CleanDepthWriteResources();
	CleanGlowResources();
	CleanHDRResources();
	CleanGodRayResources();
	mPositionInputLayout = 0;
	mFullscreenQuadVSFar = 0;
	mFullscreenQuadVSNear = 0;
	mCopyPS = 0;
	mCopyPSMS = 0;

	mDirectionalLight[0] = 0;
	mDirectionalLight[1] = 0;
	mDebugHud = 0;
	mGeomRenderer = 0;
	mFont = 0;
	mMissingMaterial = 0;
	mEnvironmentTexture = 0;

	for (int i = 0; i < DEFAULT_MATERIALS::COUNT; i++)
	{
		mMaterials[i] = 0;
	}

	mMaterialCache.clear();
	mTextureAtalsCache.clear();
	mTextureCache.clear();
	mInputLayouts.clear();

	mMaxBlendState = 0;
	mDefaultBlendState = 0;
	mAlphaBlendState = 0;
	mAdditiveBlendState = 0;
	mDefaultDepthStencilState = 0;
	mNoDepthStencilState = 0;
	mDefaultRasterizerState = 0;
	mNoDepthWriteLessEqualState = 0;
	mLessEqualDepthState = 0;

	mRenderTargets.clear();
	mInputLayouts.clear();

	for (int i = 0; i < DEFAULT_INPUTS::COUNT; i++)
	{
		mDynVBs[i] = 0;
	}

	for (auto rt : mRenderTargetPool)
	{
		FB_DELETE(rt);
	}
	mRenderTargetPool.clear();	
}

void Renderer::CleanDepthWriteResources()
{
	mDepthWriteShader = 0;
	mDepthTarget = 0;
	mPositionInputLayout = 0;
}

void Renderer::CleanGlowResources()
{
	mGlowPS = 0;
}

void Renderer::CleanHDRResources()
{
	for (int i = 0; i < FB_NUM_TONEMAP_TEXTURES_NEW; ++i)
		mToneMap[i] = 0;
	for (int i = 0; i < FB_NUM_LUMINANCE_TEXTURES; ++i)
		mLuminanceMap[i] = 0;
	mToneMappingPS = 0;
	mBrightPassPS = 0;
	mBloomPS = 0;
}

void Renderer::CleanGodRayResources()
{
	gFBEnv->mGodRayInScreen = false;
	mGodRayPS = 0;
	mOccPrePassGSShader = 0;
	mOccPrePassShader = 0;
	mOccPrePassShader = 0;
}

//----------------------------------------------------------------------------
bool Renderer::OnPrepared()
{
	mMaterials[DEFAULT_MATERIALS::QUAD] = fastbird::IMaterial::CreateMaterial(
		"es/materials/quad.material");
	mMaterials[DEFAULT_MATERIALS::QUAD_TEXTURE] = fastbird::IMaterial::CreateMaterial(
		"es/materials/QuadWithTexture.material");
	mMaterials[DEFAULT_MATERIALS::BILLBOARDQUAD] = fastbird::IMaterial::CreateMaterial(
		"es/materials/billboardQuad.material");

	// POSITION
	{
		INPUT_ELEMENT_DESC desc("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION].push_back(desc);
	}

	// POSITION_COLOR
	{
		INPUT_ELEMENT_DESC desc[] = 
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR].push_back(desc[1]);
	}

	// POSITION_COLOR_TEXCOORD
	{
		INPUT_ELEMENT_DESC desc[] =
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 16,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[1]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[2]);
	}

	// POSITION_HDR_COLOR
	{
		INPUT_ELEMENT_DESC desc[] =
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_HDR_COLOR].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_HDR_COLOR].push_back(desc[1]);
	}

	// POSITION_NORMAL,
	{
		INPUT_ELEMENT_DESC desc[] = 
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
			INPUT_ELEMENT_DESC("NORMAL", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL].push_back(desc[1]);
	}
	
	//POSITION_TEXCOORD,
	{
		INPUT_ELEMENT_DESC desc[] = 
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 12, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_TEXCOORD].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_TEXCOORD].push_back(desc[1]);
	}
	//POSITION_COLOR_TEXCOORD_BLENDINDICES,
	{
		INPUT_ELEMENT_DESC desc[] = 
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
			INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 16, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
			INPUT_ELEMENT_DESC("BLENDINDICES", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 24, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
			.push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
			.push_back(desc[1]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
			.push_back(desc[2]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
			.push_back(desc[3]);
	}

	//POSITION_NORMAL_TEXCOORD,
	{
		INPUT_ELEMENT_DESC desc[] = 
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("NORMAL", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 24, 
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 ),
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[1]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[2]);
	}

	// POSITION_VEC4,
	{
		INPUT_ELEMENT_DESC desc[] =
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[1]);
	}

	// POSITION_VEC4_COLOR,
	{
		INPUT_ELEMENT_DESC desc[] =
		{
			INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 28,
			INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
		};
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[0]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[1]);
		mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[2]);
	}

	//-----------------------------------------------------------------------
	mDynVBs[DEFAULT_INPUTS::POSITION] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_P), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PC), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PCT),
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_NORMAL] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PN), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_TEXCOORD] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PT), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES] = 
		CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PCTB), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_VEC4] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PV4),
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_VEC4_COLOR] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PV4C),
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

	//-----------------------------------------------------------------------
	static_assert(DEFAULT_INPUTS::COUNT == 10, "You may not define a new element of mInputLayoutDesc for the new description.");

	mFont = FB_NEW(Font);
	std::string fontName = gFBEnv->pScriptSystem->GetStringVariable("r_font");
	if (fontName.empty())
	{
		fontName = "es/fonts/nanum_pen_bin.fnt";
	}
	mFont->Init(fontName.c_str());
	if (mFont)
	{

		mFont->SetRenderTargetSize(GetMainRTSize());
	}
	mFont->SetTextEncoding(IFont::UTF16);

	mDebugHud = FB_NEW(DebugHud);
	mGeomRenderer = FB_NEW(GeometryRenderer);

	if (gFBEnv->pConsole)
		gFBEnv->pConsole->Init();

	mDefaultRasterizerState = CreateRasterizerState(RASTERIZER_DESC());
	mDefaultBlendState = CreateBlendState(BLEND_DESC());
	mDefaultDepthStencilState = CreateDepthStencilState(DEPTH_STENCIL_DESC());
	DEPTH_STENCIL_DESC ddesc;
	ddesc.DepthEnable = false;
	ddesc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	mNoDepthStencilState = CreateDepthStencilState(ddesc);


	IMaterial::SHADER_DEFINES emptyShaderDefines;
	mFullscreenQuadVSNear = CreateShader("es/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS,
		emptyShaderDefines);
	IMaterial::SHADER_DEFINES shaderDefinesFar;
	shaderDefinesFar.push_back(IMaterial::ShaderDefine("_FAR_SIDE_QUAD", "1"));
	mFullscreenQuadVSFar = CreateShader("es/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS,
		shaderDefinesFar);

	mCopyPS = CreateShader("es/shaders/copyps.hlsl", BINDING_SHADER_PS,
		emptyShaderDefines);
	IMaterial::SHADER_DEFINES multiSampleSD;
	multiSampleSD.push_back(IMaterial::ShaderDefine("_MULTI_SAMPLE", "1"));
	mCopyPSMS = CreateShader("es/shaders/copyps.hlsl", BINDING_SHADER_PS,
		multiSampleSD);

	SkySphere::CreateSharedEnvRT();

	SAMPLER_DESC sdesc;
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
	mDefaultSamplers[SAMPLERS::POINT] = CreateSamplerState(sdesc);
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
	mDefaultSamplers[SAMPLERS::LINEAR] = CreateSamplerState(sdesc);
	sdesc.Filter = TEXTURE_FILTER_ANISOTROPIC;
	mDefaultSamplers[SAMPLERS::ANISOTROPIC] = CreateSamplerState(sdesc);

	sdesc.Filter = TEXTURE_FILTER_COMPARISON_ANISOTROPIC;
	sdesc.AddressU = TEXTURE_ADDRESS_BORDER;
	sdesc.AddressV = TEXTURE_ADDRESS_BORDER;
	sdesc.AddressW = TEXTURE_ADDRESS_BORDER;
	for (int i = 0; i < 4; i++)
		sdesc.BorderColor[i] = 1.0f;
	sdesc.ComparisonFunc = COMPARISON_LESS_EQUAL;
	mDefaultSamplers[SAMPLERS::SHADOW] = CreateSamplerState(sdesc);

	sdesc.ComparisonFunc = COMPARISON_ALWAYS;
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
	sdesc.AddressU = TEXTURE_ADDRESS_WRAP;
	sdesc.AddressV = TEXTURE_ADDRESS_WRAP;
	sdesc.AddressW = TEXTURE_ADDRESS_WRAP;
	mDefaultSamplers[SAMPLERS::POINT_WRAP] = CreateSamplerState(sdesc);
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
	mDefaultSamplers[SAMPLERS::LINEAR_WRAP] = CreateSamplerState(sdesc);

	SAMPLER_DESC sdesc_border;
	sdesc_border.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
	sdesc_border.AddressU = TEXTURE_ADDRESS_BORDER;
	sdesc_border.AddressV = TEXTURE_ADDRESS_BORDER;
	sdesc_border.AddressW = TEXTURE_ADDRESS_BORDER;
	for (int i = 0; i < 4; i++)
		sdesc_border.BorderColor[i] = 0;
	mDefaultSamplers[SAMPLERS::BLACK_BORDER] = CreateSamplerState(sdesc_border);

	

	for (int i = 0; i < SAMPLERS::NUM; ++i)
	{	
		assert(mDefaultSamplers[i] != 0);
		SetSamplerState((SAMPLERS::Enum)i, BINDING_SHADER_PS, i);
	}
	
	SetSamplerState(SAMPLERS::POINT, BINDING_SHADER_VS, SAMPLERS::POINT);

	mMiddleGray = gFBEnv->pConsole->GetEngineCommand()->r_HDRMiddleGray;
	mStarPower = gFBEnv->pConsole->GetEngineCommand()->r_StarPower;
	mBloomPower = gFBEnv->pConsole->GetEngineCommand()->r_BloomPower;
	UpdateRareConstantsBuffer();


	return true;
}

void Renderer::OnSwapchainCreated(HWND_ID id)
{
	auto rt = mSwapChainRenderTargets[id];
	auto scene = gFBEnv->pEngine->CreateScene();
	rt->SetScene(scene);

	if (id==1) // main
	{
		rt->Bind();
		OnPrepared();
	}
}

//----------------------------------------------------------------------------
void Renderer::ProcessRenderTarget()
{
	for (auto pRT : mRenderTargets)
	{
		pRT->Render();
	}
}

//----------------------------------------------------------------------------
void Renderer::SetClearColor(HWND_ID id, const Color& color)
{
	auto it = mSwapChainRenderTargets.Find(id);
	if (it == mSwapChainRenderTargets.end())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("Cannot find the render target(%u).", id));
		return;
	}
	it->second->SetClearColor(color);
}

//----------------------------------------------------------------------------
void Renderer::SetClearDepthStencil(HWND_ID id, float z, UINT8 stencil)
{
	auto it = mSwapChainRenderTargets.Find(id);
	if (it == mSwapChainRenderTargets.end())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("Cannot find the render target(%u).", id));
		return;
	}
	it->second->SetClearDepthStencil(z, stencil);
}

//----------------------------------------------------------------------------
void Renderer::SetCamera(ICamera* pCamera)
{
	ICamera* prev = mCamera;
	if (prev)
		prev->SetCurrent(false);
	mCamera = pCamera;
	if (mCamera){
		mCamera->SetCurrent(true);
		UpdateCameraConstantsBuffer();
	}
}

//----------------------------------------------------------------------------
ICamera* Renderer::GetCamera() const
{
	return mCamera;
}

//----------------------------------------------------------------------------
ICamera* Renderer::GetMainCamera() const
{
	auto rt = GetMainRenderTarget();
	if (rt)
	{
		return rt->GetCamera();
	}
	Error(FB_DEFAULT_DEBUG_ARG, "No main camera!");
	assert(0);
	return 0;
}

//----------------------------------------------------------------------------
void Renderer::InitFrameProfiler(float dt)
{
	mFrameProfiler.Clear();
	mFrameProfiler.UpdateFrameRate(dt);
}

//----------------------------------------------------------------------------
const RENDERER_FRAME_PROFILER& Renderer::GetFrameProfiler() const
{
	return mFrameProfiler;
}

//----------------------------------------------------------------------------
Vec2I Renderer::ToSreenPos(HWND_ID id, const Vec3& ndcPos) const
{
	auto it = mSwapChainRenderTargets.Find(id);
	if (it == mSwapChainRenderTargets.end())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("Window id %u is not found.", id));
		return Vec2I(0, 0);
	}
	const auto& size = it->second->GetSize();
	Vec2I ret;
	ret.x = (int)(((float)size.x*.5f) * ndcPos.x + size.x*.5f);
	ret.y = (int)((-(float)size.y*.5f) * ndcPos.y + size.y*.5f);
	return ret;
}

Vec2 Renderer::ToNdcPos(HWND_ID id, const Vec2I& screenPos) const
{
	auto it = mSwapChainRenderTargets.Find(id);
	if (it == mSwapChainRenderTargets.end())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("Window id %u is not found.", id));
		return Vec2(0, 0);
	}
	const auto& size = it->second->GetSize();
	Vec2 ret;
	ret.x = (float)screenPos.x / (float)size.x * 2.0f - 1.0f;
	ret.y = -((float)screenPos.y / (float)size.y * 2.0f - 1.0f);
	return ret;
}

unsigned Renderer::GetWidth(HWND_ID id) const
{
	auto it = mWidth.Find(id);
	if (it != mWidth.end())
	{
		return it->second;
	}
	Error(FB_DEFAULT_DEBUG_ARG, "Window width not found!");
	return 100;
}
unsigned Renderer::GetHeight(HWND_ID id) const
{
	auto it = mHeight.Find(id);
	if (it != mHeight.end())
	{
		return it->second;
	}
	Error(FB_DEFAULT_DEBUG_ARG, "Window height not found!");
	return 100;
}
unsigned Renderer::GetWidth(HWND hWnd) const
{
	auto id = gFBEnv->pEngine->GetWindowHandleId(hWnd);
	return GetWidth(id);
}
unsigned Renderer::GetHeight(HWND hWnd) const
{
	auto id = gFBEnv->pEngine->GetWindowHandleId(hWnd);
	return GetHeight(id);
}

unsigned Renderer::GetCropWidth(HWND hWnd) const
{
	auto width = GetWidth(hWnd);
	return CropSize8(width);
}

unsigned Renderer::GetCropHeight(HWND hWnd) const
{
	auto height = GetHeight(hWnd);
	return CropSize8(height);
}

//----------------------------------------------------------------------------
void Renderer::DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
	const Color& color, float size)
{
	if (mDebugHud)
		mDebugHud->DrawTextForDuration(secs, pos, text, color, size);
}

void Renderer::DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
	const Color& color, float size)
{
	DrawTextForDuration(secs, pos, AnsiToWide(text, strlen(text)), color, size);
}

void Renderer::DrawText(const Vec2I& pos, WCHAR* text, const Color& color, float size)
{
	if (mDebugHud)
		mDebugHud->DrawText(pos, text, color, size);
}

void Renderer::DrawText(const Vec2I& pos, const char* text, const Color& color, float size)
{
	DrawText(pos, AnsiToWide(text, strlen(text)), color, size);
}

void Renderer::Draw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, float size)
{
	if (mDebugHud)
		mDebugHud->Draw3DText(worldpos, text, color, size);
}

void Renderer::Draw3DText(const Vec3& worldpos, const char* text, const Color& color, float size)
{
	Draw3DText(worldpos, AnsiToWide(text), color, size);
}

void Renderer::DrawLine(const Vec3& start, const Vec3& end, 
	const Color& color0, const Color& color1)
{
	if (mDebugHud)
		mDebugHud->DrawLine(start, end, color0, color1);
}

void Renderer::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
	const Color& color0, const Color& color1)
{
	if (mDebugHud)
		mDebugHud->DrawLineBeforeAlphaPass(start, end, color0, color1);
}

void Renderer::DrawLine(const Vec2I& start, const Vec2I& end, 
	const Color& color0, const Color& color1)
{
	if (mDebugHud)
		mDebugHud->DrawLine(start, end, color0, color1);
}

void Renderer::DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
	const char* texture, bool textureFlow)
{
	if (mGeomRenderer)
		mGeomRenderer->DrawTexturedThickLine(start, end, color0, color1, thickness, texture, textureFlow);
}


void Renderer::DrawSphere(const Vec3& pos, float radius, const Color& color)
{
	if (mGeomRenderer)
		mGeomRenderer->DrawSphere(pos, radius, color);
}
void Renderer::DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha)
{
	if (mGeomRenderer)
		mGeomRenderer->DrawBox(boxMin, boxMax, color, alpha);
}
void Renderer::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha)
{
	if (mGeomRenderer)
		mGeomRenderer->DrawTriangle(a, b, c, color, alpha);
}

void Renderer::RenderGeoms()
{
	if (!mGeomRenderer)
		return;
	mGeomRenderer->PreRender();
	mGeomRenderer->Render();
}

void Renderer::RenderDebugHud()
{
	if (!mDebugHud)
		return;
	D3DEventMarker devent("RenderDebugHud");
	for (auto l : mRenderListeners)
	{
		l->BeforeDebugHudRendered(gFBEnv->pEngine->GetMainWndHandleId());
	}

	//bool backup = GetWireframe();
	//SetWireframe(false);
	RestoreRenderStates();
	mDebugHud->PreRender();
	mDebugHud->Render();
	//SetWireframe(backup);
	for (auto l : mRenderListeners)
	{
		l->AfterDebugHudRendered(gFBEnv->pEngine->GetMainWndHandleId());
	}
}

//-------------------------------------------------------------------------
inline IFont* Renderer::GetFont() const
{
	return mFont;
}

void Renderer::SetCurRenderTarget(IRenderTarget* renderTarget)
{
	mCurRenderTarget = renderTarget;
}

bool Renderer::IsMainRenderTarget() const
{
	return GetMainRenderTarget() == mCurRenderTarget;
}

IRenderTarget* Renderer::GetCurRendrTarget() const
{
	return mCurRenderTarget;
}

void Renderer::SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num,
	ITexture* pDepthStencil, size_t dsViewIndex)
{
	static float time = 0;
	static std::set<ITexture*> usedRenderTargets;
	if (gFBEnv->pConsole->GetEngineCommand()->r_numRenderTargets)
	{
		for (int i = 0; i<num; i++)
		{
			usedRenderTargets.insert(pRenderTargets[i]);
		}
		if (gpTimer->GetTime() - time > 5)
		{
			time = gpTimer->GetTime();
			Log("used RenderTargets = %u", usedRenderTargets.size());
		}
	}
	if (pRenderTargets && num>0 && pRenderTargets[0])
	{
		mCurRTSize = pRenderTargets[0]->GetSize();
	}
	else
	{
		mCurRTSize = GetMainRTSize();
	}
	if (mFont)
		mFont->SetRenderTargetSize(mCurRTSize);

	UpdateRenderTargetConstantsBuffer();
}

const Vec2I& Renderer::GetRenderTargetSize() const
{
	return mCurRTSize;
}

IRenderTarget* Renderer::GetMainRenderTarget() const
{
	if (mSwapChainRenderTargets.empty())
		return 0;

	auto it = mSwapChainRenderTargets.begin();
	assert(it->first == 1 && "Need to investigate");
	return it->second;
}

IRenderTarget* Renderer::GetRenderTarget(HWND_ID id) const
{
	auto it = mSwapChainRenderTargets.Find(id);
	if (it == mSwapChainRenderTargets.end())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("No render target is found for the hwnd id %u", id));
		return 0;
	}
	return it->second;
}

IScene* Renderer::GetMainScene() const
{
	auto rt = GetMainRenderTarget();
	if (rt)
	{
		return rt->GetScene();
	}
	if (!gFBEnv->mExiting)
		Error(FB_DEFAULT_DEBUG_ARG, "No main render target!");
	return 0;
}

IScene* Renderer::GetScene() const
{
	auto hwndId = gFBEnv->pEngine->GetForegroundWindowId();
	auto rt = GetRenderTarget(hwndId);
	if (rt)
	{
		return rt->GetScene();
	}

	// fall back
	if (mCurRenderTarget)
		return mCurRenderTarget->GetScene();

	assert(0);
	return 0;
}

const Vec2I& Renderer::GetMainRTSize() const
{
	auto rt = GetMainRenderTarget();
	if (rt)
	{
		return rt->GetSize();
	}
	if (!gFBEnv->mExiting)
		Error(FB_DEFAULT_DEBUG_ARG, "No main render target!");
	return Vec2I::ZERO;
}

const INPUT_ELEMENT_DESCS& Renderer::GetInputElementDesc(
		DEFAULT_INPUTS::Enum e)
{
	return mInputLayoutDescs[e];
}

IInputLayout* Renderer::GetInputLayout(const INPUT_ELEMENT_DESCS& descs)
{
	auto it = mInputLayouts.find(descs);
	if (it==mInputLayouts.end())
		return 0;

	assert(it->second->GetDescs() == descs);

	return it->second;
}

IInputLayout* Renderer::GetInputLayout(DEFAULT_INPUTS::Enum e,
		IMaterial* material)
{
	const INPUT_ELEMENT_DESCS& descs = GetInputElementDesc(e);
	return GetInputLayout(descs, material);
}

IInputLayout* Renderer::GetInputLayout(DEFAULT_INPUTS::Enum e,
		IShader* shader)
{
	const INPUT_ELEMENT_DESCS& descs = GetInputElementDesc(e);
	return GetInputLayout(descs, shader);
}

TextureAtlas* Renderer::GetTextureAtlas(const char* path)
{
	std::string filepath(path);
	ToLowerCase(filepath);
	TextureAtlas* pTextureAtlas = 0;
	for (const auto& ta : mTextureAtalsCache)
	{
		if (ta->mPath == filepath)
		{
			return ta;
		}
	}

	if (!pTextureAtlas)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filepath.c_str());
		if (doc.Error())
		{
			const char* errMsg = doc.GetErrorStr1();
			if (errMsg)
				Error("texture atlas error : \t%s", errMsg);
			else
				Error(FB_DEFAULT_DEBUG_ARG, "Error while parsing material!");
			return 0;
		}
		tinyxml2::XMLElement* pRoot = doc.FirstChildElement("TextureAtlas");
		if (!pRoot)
		{
			Error(FB_DEFAULT_DEBUG_ARG, "Invalid TextureAtlas format!");
			return 0;
		}

		const char* szBuffer = pRoot->Attribute("file");
		SmartPtr<ITexture> pTexture;
		if (szBuffer)
		{
			pTextureAtlas = FB_NEW(TextureAtlas);
			pTextureAtlas->mPath = filepath;
			mTextureAtalsCache.push_back(pTextureAtlas);
			pTextureAtlas->mTexture = CreateTexture(szBuffer);
			if (!pTextureAtlas->mTexture)
			{
				Log("Texture %s not found.", szBuffer);
			}
		}
		else
		{
			Error(FB_DEFAULT_DEBUG_ARG, "Invalid TextureAtlas format! No Texture Defined.");
			return 0;
		}
		
		Vec2I textureSize = pTextureAtlas->mTexture->GetSize();
		if (textureSize.x !=0 && textureSize.y !=0)
		{
			tinyxml2::XMLElement* pRegionElem = pRoot->FirstChildElement("region");
			while (pRegionElem)
			{
				szBuffer = pRegionElem->Attribute("name");
				if (!szBuffer)
				{
					Log(FB_DEFAULT_DEBUG_ARG, "No name for texture atlas region");
					continue;
				}

				TextureAtlasRegion* pRegion = pTextureAtlas->CreateRegion();
				pRegion->mName = szBuffer;
				pRegion->mID = pRegionElem->UnsignedAttribute("id");
				
				pRegion->mStart.x = pRegionElem->IntAttribute("x");
				pRegion->mStart.y = pRegionElem->IntAttribute("y");
				pRegion->mSize.x = pRegionElem->IntAttribute("width");
				pRegion->mSize.y = pRegionElem->IntAttribute("height");
				Vec2 start((float)pRegion->mStart.x, (float)pRegion->mStart.y);
				Vec2 end(start.x + pRegion->mSize.x, start.y + pRegion->mSize.y);
				pRegion->mUVStart = start / textureSize;
				pRegion->mUVEnd = end / textureSize;
				pTextureAtlas->AddRegion(pRegion);

				pRegionElem = pRegionElem->NextSiblingElement();
			}
		}
		else
		{
			Error("Texture size is 0,0");
		}
	}

	return pTextureAtlas;
}

TextureAtlasRegion* Renderer::GetTextureAtlasRegion(const char* path, const char* region)
{
	TextureAtlas* pTextureAtlas = GetTextureAtlas(path);
	if (pTextureAtlas)
	{
		return pTextureAtlas->GetRegion(region);
	}

	return 0;
}

bool Renderer::ReloadTextureAtlas(const char* path)
{
	for (const auto& ta : mTextureAtalsCache)
	{
		if (_stricmp(ta->mPath.c_str(), path)==0)
		{
			return ta->ReloadTextureAtlas();
		}
	}
	return false;
}

bool TextureAtlas::ReloadTextureAtlas()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(mPath.c_str());
	if (doc.Error()) {
		const char* errMsg = doc.GetErrorStr1();
		if (errMsg)
			Error(FB_DEFAULT_DEBUG_ARG, FormatString("texture atlas error : \t%s", errMsg));
		else
			Error(FB_DEFAULT_DEBUG_ARG, "ReloadTextureAtlas Failed");
		return false;
	}
	tinyxml2::XMLElement* pRoot = doc.FirstChildElement("TextureAtlas");
	if (!pRoot) {
		Error(FB_DEFAULT_DEBUG_ARG, "Invalid TextureAtlas format!");
		return false;
	}

	const char* szBuffer = pRoot->Attribute("file");
	SmartPtr<ITexture> pTexture;
	if (szBuffer) {
		mTexture = gFBEnv->pRenderer->CreateTexture(szBuffer);
		if (!mTexture)
		{
			Error(FB_DEFAULT_DEBUG_ARG, FormatString("Texture %s not found.", szBuffer));
		}
	} else {
		Error(FB_DEFAULT_DEBUG_ARG, "Invalid TextureAtlas format! No Texture Defined.");
		return false;
	}

	Vec2I textureSize = mTexture->GetSize();
	if (textureSize.x != 0 && textureSize.y != 0) {
		tinyxml2::XMLElement* pRegionElem = pRoot->FirstChildElement("region");
		while (pRegionElem) {
			szBuffer = pRegionElem->Attribute("name");
			if (!szBuffer)
			{
				Error(FB_DEFAULT_DEBUG_ARG, "No name for texture atlas region");
				continue;
			}			
			TextureAtlasRegion* pRegion = GetRegion(szBuffer);
			if (!pRegion)
			{
				pRegion = CreateRegion();
				pRegion->mName = szBuffer;
				AddRegion(pRegion);
			}
			pRegion->mID = pRegionElem->UnsignedAttribute("id");
			pRegion->mStart.x = pRegionElem->IntAttribute("x");
			pRegion->mStart.y = pRegionElem->IntAttribute("y");
			pRegion->mSize.x = pRegionElem->IntAttribute("width");
			pRegion->mSize.y = pRegionElem->IntAttribute("height");
			Vec2 start((float)pRegion->mStart.x, (float)pRegion->mStart.y);
			Vec2 end(start.x + pRegion->mSize.x, start.y + pRegion->mSize.y);
			pRegion->mUVStart = start / textureSize;
			pRegion->mUVEnd = end / textureSize;
			pRegionElem = pRegionElem->NextSiblingElement();
		}
		return true;
	} else {
		Error("Texture size is 0,0");
		return false;
	}
}

IMaterial* Renderer::CreateMaterial(const char* file)
{
	std::string filepath(file);
	ToLowerCase(filepath);
	if (filepath.empty())
	{
		Log("Cannot create a material with empty file name. Loading missing material instead.");
		filepath = "es/materials/missing.material";
	}
	auto it = mMaterialCache.Find(filepath);
	if (it != mMaterialCache.end())
	{
		return it->second->Clone();
	}
	else
	{
		IMaterial* pNewMaterial = FB_NEW(Material);
		pNewMaterial->LoadFromFile(filepath.c_str());
		mMaterialCache.Insert(std::make_pair(filepath, pNewMaterial));
		return pNewMaterial->Clone();
	}
}

IMaterial* Renderer::GetMissingMaterial()
{
	static bool loaded = false;
	if (!loaded)
	{
		loaded = true;
		mMissingMaterial = CreateMaterial("es/materials/missing.material");
		if (!mMissingMaterial)
		{
			Error("Missing material not found!");
		}
	}

	return mMissingMaterial;
	
}

void Renderer::SetDirectionalLight(ILight* pLight, int idx)
{
	mDirectionalLight[idx] = pLight;
}

ILight* Renderer::GetDirectionalLight(int idx) const
{
	return mDirectionalLight[idx];
}

void Renderer::SetEnvironmentTexture(ITexture* pTexture)
{
	mEnvironmentTexture = pTexture;
	mEnvironmentTexture->SetShaderStage(BINDING_SHADER_PS);
	mEnvironmentTexture->SetSlot(4); // hardcoded environment slot.
	mEnvironmentTexture->Bind();
}

void Renderer::SetEnvironmentTextureOverride(ITexture* texture)
{
	mEnvironmentTextureOverride = texture;
	if (mEnvironmentTextureOverride)
	{
		mEnvironmentTextureOverride->SetShaderStage(BINDING_SHADER_PS);
		mEnvironmentTextureOverride->SetSlot(4);
		mEnvironmentTextureOverride->Bind();
	}
	else
	{
		if (mEnvironmentTexture)
		{
			mEnvironmentTexture->SetShaderStage(BINDING_SHADER_PS);
			mEnvironmentTexture->SetSlot(4); // hardcoded environment slot.
			mEnvironmentTexture->Bind();
		}
		else
		{
			SetTexture(0, BINDING_SHADER_PS, 4);
		}
	}
}

//---------------------------------------------------------------------------
void Renderer::RestoreRenderStates()
{
	RestoreBlendState();
	RestoreRasterizerState();
	RestoreDepthStencilState();
}

//---------------------------------------------------------------------------
void Renderer::RestoreRasterizerState()
{
	if (mDefaultRasterizerState)
		mDefaultRasterizerState->Bind();
}

//---------------------------------------------------------------------------
void Renderer::RestoreBlendState()
{
	if (mLockBlendState)
		return;
	if (mDefaultBlendState)
		mDefaultBlendState->Bind();
}

//---------------------------------------------------------------------------
void Renderer::RestoreDepthStencilState()
{
	if (mLockDepthStencil)
		return;
	if (mDefaultDepthStencilState)
		mDefaultDepthStencilState->Bind(0);
}

//---------------------------------------------------------------------------
void Renderer::LockDepthStencilState()
{
	mLockDepthStencil = true;
}

//---------------------------------------------------------------------------
void Renderer::UnlockDepthStencilState()
{
	mLockDepthStencil = false;
}

//---------------------------------------------------------------------------
void Renderer::SetAlphaBlendState()
{
	if (!mAlphaBlendState)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
		bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
		mAlphaBlendState = CreateBlendState(bdesc);
	}
	mAlphaBlendState->Bind();
}

void Renderer::SetAdditiveBlendState()
{
	if (!mAdditiveBlendState)
	{
		BLEND_DESC addDesc;
		addDesc.RenderTarget[0].BlendEnable = true;
		addDesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		addDesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		addDesc.RenderTarget[0].DestBlend = BLEND_ONE;
		mAdditiveBlendState = CreateBlendState(addDesc);
	}
	mAdditiveBlendState->Bind();	
}

void Renderer::SetMaxBlendState()
{
	if (!mMaxBlendState)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_MAX;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;
		mMaxBlendState = CreateBlendState(bdesc);
	}
	mMaxBlendState->Bind();
}
//---------------------------------------------------------------------------
void Renderer::SetRedAlphaMask()
{
	if (!mRedAlphaMaskBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
		mRedAlphaMaskBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mRedAlphaMaskBlend->Bind();	
}

//---------------------------------------------------------------------------
void Renderer::SetGreenAlphaMask()
{
	if (!mGreenAlphaMaskBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;
		mGreenAlphaMaskBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mGreenAlphaMaskBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetGreenAlphaMaskMax()
{
	if (!mGreenAlphaMaskMaxBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_MAX;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;

		mGreenAlphaMaskMaxBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mGreenAlphaMaskMaxBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetGreenAlphaMaskAddAddBlend()
{
	if (!mGreenAlphaMaskAddAddBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;

		bdesc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;

		mGreenAlphaMaskAddAddBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mGreenAlphaMaskAddAddBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetRedAlphaMaskAddMinusBlend()
{
	if (!mRedAlphaMaskAddMinusBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;

		bdesc.RenderTarget[0].BlendOpAlpha = BLEND_OP_REV_SUBTRACT;
		bdesc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
		mRedAlphaMaskAddMinusBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mRedAlphaMaskAddMinusBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetGreenAlphaMaskAddMinusBlend()
{
	if (!mGreenAlphaMaskAddMinusBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;

		bdesc.RenderTarget[0].BlendOpAlpha = BLEND_OP_REV_SUBTRACT;
		bdesc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;

		mGreenAlphaMaskAddMinusBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mGreenAlphaMaskAddMinusBlend->Bind();
}
//---------------------------------------------------------------------------
void Renderer::SetRedAlphaMaskAddAddBlend()
{
	if (!mRedAlphaMaskAddAddBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlend = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;

		bdesc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
		bdesc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
		mRedAlphaMaskAddAddBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mRedAlphaMaskAddAddBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetGreenMask()
{
	if (!mGreenMaskBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN;
		mGreenMaskBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mGreenMaskBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetBlueMask()
{
	if (!mBlueMaskBlend)
	{
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_BLUE;
		mBlueMaskBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mBlueMaskBlend->Bind();
}

void Renderer::SetNoColorWriteState(){
	if (!mNoColorWriteBlend){
		BLEND_DESC bdesc;
		bdesc.RenderTarget[0].RenderTargetWriteMask = 0;
		mNoColorWriteBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mNoColorWriteBlend->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetNoDepthWriteLessEqual()
{
	if (!mNoDepthWriteLessEqualState)
	{
		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
		ddesc.DepthFunc = COMPARISON_LESS_EQUAL;
		mNoDepthWriteLessEqualState = CreateDepthStencilState(ddesc);
	}
	mNoDepthWriteLessEqualState->Bind(0);
}

//---------------------------------------------------------------------------
void Renderer::SetLessEqualDepth()
{
	if (!mLessEqualDepthState)
	{
		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthFunc = COMPARISON_LESS_EQUAL;
		mLessEqualDepthState = CreateDepthStencilState(ddesc);
	}
	mLessEqualDepthState->Bind(0);
}

//---------------------------------------------------------------------------
void Renderer::SetNoDepthStencil()
{
	if (!mNoDepthStencilState)
	{
		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthEnable = false;
		ddesc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
		mNoDepthStencilState = CreateDepthStencilState(ddesc);
	}
	mNoDepthStencilState->Bind(0);
}

//---------------------------------------------------------------------------
void Renderer::SetFrontFaceCullRS()
{
	if (!mFrontFaceCullRS)
	{
		RASTERIZER_DESC rdesc;
		rdesc.CullMode = CULL_MODE_FRONT;
		mFrontFaceCullRS = CreateRasterizerState(rdesc);
	}
	mFrontFaceCullRS->Bind();
}

void Renderer::SetOneBiasedDepthRS(){
	if (!mOneBiasedDepthRS){
		RASTERIZER_DESC rdesc;
		rdesc.DepthBias = 1;
		mOneBiasedDepthRS = CreateRasterizerState(rdesc);
	}
	mOneBiasedDepthRS->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetSamplerState(SAMPLERS::Enum s, BINDING_SHADER shader, int slot)
{
	assert(s >= SAMPLERS::POINT && s < SAMPLERS::NUM);
	mDefaultSamplers[s]->Bind(shader, slot);
}

//---------------------------------------------------------------------------
float GaussianDistribution(float x, float y, float rho)
{
	//http://en.wikipedia.org/wiki/Gaussian_filter

	float g = 1.0f / sqrt(2.0f * PI * rho * rho);
	g *= expf(-(x * x + y * y) / (2 * rho * rho));

	return g;
}

//---------------------------------------------------------------------------
bool Renderer::GetSampleOffsets_Bloom(DWORD dwTexSize,
	float afTexCoordOffset[15],
	Vec4* avColorWeight,
	float fDeviation, float fMultiplier)
{
	// if deviation is big, samples tend to have more distance among them.
	int i = 0;
	float tu = 1.0f / (float)dwTexSize;

	// Fill the center texel
	float weight = fMultiplier * GaussianDistribution(0, 0, fDeviation);
	avColorWeight[7] = Vec4(weight, weight, weight, weight);

	afTexCoordOffset[7] = 0.0f;

	// Fill one side
	for (i = 1; i < 8; i++)
	{
		weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
		afTexCoordOffset[7 - i] = -i * tu;

		avColorWeight[7 - i] = Vec4(weight, weight, weight, weight);
	}

	// Copy to the other side
	for (i = 8; i < 15; i++)
	{
		avColorWeight[i] = avColorWeight[14 - i];
		afTexCoordOffset[i] = -afTexCoordOffset[14 - i];
	}

	// Debug convolution kernel which doesn't transform input data
	/*ZeroMemory( avColorWeight, sizeof(D3DXVECTOR4)*15 );
	avColorWeight[7] = D3DXVECTOR4( 1, 1, 1, 1 );*/

	return S_OK;
}

//---------------------------------------------------------------------------
void Renderer::GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight,
	Vec4** avTexCoordOffset, Vec4** avSampleWeight, float fMultiplier)
{
	assert(avTexCoordOffset && avSampleWeight);
	auto it = mGauss5x5.Find(std::make_pair(texWidth, texHeight));
	if (it == mGauss5x5.end())
	{
		float tu = 1.0f / (float)texWidth;
		float tv = 1.0f / (float)texHeight;

		Vec4 vWhite(1.0f, 1.0f, 1.0f, 1.0f);
		std::vector<Vec4> offsets;
		std::vector<Vec4> weights;

		float totalWeight = 0.0f;
		int index = 0;
		for (int x = -2; x <= 2; x++)
		{
			for (int y = -2; y <= 2; y++)
			{
				// Exclude pixels with a block distance greater than 2. This will
				// create a kernel which approximates a 5x5 kernel using only 13
				// sample points instead of 25; this is necessary since 2.0 shaders
				// only support 16 texture grabs.
				if (abs(x) + abs(y) > 2)
					continue;

				// Get the unscaled Gaussian intensity for this offset
				offsets.push_back(Vec4(x * tu, y * tv, 0, 0));
				weights.push_back(vWhite * GaussianDistribution((float)x, (float)y, 1.0f));
				totalWeight += weights.back().x;
				++index;
			}
		}
		assert(weights.size() == 13);
		// Divide the current weight by the total weight of all the samples; Gaussian
		// blur kernels add to 1.0f to ensure that the intensity of the image isn't
		// changed when the blur occurs. An optional multiplier variable is used to
		// add or remove image intensity during the blur.
		for (int i = 0; i < index; i++)
		{
			weights[i] /= totalWeight;
			weights[i] *= fMultiplier;
		}
		auto it = mGauss5x5.Insert(std::make_pair( std::make_pair(texWidth, texHeight), std::make_pair(offsets, weights) ));
		*avTexCoordOffset = &(it->second.first[0]);
		*avSampleWeight = &(it->second.second[0]);
	}
	else
	{
		*avTexCoordOffset = &(it->second.first[0]);
		*avSampleWeight = &(it->second.second[0]);
	}
	
	
}

//---------------------------------------------------------------------------
void Renderer::GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4* avSampleOffsets)
{
	if (NULL == avSampleOffsets)
		return;

	float tU = 1.0f / texWidth;
	float tV = 1.0f / texHeight;

	// Sample from the 4 surrounding points. Since the center point will be in
	// the exact center of 4 texels, a 0.5f offset is needed to specify a texel
	// center.
	int index = 0;
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			avSampleOffsets[index].x = (x - 0.5f) * tU;
			avSampleOffsets[index].y = (y - 0.5f) * tV;

			index++;
		}
	}
}

bool Renderer::OnChangeCVar(CVar* pCVar)
{
	// name is always lower case
	if (pCVar->mName == "r_hdr")
	{
		CleanHDRResources();
		return true;
	}
	else if (pCVar->mName == "r_hdrcpuluminance")
	{
		if (pCVar->GetInt())
		{
			ToneMapLuminanceOnCpu(true);
		}
		else
		{
			ToneMapLuminanceOnCpu(false);
		}
	}
	else if (pCVar->mName == "r_hdrfilmic")
	{
		UseFilmicToneMapping(pCVar->GetInt() != 0);
	}
	else if (pCVar->mName == "r_hdrmiddlegray")
	{
		mMiddleGray = gFBEnv->pConsole->GetEngineCommand()->r_HDRMiddleGray;
		UpdateRareConstantsBuffer();
	}
	else if (pCVar->mName == "r_bloompower")
	{
		mBloomPower = gFBEnv->pConsole->GetEngineCommand()->r_BloomPower;
		UpdateRareConstantsBuffer();
	}
	else if (pCVar->mName == "r_starpower")
	{
		mStarPower = gFBEnv->pConsole->GetEngineCommand()->r_StarPower;
		UpdateRareConstantsBuffer();
	}
	else if (pCVar->mName == "r_shadowmapwidth" ||
		pCVar->mName == "r_shadowmapheight")
	{
		for (auto it : mSwapChainRenderTargets)
		{
			it.second->DeleteShadowMap();
		}
	}
	else if (pCVar->mName == "r_shadowcamwidth" )
	{
		for (auto it : mSwapChainRenderTargets)
		{
			it.second->SetLightCamWidth(pCVar->GetFloat());
		}
	}
	else if (pCVar->mName == "r_shadowcamheight")
	{
		for (auto it : mSwapChainRenderTargets)
		{
			it.second->SetLightCamHeight(pCVar->GetFloat());
		}
	}
	else if (pCVar->mName == "r_shadownear")
	{
		for (auto it : mSwapChainRenderTargets)
		{
			it.second->SetLightCamNear(pCVar->GetFloat());
		}
	}
	else if (pCVar->mName == "r_shadowfar")
	{
		for (auto it : mSwapChainRenderTargets)
		{
			it.second->SetLightCamFar(pCVar->GetFloat());
		}
	}

	return false;
}

void Renderer::UpdateCloud(float dt)
{
	assert(mCloudManager);
	mCloudManager->Update();
}

void Renderer::Update(float dt)
{
	if (mNextEnvUpdateSkySphere)
	{
		mNextEnvUpdateSkySphere->UpdateEnvironmentMap(Vec3(0, 0, 0));
		mNextEnvUpdateSkySphere = 0;
	}
		

	UpdateLights(dt);
}

void Renderer::UpdateLights(float dt)
{
	for (int i = 0; i < 2; i++)
		mDirectionalLight[i]->Update(dt);

	mPointLightMan->Update(dt);
	// good point to reset.
	mRefreshPointLight = false;
}

ITexture* Renderer::FindRenderTarget(const Vec2I& size)
{
	assert(0);

	return 0;
}

IShader* Renderer::GetGodRayPS()
{
	if (!mGodRayPS)
	{
		mGodRayPS = CreateShader("es/shaders/GodRayPS.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
		mOccPrePassShader = CreateShader("es/shaders/OccPrePass.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
		mOccPrePassGSShader = CreateShader("es/shaders/OccPrePassGS.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS | BINDING_SHADER_GS,
			IMaterial::SHADER_DEFINES());
	}
	return mGodRayPS;
}

//---------------------------------------------------------------------------
IShader* Renderer::GetGlowShader()
{
	if (!mGlowPS)
	{
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mGlowPS = CreateShader("es/shaders/BloomPS.hlsl", BINDING_SHADER_PS, shaderDefines);
		FB_SET_DEVICE_DEBUG_NAME(mGlowPS, "GlowPS");
	}
	return mGlowPS;
}

//---------------------------------------------------------------------------
void Renderer::SetDepthWriteShader()
{
	if (!mDepthWriteShader)
	{
		mDepthWriteShader = CreateShader("es/shaders/depth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
		if (!mPositionInputLayout)
			mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mDepthWriteShader);
	}
	mPositionInputLayout->Bind();
	mDepthWriteShader->Bind();	
}

void Renderer::SetDepthWriteShaderCloud()
{
	if (!mCloudDepthWriteShader)
	{
		mCloudDepthWriteShader = CreateShader("es/shaders/depth_cloud.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
		if (!mPositionInputLayout)
			mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mCloudDepthWriteShader);
	}
	mPositionInputLayout->Bind();
	mCloudDepthWriteShader->Bind();
}

void Renderer::SetDepthOnlyShader(){
	if (!mDepthOnlyShader){
		mDepthOnlyShader = CreateShader("es/shaders/DepthOnly.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
		if (!mPositionInputLayout)
			mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mDepthOnlyShader);
	}
	mPositionInputLayout->Bind();
	mDepthOnlyShader->Bind();
}

//---------------------------------------------------------------------------
ITexture* Renderer::GetTemporalDepthBuffer(const Vec2I& size)
{
	auto it = mTempDepthBuffers.Find(size);
	if (it == mTempDepthBuffers.end())
	{
		auto depthBuffer = CreateTexture(0, size.x, size.y, PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);
		mTempDepthBuffers.Insert(std::make_pair(size, depthBuffer));
		return depthBuffer;
	}
	return it->second;
	
}

//---------------------------------------------------------------------------
void Renderer::SetOccPreShader()
{
	assert(mOccPrePassShader);
	if (mOccPrePassShader)
		mOccPrePassShader->Bind();
}
void Renderer::SetOccPreGSShader()
{
	assert(mOccPrePassGSShader);
	if (mOccPrePassGSShader)
		mOccPrePassGSShader->Bind();
}

void Renderer::SetPositionInputLayout()
{
	if (!mPositionInputLayout)
	{
		assert(mShadowMapShader);
		mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mShadowMapShader);
	}
	mPositionInputLayout->Bind();
}

void Renderer::UpdateEnvMapInNextFrame(ISkySphere* sky)
{
	mNextEnvUpdateSkySphere = sky;
}

void Renderer::InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds)
{
	assert(mCloudManager);
	mCloudManager->InitCloud(numThreads, numCloud, clouds);
}

void Renderer::CleanCloud()
{
	mCloudManager->CleanCloud();
}

void Renderer::BindDepthTexture(bool set)
{
	auto mainRenderTarget = (RenderTarget*)GetMainRenderTarget();
	if (mainRenderTarget)
	{
		mainRenderTarget->BindDepthTexture(set);
	}
}

//void Renderer::SetCloudVolumeTarget()
//{
//	if (!mCloudVolumeDepth)
//	{
//		mCloudVolumeDepth = CreateTexture(0, mWidth / 2, mHeight / 2, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
//			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
//		/*SAMPLER_DESC sdesc;
//		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
//		mCloudVolumeDepth->SetSamplerDesc(sdesc);*/
//	}
//	assert(mTempDepthBufferHalf);
//	ITexture* rts[] = { mCloudVolumeDepth };
//	size_t index[] = { 0 };
//	// mTempDepthBufferHalf already filled with scene objects. while writing the depth texture;
//	SetRenderTarget(rts, index, 1, mTempDepthBufferHalf, 0);
//	Viewport vp = { 0, 0, mWidth*.5f, mHeight*.5f, 0.f, 1.f };
//	SetViewports(&vp, 1);
//	Clear(0.0f, 0.0f, 0.0f, 0.f, 1, 0);
//}
//
//void Renderer::SetCloudVolumeTexture(bool set)
//{
//	if (set)
//		SetTexture(mCloudVolumeDepth, BINDING_SHADER_PS, 6);
//	else
//		SetTexture(0, BINDING_SHADER_PS, 6);
//}


void Renderer::LockBlendState()
{
	mLockBlendState = true;
}
void Renderer::UnlockBlendState(){
	mLockBlendState = false;
}

void Renderer::BindNoiseMap()
{
	if (!mNoiseMap)
	{
		mNoiseMap = CreateTexture("es/textures/pnoise.dds");
		mNoiseMap->SetShaderStage(BINDING_SHADER_PS);
		mNoiseMap->SetSlot(7);
	}
	mNoiseMap->Bind();
}

void Renderer::SetShadowMapShader()
{
	if (!mShadowMapShader)
	{
		mShadowMapShader = CreateShader("es/shaders/shadowdepth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
	}
	mShadowMapShader->Bind();
}

IShader* Renderer::GetSilouetteShader()
{
	if (!mSilouetteShader)
	{
		mSilouetteShader = CreateShader("es/shaders/silouette.hlsl", BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
	}
	return mSilouetteShader;
}

IShader* Renderer::GetCopyPS()
{
	assert(mCopyPS);
	return mCopyPS;
}
IShader* Renderer::GetCopyPSMS()
{
	assert(mCopyPSMS);
	return mCopyPSMS;
}

ITexture* Renderer::GetToneMap(unsigned idx)
{
	if (mToneMap[0] == 0)
	{
		int nSampleLen = 1;
		for (int i = 0; i < FB_NUM_TONEMAP_TEXTURES_NEW; i++)
		{
			// 1, 3, 9, 27, 81
			mToneMap[i] = CreateTexture(0, nSampleLen, nSampleLen, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			char buff[255];
			sprintf_s(buff, "ToneMap(%d)", nSampleLen);
			FB_SET_DEVICE_DEBUG_NAME(mToneMap[i], buff);
			nSampleLen *= 3;
		}
		for (int i = 0; i < FB_NUM_LUMINANCE_TEXTURES; i++)
		{
			mLuminanceMap[i] = CreateTexture(0, 1, 1, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		ITexture* textures[] = { mLuminanceMap[0], mLuminanceMap[1] };
		size_t index[] = { 0, 0 };
		SetRenderTarget(textures, index, 2, 0, 0);
		Clear(0, 0, 0, 1);

		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}

		mSampleLumInitialShader = CreateShader("es/shaders/SampleLumInitialNew.hlsl", BINDING_SHADER_PS, shaderDefines);
		mSampleLumIterativeShader = CreateShader("es/shaders/SampleLumIterativeNew.hlsl", BINDING_SHADER_PS);
		mSampleLumFinalShader = CreateShader("es/shaders/SampleLumFinalNew.hlsl", BINDING_SHADER_PS);
		mCalcAdaptedLumShader = CreateShader("es/shaders/CalculateAdaptedLum.hlsl", BINDING_SHADER_PS);
	}

	assert(idx < FB_NUM_TONEMAP_TEXTURES_NEW);
	assert(mToneMap[idx] != 0);
	return mToneMap[idx];
}

IShader* Renderer::GetSampleLumInitialShader()
{
	assert(mSampleLumInitialShader);
	return mSampleLumInitialShader;
}

IShader* Renderer::GetSampleLumIterativeShader()
{
	assert(mSampleLumIterativeShader);
	return mSampleLumIterativeShader;
}

IShader* Renderer::GetSampleLumFinalShader()
{
	assert(mSampleLumFinalShader);
	return mSampleLumFinalShader;
}

void Renderer::SwapLuminanceMap()
{
	std::swap(mLuminanceMap[0], mLuminanceMap[1]);
}

ITexture* Renderer::GetLuminanceMap(unsigned idx)
{
	assert(idx < FB_NUM_LUMINANCE_TEXTURES);
	return mLuminanceMap[idx];
}

IShader* Renderer::GetCalcAdapedLumShader()
{
	assert(mCalcAdaptedLumShader);
	return mCalcAdaptedLumShader;
}

IShader* Renderer::GetBrightPassPS()
{
	if (!mBrightPassPS)
	{
		const char* bpPath = "es/shaders/brightpassps.hlsl";
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mBrightPassPS = CreateShader(bpPath, BINDING_SHADER_PS, shaderDefines);
	}
	assert(mBrightPassPS);
	return mBrightPassPS;
}

IShader* Renderer::GetBlur5x5PS()
{
	if (!mBlur5x5)
	{
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mBlur5x5 = CreateShader("es/shaders/gaussblur5x5.hlsl", BINDING_SHADER_PS, shaderDefines);
	}
	return mBlur5x5;
}

IShader* Renderer::GetBloomPS()
{
	if (!mBloomPS)
	{
		const char* blPath = "es/shaders/bloomps.hlsl";
		mBloomPS = CreateShader(blPath, BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
	}
	return mBloomPS;
}

IShader* Renderer::GetStarGlareShader()
{
	if (!mStarGlareShader)
		mStarGlareShader = CreateShader("es/shaders/starglare.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());

	return mStarGlareShader;
}

IShader* Renderer::GetMergeTexturePS()
{
	if (!mMergeTexture2)
		mMergeTexture2 = CreateShader("es/shaders/mergetextures2ps.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
	return mMergeTexture2;
}

IShader* Renderer::GetToneMappingPS()
{
	if (!mToneMappingPS)
		CreateToneMappingShader();
	return mToneMappingPS;
}

void Renderer::Render(float dt)
{
	InitFrameProfiler(dt);
	UpdateFrameConstantsBuffer();

	ProcessRenderTarget();
	Render3DUIsToTexture();
	auto mainRT = GetMainRenderTarget();
	for (auto it : mSwapChainRenderTargets)
	{
		D3DEventMarker mark(FormatString("Processing render target for %u", it.first));
		auto hwndId = it.first;
		auto rt = (RenderTarget*)it.second;
		assert(rt);
		bool rendered = rt->Render();
		if (rendered) {
			if (rt == mainRT) {
				RenderMarks();
			}

			for (auto l : mRenderListeners)	{
				l->BeforeUIRendering(hwndId);
			}
			RenderUI(hwndId);

			for (auto l : mRenderListeners){
				l->AfterUIRendered(hwndId);
			}
		}
	}
	mainRT->BindTargetOnly(false);
	RenderDebugHud();
	RenderDebugRenderTargets();
	RenderFade();	
}

void Renderer::RenderDebugRenderTargets()
{
	auto rt = GetMainRenderTarget();
	assert(rt);
	const auto& size = rt->GetSize();
	for (int i = 0; i < MaxDebugRenderTargets; i++)
	{
		if (mDebugRenderTargets[i].mTexture)
		{
			Vec2 pixelPos = mDebugRenderTargets[i].mPos * Vec2((float)size.x, (float)size.y);
			Vec2 pixelSize = mDebugRenderTargets[i].mSize * Vec2((float)size.x, (float)size.y);
			DrawQuadWithTexture(Round(pixelPos), Round(pixelSize), Color(1, 1, 1, 1),
				mDebugRenderTargets[i].mTexture);
		}
	}
}
void Renderer::SetDebugRenderTarget(unsigned idx, const char* textureName)
{
	assert(idx < MaxDebugRenderTargets);
	auto mainRT = (RenderTarget*)GetMainRenderTarget();
	assert(mainRT);
	if (stricmp(textureName, "Shadow") == 0)
		mDebugRenderTargets[idx].mTexture = mainRT->GetShadowMap();
	else
		mDebugRenderTargets[idx].mTexture = 0;
}

IRenderTarget* Renderer::CreateRenderTarget(const RenderTargetParam& param)
{
	if (param.mUsePool)
	{
		for (auto it = mRenderTargetPool.begin(); it != mRenderTargetPool.end(); it++)
		{
			if ((*it)->CheckOptions(param))
			{
				if (param.mEveryFrame)
					mRenderTargets.push_back(*it);
				IRenderTarget* rt = *it;
				mRenderTargetPool.erase(it);
				return rt;
			}

		}
	}
	return 0;
}

void Renderer::DeleteRenderTarget(IRenderTarget* rt)
{
	if (!rt)
		return;
	if (rt->GetUsePool())
	{
		if (ValueNotExistInVector(mRenderTargetPool, rt))
		{
			mRenderTargetPool.push_back(rt);
		}
	}
	else
	{
		FB_DELETE(rt);
	}
}

void Renderer::GatherPointLightData(BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst)
{
	assert(plConst);
	mPointLightMan->GatherPointLightData(aabb, transform, plConst);
}

void Renderer::RefreshPointLight()
{
	mRefreshPointLight = true;
}

IPointLight* Renderer::CreatePointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime,
	bool manualDeletion)
{
	assert(mPointLightMan);
	RefreshPointLight();
	return mPointLightMan->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
}

void Renderer::DeletePointLight(IPointLight* pointLight)
{
	assert(mPointLightMan);
	mPointLightMan->DeletePointLight(pointLight);
}



void Renderer::ToneMapLuminanceOnCpu(bool oncpu)
{
	if (mLuminanceOnCpu == oncpu)
		return;

	mLuminanceOnCpu = oncpu;

	CreateToneMappingShader();
}

void Renderer::UseFilmicToneMapping(bool filmic)
{
	if (mUseFilmicToneMapping == filmic)
		return;
	mUseFilmicToneMapping = filmic;
	CreateToneMappingShader();
}

void Renderer::CreateToneMappingShader()
{
	IMaterial::SHADER_DEFINES shaderDefines;
	if (GetMultiSampleCount() != 1)
		shaderDefines.push_back(IMaterial::ShaderDefine("_MULTI_SAMPLE", "1"));

	if (mLuminanceOnCpu)
		shaderDefines.push_back(IMaterial::ShaderDefine("_CPU_LUMINANCE", "1"));

	if (mUseFilmicToneMapping)
		shaderDefines.push_back(IMaterial::ShaderDefine("_FILMIC_TONEMAP", "1"));

	mToneMappingPS = CreateShader("es/shaders/tonemapping.hlsl", BINDING_SHADER_PS, shaderDefines);
}

void Renderer::SetFadeAlpha(float alpha)
{
	mFadeAlpha = alpha;
}

void Renderer::RenderFade()
{
	if (mFadeAlpha <= 0)
		return;
	auto mainRT = GetMainRenderTarget();
	assert(mainRT);
	DrawQuad(Vec2I(0, 0), mainRT->GetSize(), Color(0, 0, 0, mFadeAlpha));	
}

IMaterial* Renderer::GetMaterial(DEFAULT_MATERIALS::Enum type)
{
	assert(type < DEFAULT_MATERIALS::COUNT);
	return mMaterials[type];
}


void Renderer::ProcessInputData()
{
	for(auto it : mSwapChainRenderTargets)
	{
		it.second->GetCamera()->ProcessInputData();
	}
}

void Renderer::OnInputFromEngineForCamera(IMouse* mouse, IKeyboard* keyboard)
{
	for (auto it : mSwapChainRenderTargets)
	{
		it.second->GetCamera()->OnInputFromEngine(mouse, keyboard);
	}
}

int Renderer::CropSize8(int size) const
{
	return size - size % 8;
}

void Renderer::Render3DUIsToTexture()
{
	if (!m3DUIEnabled)
		return;

	D3DEventMarker mark("Render3DUIsToTexture");
	for (auto scIt : mSwapChainRenderTargets) {
		for (auto rtIt : mUI3DObjectsRTs) {
			if (!rtIt.second->GetEnable())
				continue;
			auto& uiObjectsIt = mUI3DObjects.Find(std::make_pair(scIt.first, rtIt.first));
			if (uiObjectsIt != mUI3DObjects.end()){
				auto& uiObjects = uiObjectsIt->second;				
				auto& rt = rtIt.second;
				rt->Bind();

				for (auto& uiobj : uiObjects)
				{
					uiobj->PreRender();
					uiobj->Render();
					uiobj->PostRender();
				}

				rt->Unbind();
				rt->GetRenderTargetTexture()->GenerateMips();
			}
		}
	}
}

//--------------------------------------------------------------------------
void Renderer::RegisterUIs(HWND_ID hwndId, std::vector<IUIObject*>& uiobj)
{
	auto& objectsToRender = mUIObjectsToRender[hwndId];
	objectsToRender.swap(uiobj);
}

void Renderer::UnregisterUIs(HWND_ID hwndId)
{
	auto& objectsToRender = mUIObjectsToRender[hwndId];
	objectsToRender.clear();
}

void Renderer::Register3DUIs(HWND_ID hwndId, const char* name, std::vector<IUIObject*>& objects)
{
	assert(!objects.empty());

	auto it = mUI3DObjectsRTs.Find(name);
	if (it == mUI3DObjectsRTs.end())
	{
		const Vec2I& rtSize = objects[0]->GetRenderTargetSize();
		RenderTargetParam param;
		param.mEveryFrame = false;
		param.mSize = rtSize;
		param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
		param.mShaderResourceView = true;
		param.mMipmap = true;
		param.mCubemap = false;
		param.mWillCreateDepth = false;
		param.mUsePool = true;
		auto rtt = gFBEnv->pRenderer->CreateRenderTarget(param);
		assert(rtt);
		mUI3DObjectsRTs.Insert(std::make_pair(std::string(name), rtt));
		assert(mUI3DRenderObjs.Find(name) == mUI3DRenderObjs.end());
		auto renderObj = FB_NEW(UI3DObj);
		mUI3DRenderObjs.Insert(std::make_pair(std::string(name), renderObj));
		renderObj->SetTexture(rtt->GetRenderTargetTexture());
		renderObj->AttachToScene();
	}
	else
	{
		assert(it->second->GetSize() == objects[0]->GetRenderTargetSize());
		if (!it->second->GetEnable())
		{
			it->second->SetEnable(true);
			auto it2 = mUI3DRenderObjs.Find(name);
			if_assert_pass(it2 != mUI3DRenderObjs.end())
			{
				it2->second->ModifyObjFlag(IObject::OF_HIDE, false);
			}
		}
	}
	mUI3DObjects[std::make_pair(hwndId, name)].swap(objects);
}

// the IWinbase is deleted.
void Renderer::Unregister3DUIs(const char* name)
{
	auto it = mUI3DRenderObjs.Find(name);
	if (it != mUI3DRenderObjs.end())
	{
		FB_DELETE(it->second);
		mUI3DRenderObjs.erase(it);
	}

	auto it2 = mUI3DObjectsRTs.Find(name);
	if (it2 != mUI3DObjectsRTs.end())
	{
		it2->second->SetEnable(false);
	}
}

void Renderer::Set3DUIPosSize(const char* name, const Vec3& pos, const Vec2& sizeInWorld)
{
	auto it = mUI3DRenderObjs.Find(name);
	if (it != mUI3DRenderObjs.end())
	{
		it->second->SetPosSize(pos, sizeInWorld);
	}
}

void Renderer::Reset3DUI(const char* name)
{
	auto it = mUI3DRenderObjs.Find(name);
	if (it != mUI3DRenderObjs.end())
	{
		it->second->Reset3DUI();
	}
}

void Renderer::SetEnable3DUIs(bool enable)
{
	m3DUIEnabled = enable;
}

//------------------------------------------------------------------------
void Renderer::AddMarkObject(IObject* mark)
{
	if (ValueNotExistInVector(mMarkObjects, mark))
		mMarkObjects.push_back(mark);
}

//------------------------------------------------------------------------
void Renderer::RemoveMarkObject(IObject* mark)
{
	DeleteValuesInVector(mMarkObjects, mark);
}

//------------------------------------------------------------------------
void Renderer::AddHPBarObject(IObject* hpBar)
{
	if (ValueNotExistInVector(mHPBarObjects, hpBar))
		mHPBarObjects.push_back(hpBar);
}

//------------------------------------------------------------------------
void Renderer::RemoveHPBarObject(IObject* hpBar)
{
	DeleteValuesInVector(mHPBarObjects, hpBar);
}

//----------------------------------------------------------------------------
void Renderer::RenderMarks()
{
	D3DEventMarker mark("Render Marks / HPBar");
	FB_FOREACH(it, mMarkObjects)
	{
		(*it)->PreRender();
		(*it)->Render();
		(*it)->PostRender();
	}

	for (auto it : mHPBarObjects)
	{
		it->PreRender();
		it->Render();
		it->PostRender();
	}
}

//----------------------------------------------------------------------------
void Renderer::RenderUI(HWND_ID hwndId)
{
	D3DEventMarker mark("RenderUI");
	auto& uiobjects = mUIObjectsToRender[hwndId];
	auto it = uiobjects.begin(), itEnd = uiobjects.end();
	for (; it != itEnd; it++)
	{
		(*it)->PreRender(); // temporary :)
		(*it)->Render();
		(*it)->PostRender();
	}
}

//----------------------------------------------------------------------------
void Renderer::RenderFrameProfiler()
{
	wchar_t msg[255];
	int x = 1100;
	int y = 34;
	int yStep = 18;
	if (mFont)
		yStep = (int)mFont->GetHeight();

	const RENDERER_FRAME_PROFILER& profiler = mFrameProfiler;

	swprintf_s(msg, 255, L"FrameRate = %.0f", profiler.FrameRateDisplay);
	DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Num draw calls = %d", profiler.NumDrawCall);
	DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Num vertices = %d", profiler.NumVertexCount);
	DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep * 2;

	swprintf_s(msg, 255, L"Num Particles = %d", ParticleManager::GetParticleManager().GetNumActiveParticles());
	DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	auto pman = gFBEnv->pRenderer->GetPointLightMan();
	if (pman)
	{
		swprintf_s(msg, 255, L"Num PointLights = %d", pman->GetNumPointLights());
		DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;
	}
	
	auto scene = GetMainScene();
	if (scene)
	{
		unsigned num = scene->GetNumSpatialObjects();
		swprintf_s(msg, L"Num spatial objects = %d", num);
		DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;
	}

	/*swprintf_s(msg, 255, L"Num draw indexed calls = %d", profiler.NumDrawIndexedCall);
	mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y+= yStep;

	swprintf_s(msg, 255, L"Num draw indices = %d", profiler.NumIndexCount);
	mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));		*/
}


void Renderer::OnRenderTargetDeleted(RenderTarget* renderTarget)
{
	if (mCurRenderTarget == renderTarget)
	{
		mCurRenderTarget = GetMainRenderTarget();
		mCamera = mCurRenderTarget->GetCamera();
		for (int i = 0; i < 2; i++)
			mDirectionalLight[i] = mCurRenderTarget->GetScene()->GetLight(i);
	}
}

void Renderer::SetScene(IScene* scene)
{
	mCurProcessingScene = scene;
	UpdateSceneConstantsBuffer();
}

void Renderer::AddRenderListener(IRenderListener* listener)
{
	if (ValueNotExistInVector(mRenderListeners, listener))
		mRenderListeners.push_back(listener);
}

void Renderer::RemoveRenderListener(IRenderListener* listener)
{
	DeleteValuesInVector(mRenderListeners, listener);
}

}