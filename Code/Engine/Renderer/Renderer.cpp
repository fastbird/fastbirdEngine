#include <Engine/StdAfx.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/Font.h>
#include <Engine/Renderer/Material.h>
#include <Engine/RenderObjects/DebugHud.h>
#include <Engine/Renderer/RenderToTexture.h>
#include <Engine/ILight.h>
#include <Engine/RenderObjects/SkySphere.h>
#include <Engine/IConsole.h>
#include <Engine/RenderObjects/MeshObject.h>
#include <Engine/IScriptSystem.h>
#include <Engine/Renderer/VolumetricCloud.h>
#include <Engine/Renderer/CloudManager.h>
#include <CommonLib/Unicode.h>

namespace fastbird
{

	extern STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES];
	extern int                      s_nLibStarDefs;
//----------------------------------------------------------------------------
	Renderer::Renderer()
		: DEFAULT_DYN_VERTEX_COUNTS(100)
		, mWidth(0)
		, mHeight(0)
		, mCropWidth(0)
		, mCropHeight(0)
		, mClearColor(0.0f, 0.0f, 0.0f)
		, mDepthClear(1.f)
		, mStencilClear(0)
		, mCamera(0)
		, mDepthStencilCreated(false)
		, mForcedWireframe(false)
		, mLuminanceIndex(-1)
		, mGaussianDistCalculated(false)
		, mGaussianDistGlowCalculated(false)
		, mNextEnvUpdateSkySphere(0)
		, mLockBlendState(false)
		, mLockDepthStencil(false)
		, mCloudRendering(false)
		, mCameraBackup(0)
		, mGaussianDownScale2x2Calculated(false)
		, m_fChromaticAberration(0.5f)
		, m_fStarInclination(HALF_PI)
		, mCurRTSize(100, 100)
{
	assert(gFBEnv->pConsole);
	gFBEnv->pConsole->AddListener(this);
	mCloudManager = FB_NEW(CloudManager);

	for (int i = 0; i < SAMPLERS::NUM; ++i)
	{
		mDefaultSamplers[i] = 0;
	}
	
	StarDef::InitializeStatic();
	mStarDef.Initialize(STLT_VERTICAL);	

	float y = 0.0f;
	for (int i = 0; i < MaxDebugRenderTargets; i++)
	{
		mDebugRenderTargets[i].mPos = Vec2(0, y);
		mDebugRenderTargets[i].mSize = Vec2(0.25, 0.24f);
		y += 0.25f;
	}
}
Renderer::~Renderer()
{
	StarDef::FinalizeStatic();
	if (gFBEnv->pConsole)
		gFBEnv->pConsole->RemoveListener(this);
}

// called from inherited classes.
void Renderer::Deinit()
{
	mMergeTexture2 = 0;
	mStarGlareShader = 0;
	mStarSourceTex = 0;
	for (int i = 0; i < FB_NUM_STAR_TEXTURES; ++i)
	{
		mStarTextures[i] = 0;
	}
	mBlur5x5 = 0;
	mTempDepthBuffer = 0;
	mTempDepthBufferHalf = 0;
	mBigSilouetteBuffer = 0;
	mSmallSilouetteBuffer = 0;
	mSilouetteShader = 0;
	mShadowMapShader = 0;
	mShadowMap = 0;
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
	FB_SAFE_DEL(mCloudManager);
	// All release operation should be done here.
	MeshObject::ClearHighlightMesh();
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
	mDirectionalLightOverride[0] = 0;
	mDirectionalLightOverride[1] = 0;
	mDebugHud = 0;
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

	mRenderToTextures.clear();
	mInputLayouts.clear();

	for (int i = 0; i < DEFAULT_INPUTS::COUNT; i++)
	{
		mDynVBs[i] = 0;
	}

	mHDRTarget = 0;

	for (auto rt : mRenderToTexturePool)
	{
		FB_DELETE(rt);
	}
	mRenderToTexturePool.clear();
}

void Renderer::CleanDepthWriteResources()
{
	mMinBlendState = 0;
	mDepthWriteShader = 0;
	mDepthTarget = 0;
	mPositionInputLayout = 0;
}

void Renderer::CleanGlowResources()
{
	mGlowTarget = 0;
	mGlowTexture[0] = 0;
	mGlowTexture[1] = 0;
	mGlowPS = 0;
}

void Renderer::CleanHDRResources()
{
	mHDRTarget = 0;
	for (int i = 0; i < FB_NUM_TONEMAP_TEXTURES; ++i)
		mToneMap[i] = 0;
	for (int i = 0; i < FB_NUM_LUMINANCE_TEXTURES; ++i)
		mLuminanceMap[i] = 0;
	mLuminanceIndex = -1;
	mDownScale2x2LumPS = 0;
	mDownScale3x3PS = 0;
	mToneMappingPS = 0;
	mLuminanceAvgPS = 0;
	mBrightPassTexture = 0;
	for (int i = 0; i < FB_NUM_BLOOM_TEXTURES; ++i)
	{
		mBloomTexture[i] = 0;
	}
	mBrightPassPS = 0;
	mBloomPS = 0;
	mBloomSourceTex = 0;
	mDownScale2x2PS = 0;
}

void Renderer::CleanGodRayResources()
{
	gFBEnv->mGodRayInScreen = false;
	for (int i = 0; i < 2; ++i)
		mGodRayTarget[i] = 0;
	mGodRayPS = 0;
	mNoMSDepthStencil = 0;
	mOccPrePassGSShader = 0;
	mOccPrePassShader = 0;
	mOccPrePassShader = 0;
}

//----------------------------------------------------------------------------
bool Renderer::OnPrepared()
{
	mCurRTSize.x = GetWidth();
	mCurRTSize.y = GetHeight();

	// Light
	for (int i = 0; i < 2; ++i)
	{
		mDirectionalLight[i] = ILight::CreateLight(ILight::LIGHT_TYPE_DIRECTIONAL);
		mDirectionalLight[i]->SetIntensity(1.0f);
	}

	mDirectionalLight[0]->SetPosition(Vec3(-3, 1, 1));
	mDirectionalLight[0]->SetDiffuse(Vec3(1, 1, 1));
	mDirectionalLight[0]->SetSpecular(Vec3(1, 1, 1));

	mDirectionalLight[1]->SetPosition(Vec3(3, 1, -1));
	mDirectionalLight[1]->SetDiffuse(Vec3(0.8f, 0.4f, 0.1f));
	mDirectionalLight[1]->SetSpecular(Vec3(0, 0, 0));


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
	std::string fontName = gFBEnv->pEngine->GetConfigStringValue("ETC", "Font");
	if (fontName.empty())
	{
		fontName = "es/fonts/nanum_pen_bin.fnt";
	}
	mFont->Init(fontName.c_str());
	if (mFont)
		mFont->SetRenderTargetSize(mCurRTSize);
	mFont->SetTextEncoding(IFont::UTF16);

	mDebugHud = FB_NEW(DebugHud);

	if (gFBEnv->pConsole)
		gFBEnv->pConsole->Init();

	UpdateRareConstantsBuffer();

	mDefaultRasterizerState = CreateRasterizerState(RASTERIZER_DESC());
	mDefaultBlendState = CreateBlendState(BLEND_DESC());
	mDefaultDepthStencilState = CreateDepthStencilState(DEPTH_STENCIL_DESC());
	DEPTH_STENCIL_DESC ddesc;
	ddesc.DepthEnable = false;
	ddesc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	mNoDepthStencilState = CreateDepthStencilState(ddesc);


	IMaterial::SHADER_DEFINES emptyShaderDefines;
	mFullscreenQuadVSNear = CreateShader("code/engine/renderer/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS,
		emptyShaderDefines);
	IMaterial::SHADER_DEFINES shaderDefinesFar;
	shaderDefinesFar.push_back(IMaterial::ShaderDefine("_FAR_SIDE_QUAD", "1"));
	mFullscreenQuadVSFar = CreateShader("code/engine/renderer/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS,
		shaderDefinesFar);

	mCopyPS = CreateShader("code/engine/renderer/shaders/copyps.hlsl", BINDING_SHADER_PS,
		emptyShaderDefines);
	IMaterial::SHADER_DEFINES multiSampleSD;
	multiSampleSD.push_back(IMaterial::ShaderDefine("_MULTI_SAMPLE", "1"));
	mCopyPSMS = CreateShader("code/engine/renderer/shaders/copyps.hlsl", BINDING_SHADER_PS,
		multiSampleSD);

	SkySphere::CreateSharedEnvRT();

	SAMPLER_DESC sdesc;
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
	mDefaultSamplers[SAMPLERS::POINT] = CreateSamplerState(sdesc);
	sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
	mDefaultSamplers[SAMPLERS::LINEAR] = CreateSamplerState(sdesc);
	sdesc.Filter = TEXTURE_FILTER_ANISOTROPIC;
	mDefaultSamplers[SAMPLERS::ANISOTROPIC] = CreateSamplerState(sdesc);

	sdesc.Filter = TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sdesc.AddressU = TEXTURE_ADDRESS_BORDER;
	sdesc.AddressV = TEXTURE_ADDRESS_BORDER;
	sdesc.AddressW = TEXTURE_ADDRESS_BORDER;
	for (int i = 0; i < 4; i++)
		sdesc.BorderColor[i] = 1.0f;
	sdesc.ComparisonFunc = COMPARISON_LESS;
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

	mTempDepthBufferHalf = CreateTexture(0, mWidth / 2, mHeight / 2, PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
		BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);
	mTempDepthBuffer = CreateTexture(0, mWidth, mHeight, PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
		BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);

	mMiddleGray = gFBEnv->pConsole->GetEngineCommand()->r_HDRMiddleGray;
	mStarPower = gFBEnv->pConsole->GetEngineCommand()->r_StarPower;
	mBloomPower = gFBEnv->pConsole->GetEngineCommand()->r_BloomPower;

	return true;
}

//----------------------------------------------------------------------------
void Renderer::ProcessRenderToTexture()
{
	for (auto pRT : mRenderToTextures)
	{
		pRT->Render();
	}
}

//----------------------------------------------------------------------------
void Renderer::SetClearColor(float r, float g, float b, float a/*=1.f*/)
{
	mClearColor.SetColor(r, g, b, a);
}

//----------------------------------------------------------------------------
void Renderer::SetClearDepthStencil(float z, UINT8 stencil)
{
	mDepthClear = z;
	mStencilClear = stencil;
}

//----------------------------------------------------------------------------
void Renderer::SetCamera(ICamera* pCamera)
{
	ICamera* prev = mCamera;
	if (prev)
		prev->SetCurrent(false);
	mCamera = pCamera;
	mCamera->SetCurrent(true);
	if (prev !=0 && prev != mCamera)
		UpdateRareConstantsBuffer();
}

ICamera* Renderer::GetCamera() const
{
	return mCamera;
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
Vec2I Renderer::ToSreenPos(const Vec3& ndcPos) const
{
	Vec2I ret;
	ret.x = (int)(((float)mWidth*.5f) * ndcPos.x + mWidth*.5f);
	ret.y = (int)((-(float)mHeight*.5f) * ndcPos.y + mHeight*.5f);
	return ret;
}

Vec2 Renderer::ToNdcPos(const Vec2I& screenPos) const
{
	Vec2 ret;
	ret.x = (float)screenPos.x / (float)mWidth * 2.0f - 1.0f;
	ret.y = -((float)screenPos.y / (float)mHeight * 2.0f - 1.0f);
	return ret;
}

//----------------------------------------------------------------------------
void Renderer::DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color)
{
	mDebugHud->DrawTextForDuration(secs, pos, text, color);
}

void Renderer::DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
	const Color& color)
{
	DrawTextForDuration(secs, pos, AnsiToWide(text, strlen(text)), color);
}

void Renderer::DrawText(const Vec2I& pos, WCHAR* text, const Color& color)
{
	mDebugHud->DrawText(pos, text, color);
}

void Renderer::DrawText(const Vec2I& pos, const char* text, const Color& color)
{
	DrawText(pos, AnsiToWide(text, strlen(text)), color);
}

void Renderer::DrawLine(const Vec3& start, const Vec3& end, 
	const Color& color0, const Color& color1)
{
	mDebugHud->DrawLine(start, end, color0, color1);
}

void Renderer::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
	const Color& color0, const Color& color1)
{
	mDebugHud->DrawLineBeforeAlphaPass(start, end, color0, color1);
}

void Renderer::DrawLine(const Vec2I& start, const Vec2I& end, 
	const Color& color0, const Color& color1)
{
	mDebugHud->DrawLine(start, end, color0, color0);
}

void Renderer::RenderDebugHud()
{
	D3DEventMarker devent("RenderDebugHud");
	bool backup = GetWireframe();
	SetWireframe(false);
	mDebugHud->Render();
	SetWireframe(backup);
}

//-------------------------------------------------------------------------
inline IFont* Renderer::GetFont() const
{
	return mFont;
}

void Renderer::SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num,
	ITexture* pDepthStencil, size_t dsIndex)
{
	if (pRenderTargets && num>0 && pRenderTargets[0])
	{
		mCurRTSize = pRenderTargets[0]->GetSize();
	}
	else
	{
		mCurRTSize = Vec2I(mWidth, mHeight);
	}
	if (mFont)
		mFont->SetRenderTargetSize(mCurRTSize);
}

void Renderer::SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num)
{
	if (pRenderTargets && num>0)
	{
		mCurRTSize = pRenderTargets[0]->GetSize();
	}
	else
	{
		mCurRTSize = Vec2I(mWidth, mHeight);
	}
	if (mFont)
		mFont->SetRenderTargetSize(mCurRTSize);
}

const Vec2I& Renderer::GetRenderTargetSize() const
{
	return mCurRTSize;
}

void Renderer::RestoreRenderTarget()
{
	mCurRTSize = Vec2I(mWidth, mHeight);
	if (mFont)
		mFont->SetRenderTargetSize(mCurRTSize);
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
				Error("\t%s", errMsg);
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
	mDirectionalLightOverride[idx] = pLight;
}

ILight* Renderer::GetDirectionalLight(int idx) const
{
	return mDirectionalLightOverride[idx] ? mDirectionalLightOverride[idx] : mDirectionalLight[idx];
}

void Renderer::SetEnvironmentTexture(ITexture* pTexture)
{
	mEnvironmentTexture = pTexture;
	mEnvironmentTexture->SetShaderStage(BINDING_SHADER_PS);
	/*SAMPLER_DESC desc;
	desc.AddressU = TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = TEXTURE_ADDRESS_CLAMP;
	desc.Filter = TEXTURE_FILTER_ANISOTROPIC;
	mEnvironmentTexture->SetSamplerDesc(desc);*/
	mEnvironmentTexture->SetSlot(4); // hardcoded environment slot.
	mEnvironmentTexture->Bind();
}

//---------------------------------------------------------------------------
void Renderer::RestoreRasterizerState()
{
	mDefaultRasterizerState->Bind();
}

//---------------------------------------------------------------------------
void Renderer::RestoreBlendState()
{
	mDefaultBlendState->Bind();
}

//---------------------------------------------------------------------------
void Renderer::RestoreDepthStencilState()
{
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_ALPHA;
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_GREEN | COLOR_WRITE_ENABLE_ALPHA;
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_GREEN | COLOR_WRITE_ENABLE_ALPHA;

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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_GREEN | COLOR_WRITE_ENABLE_ALPHA;

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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_ALPHA;
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_GREEN | COLOR_WRITE_ENABLE_ALPHA;

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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_ALPHA;
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_GREEN;
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
		bdesc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_BLUE;
		mBlueMaskBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	}
	mBlueMaskBlend->Bind();
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

//---------------------------------------------------------------------------
void Renderer::SetSamplerState(SAMPLERS::Enum s, BINDING_SHADER shader, int slot)
{
	assert(s >= SAMPLERS::POINT && s < SAMPLERS::NUM);
	mDefaultSamplers[s]->Bind(shader, slot);
}

//---------------------------------------------------------------------------
void Renderer::SetHDRTarget()
{
	if (!mHDRTarget)
	{
		mHDRTarget = CreateTexture(0, mWidth, mHeight, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, 
			TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		if (!mHDRTarget)
		{
			Error("Cannot create HDR RenderTarget.");
			return;
		}
		/*SAMPLER_DESC desc;
		mHDRTarget->SetSamplerDesc(desc);*/
		FB_SET_DEVICE_DEBUG_NAME(mHDRTarget, "HDRTargetTexture");
	}

	ITexture* rts[] = { mHDRTarget };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1);
}
void Renderer::MeasureLuminanceOfHDRTarget()
{
	/*if (++mLuminanceIndex >= FB_NUM_LUMINANCE_TEXTURES)
		mLuminanceIndex = 0;*/

	mDefaultBlendState->Bind();
	mNoDepthStencilState->Bind(0);
	if (mToneMap[0] == 0)
	{
		int nSampleLen = 1;
		for (int i = 0; i < FB_NUM_TONEMAP_TEXTURES; i++)
		{
			// 1, 4, 16, 32
			int iSampleLen = 1 << (2 * i);

			mToneMap[i] = CreateTexture(0, iSampleLen, iSampleLen, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			char buff[255];
			sprintf_s(buff, "ToneMap(%d)", nSampleLen);
			FB_SET_DEVICE_DEBUG_NAME(mToneMap[i], buff);
		}
		for (int i = 0; i < FB_NUM_LUMINANCE_TEXTURES; i++)
		{
			mLuminanceMap[i] = CreateTexture(0, 1, 1, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}
		ITexture* textures[] = { mLuminanceMap[0], mLuminanceMap[1] };
		size_t index[] = { 0, 0 };
		SetRenderTarget(textures, index, 2, 0, 0);
		Clear();
		
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		/*mDownScale2x2LumPS = CreateShader("code/engine/renderer/shaders/downscale2x2_lum.hlsl", BINDING_SHADER_PS, shaderDefines);
		mDownScale3x3PS = CreateShader("code/engine/renderer/shaders/downscale3x3.hlsl", BINDING_SHADER_PS);
		mLuminanceAvgPS = CreateShader("code/engine/renderer/shaders/luminanceavgps.hlsl", BINDING_SHADER_PS);*/

		mSampleLumInitialShader = CreateShader("code/engine/renderer/shaders/SampleLumInitial.hlsl", BINDING_SHADER_PS, shaderDefines);
		mSampleLumIterativeShader = CreateShader("code/engine/renderer/shaders/SampleLumIterative.hlsl", BINDING_SHADER_PS);
		mSampleLumFinalShader = CreateShader("code/engine/renderer/shaders/SampleLumFinal.hlsl", BINDING_SHADER_PS);
		mCalcAdaptedLumShader = CreateShader("code/engine/renderer/shaders/CalculateAdaptedLum.hlsl", BINDING_SHADER_PS);
	}

	D3DEventMarker mark("Luminance");
	assert(mHDRTarget);
	DWORD dwCurTexture = FB_NUM_TONEMAP_TEXTURES - 1;
	ITexture* renderTarget = mToneMap[dwCurTexture];
	ITexture* rts[] = { renderTarget };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
	SetTexture(mHDRTarget, BINDING_SHADER_PS, 0);
	bool msaa = GetMultiSampleCount() > 1;
	if (msaa)
	{
		Vec4* pDest = (Vec4*)MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mHDRTarget->GetWidth();
			pDest->y = (float)mHDRTarget->GetHeight();
			UnmapMaterialParameterBuffer();
		}
	}
	
	const Vec2I& resol = renderTarget->GetSize();
	Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
	SetViewports(&vp, 1);
	DrawFullscreenQuad(mSampleLumInitialShader, false);
	--dwCurTexture;

	while (dwCurTexture>0)
	{
		ITexture* src = mToneMap[dwCurTexture+1];
		ITexture* renderTarget = mToneMap[dwCurTexture];

		ITexture* rts[] = { renderTarget };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		SetTexture(src, BINDING_SHADER_PS, 0);

		const Vec2I& resol = renderTarget->GetSize();
		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		SetViewports(&vp, 1);
		DrawFullscreenQuad(mSampleLumIterativeShader, false);
		--dwCurTexture;
	}

	// Perform the final pass of the average luminance calculation.
	{
		ITexture* src = mToneMap[dwCurTexture + 1];
		ITexture* renderTarget = mToneMap[dwCurTexture];
		ITexture* rts[] = { renderTarget };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		SetTexture(src, BINDING_SHADER_PS, 0);
		const Vec2I& resol = renderTarget->GetSize();
		{Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		SetViewports(&vp, 1); }

		DrawFullscreenQuad(mSampleLumFinalShader, false);
	}
	

	// AdaptedLum
	{
		std::swap(mLuminanceMap[0], mLuminanceMap[1]);
		ITexture* rts[] = { mLuminanceMap[0] };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;
		SetTexture(mLuminanceMap[1], BINDING_SHADER_PS, 0);
		SetTexture(mToneMap[0], BINDING_SHADER_PS, 1);
		
		Viewport vp = { 0, 0, (float)mLuminanceMap[0]->GetWidth(), (float)mLuminanceMap[0]->GetHeight(), 0.f, 1.f };
		SetViewports(&vp, 1);
		DrawFullscreenQuad(mCalcAdaptedLumShader, false);
	}


	// average exposure
	//{
	//	ITexture* rts[] = { mLuminanceMap[mLuminanceIndex] };
	//	size_t index[] = { 0 };
	//	SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;

	//	std::vector<ITexture*> rvs;
	//	rvs.reserve(FB_NUM_LUMINANCE_TEXTURES);
	//	rvs.push_back(mToneMap[0]);
	//	for (int i = 0; i < FB_NUM_LUMINANCE_TEXTURES; ++i)
	//	{
	//		if (i != mLuminanceIndex)
	//		{
	//			rvs.push_back(mLuminanceMap[i]);
	//		}
	//	}
	//	SetTextures(&rvs[0], rvs.size(), BINDING_SHADER_PS, 0);

	//	Viewport vp = { 0, 0, 1, 1, 0.f, 1.f };
	//	SetViewports(&vp, 1);
	//	DrawFullscreenQuad(mLuminanceAvgPS, false);
	//}

}

//---------------------------------------------------------------------------
void Renderer::BrightPass()
{
	if (!mBrightPassTexture)
	{

		mBrightPassTexture = CreateTexture(0, mCropWidth / 4, mCropHeight / 4, PIXEL_FORMAT_R8G8B8A8_UNORM,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		FB_SET_DEVICE_DEBUG_NAME(mBrightPassTexture, "BrightPass");

		const char* bpPath = "code/engine/renderer/shaders/brightpassps.hlsl";
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mBrightPassPS = CreateShader(bpPath, BINDING_SHADER_PS, shaderDefines);
	}

	const Vec2I& resol = mBrightPassTexture->GetSize();
	{
		D3DEventMarker mark("Bloom - BrightPass");
		// brightpass
		ITexture* rts[] = { mBrightPassTexture };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0); // no depth buffer;

		ITexture* rvs[] = { mHDRTarget, mLuminanceMap[0] };
		SetTextures(rvs, 2, BINDING_SHADER_PS, 0);

		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		SetViewports(&vp, 1);

		if (GetMultiSampleCount() != 1)
		{
			Vec4* pDest = (Vec4*)MapMaterialParameterBuffer();
			if (pDest)
			{
				pDest->x = (float)mHDRTarget->GetWidth();
				pDest->y = (float)mHDRTarget->GetHeight();
				UnmapMaterialParameterBuffer();
			}
		}

		DrawFullscreenQuad(mBrightPassPS, false);
	}
}

//---------------------------------------------------------------------------
void Renderer::BrightPassToStarSource()
{
	if (!mStarSourceTex)
	{
		mStarSourceTex = CreateTexture(0, mCropWidth / 4, mCropHeight / 4, PIXEL_FORMAT_R8G8B8A8_UNORM,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);

		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mBlur5x5 = CreateShader("code/engine/renderer/shaders/gaussblur5x5.hlsl", BINDING_SHADER_PS, shaderDefines);
	}

	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	GetSampleOffsets_GaussBlur5x5(mGlowTarget->GetWidth(), mGlowTarget->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4)* 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4)* 13);
	UnmapBigBuffer();

	ITexture* rts[] = { mStarSourceTex };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mStarSourceTex->GetWidth(), (float)mStarSourceTex->GetHeight(), 0.f, 1.f };
	SetViewports(&vp, 1);
	Clear();
	SetTexture(mGlowTarget, BINDING_SHADER_PS, 0);
	if (GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mGlowTarget->GetWidth();
			pDest->y = (float)mGlowTarget->GetHeight();
			UnmapMaterialParameterBuffer();
		}
	}

	DrawFullscreenQuad(mBlur5x5, false);
}

//---------------------------------------------------------------------------
void Renderer::StarSourceToBloomSource()
{
	if (!mBloomSourceTex)
	{
		mBloomSourceTex = CreateTexture(0, mCropWidth / 8, mCropHeight / 8, PIXEL_FORMAT_R8G8B8A8_UNORM, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		mGaussianDownScale2x2Calculated = false;
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mDownScale2x2PS = CreateShader("code/engine/renderer/shaders/downscale2x2.hlsl", BINDING_SHADER_PS, shaderDefines);
	}
	/*if (!mGaussianDownScale2x2Calculated)
	{
		mGaussianDownScale2x2Calculated = true;
		GetSampleOffsets_DownScale2x2(mStarSourceTex->GetWidth(), mStarSourceTex->GetHeight(), mGaussianOffsetsDownScale2x2);
	}
	BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
	memcpy(pData->gSampleOffsets, mGaussianOffsetsDownScale2x2, sizeof(Vec4)* 4);
	UnmapBigBuffer();*/

	assert(mBlur5x5);	

	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	GetSampleOffsets_GaussBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4)* 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4)* 13);
	UnmapBigBuffer();

	

	ITexture* rts[] = { mBloomSourceTex };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0);
	SetTexture(mBrightPassTexture, BINDING_SHADER_PS, 0);

	Viewport vp = { 0, 0, (float)mBloomSourceTex->GetWidth(), (float)mBloomSourceTex->GetHeight(), 0.f, 1.f };
	SetViewports(&vp, 1);

	if (GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mBrightPassTexture->GetWidth();
			pDest->y = (float)mBrightPassTexture->GetHeight();
			UnmapMaterialParameterBuffer();
		}
	}

	DrawFullscreenQuad(mBlur5x5, false);
}

//---------------------------------------------------------------------------
void Renderer::Bloom()
{
	if (!mBloomTexture[0])
	{
		for (int i = 0; i < FB_NUM_BLOOM_TEXTURES; i++)
		{
			mBloomTexture[i] = CreateTexture(0, mCropWidth / 8, mCropHeight / 8, PIXEL_FORMAT_R8G8B8A8_UNORM,
				BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			char buff[255];
			sprintf_s(buff, "Blood(%d)", i);
			FB_SET_DEVICE_DEBUG_NAME(mBloomTexture[i], buff);
		}
		
		const char* blPath = "code/engine/renderer/shaders/bloomps.hlsl";		
		mBloomPS = CreateShader(blPath, BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
	}	
	// blur
	D3DEventMarker mark("Bloom - Blur");
	assert(mBloomSourceTex);
	ITexture* rts[] = { mBloomTexture[2] };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mBloomTexture[2]->GetWidth(), (float)mBloomTexture[2]->GetHeight(), 0.f, 1.f };
	SetViewports(&vp, 1);

	SetTexture(mBloomSourceTex, BINDING_SHADER_PS, 0);

	Vec4* pOffsets = 0;
	Vec4* pWeights = 0;
	GetSampleOffsets_GaussBlur5x5(mBrightPassTexture->GetWidth(), mBrightPassTexture->GetHeight(),
		&pOffsets, &pWeights, 1.0f);
	assert(pOffsets && pWeights);

	BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
	memcpy(pData->gSampleOffsets, pOffsets, sizeof(Vec4)* 13);
	memcpy(pData->gSampleWeights, pWeights, sizeof(Vec4)* 13);
	UnmapBigBuffer();

	if (GetMultiSampleCount() != 1)
	{
		Vec4* pDest = (Vec4*)MapMaterialParameterBuffer();
		if (pDest)
		{
			pDest->x = (float)mBloomSourceTex->GetWidth();
			pDest->y = (float)mBloomSourceTex->GetHeight();
			UnmapMaterialParameterBuffer();
		}
	}

	DrawFullscreenQuad(mBlur5x5, false);




	const Vec2I& resol = mBloomTexture[2]->GetSize();
	// Horizontal Blur
	{
		D3DEventMarker mark("Bloom - Apply hori gaussian filter");
		BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		if (mGaussianDistCalculated)
		{
			memcpy(avSampleOffsets, mGaussianDistOffsetX, sizeof(Vec4)* 15);
			memcpy(avSampleWeights, mGaussianDistWeightX, sizeof(Vec4)* 15);
		}
		else
		{
			float afSampleOffsets[15];
			GetSampleOffsets_Bloom(resol.x, afSampleOffsets, mGaussianDistWeightX, 3.0f, 1.25f);
			for (int i = 0; i < 15; i++)
			{

				mGaussianDistOffsetX[i] = avSampleOffsets[i] = Vec4(afSampleOffsets[i], 0.0f, 0.0f, 0.0f);
			}
		}
		UnmapBigBuffer();
		ITexture* rts[] = { mBloomTexture[1] };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0);
		SetTexture(mBloomTexture[2], BINDING_SHADER_PS, 0);
		DrawFullscreenQuad(mBloomPS, false);
	}

	// Vertical Blur
	{
		D3DEventMarker mark("Bloom - Apply verti gaussian filter");
		BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		if (mGaussianDistCalculated)
		{
			memcpy(avSampleOffsets, mGaussianDistOffsetY, sizeof(Vec4)* 15);
			memcpy(avSampleWeights, mGaussianDistWeightY, sizeof(Vec4)* 15);
		}
		else
		{
			float afSampleOffsets[15];
			GetSampleOffsets_Bloom(resol.y, afSampleOffsets, mGaussianDistWeightY, 3.0f, 1.25f);
			for (int i = 0; i < 15; i++)
			{
				mGaussianDistOffsetY[i] = avSampleOffsets[i] = Vec4(0.f, afSampleOffsets[i], 0.0f, 0.0f);
			}
			mGaussianDistCalculated = true;
		}

		UnmapBigBuffer();

		rts[0] = mBloomTexture[0];
		SetRenderTarget(rts, index, 1, 0, 0);
		SetTexture(mBloomTexture[1], BINDING_SHADER_PS, 0);

		DrawFullscreenQuad(mBloomPS, false);
	}
}

//---------------------------------------------------------------------------
void Renderer::RenderStarGlare()
{
	if (mStarTextures[0] == 0)
	{
		for (int i = 0; i < FB_NUM_STAR_TEXTURES; ++i)
		{
			mStarTextures[i] = CreateTexture(0, mCropWidth / 4, mCropHeight / 4, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		}

		mStarGlareShader = CreateShader("code/engine/renderer/shaders/starglare.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
		mMergeTexture2 = CreateShader("code/engine/renderer/shaders/mergetextures2ps.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
	}

	ITexture* rts[] = { mStarTextures[0] };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0);
	Viewport vp = { 0, 0, (float)mStarTextures[0]->GetWidth(), (float)mStarTextures[0]->GetHeight(), 0.f, 1.f };
	gFBEnv->pRenderer->SetViewports(&vp, 1);
	Clear();
	const float fTanFoV = mCamera->GetFOV();
	const Color vWhite(1.0f, 1.0f, 1.0f, 1.0f);
	static const int s_maxPasses = 3;
	static const int nSamples = 8;
	static Vec4 s_aaColor[s_maxPasses][8];
	static const Color s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f);

	Vec4 avSampleWeights[FB_MAX_SAMPLES];
	Vec4 avSampleOffsets[FB_MAX_SAMPLES];

	//mAdditiveBlendState->Bind();

	float srcW = (float)mStarSourceTex->GetWidth();
	float srcH = (float)mStarSourceTex->GetHeight();

	for (int p = 0; p < s_maxPasses; ++p)
	{
		float ratio;
		ratio = (float)(p + 1) / (float)s_maxPasses;

		for (int s = 0; s < nSamples; s++)
		{
			Color chromaticAberrColor = Lerp(StarDef::GetChromaticAberrationColor(s), s_colorWhite, ratio);
			s_aaColor[p][s] = Lerp(s_colorWhite, chromaticAberrColor, m_fChromaticAberration).GetVec4();
		}
	}
	float radOffset;
	radOffset = m_fStarInclination + mStarDef.m_fInclination;
	
	ITexture* pSrcTexture = 0;
	ITexture* pDestTexture = 0;

	// Direction loop
	for (int d = 0; d < mStarDef.m_nStarLines; ++d)
	{
		CONST STARLINE& starLine = mStarDef.m_pStarLine[d];

		pSrcTexture = mStarSourceTex;

		float rad;
		rad = radOffset + starLine.fInclination;
		float sn, cs;
		sn = sinf(rad), cs = cosf(rad);
		Vec2 vtStepUV;
		vtStepUV.x = sn / srcW * starLine.fSampleLength;
		vtStepUV.y = cs / srcH * starLine.fSampleLength;

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
			for (int i = 0; i < nSamples; ++i)
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
			BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
			memcpy(pData->gSampleOffsets, avSampleOffsets, sizeof(Vec4)* FB_MAX_SAMPLES);
			memcpy(pData->gSampleWeights, avSampleWeights, sizeof(Vec4)* FB_MAX_SAMPLES);
			UnmapBigBuffer();

			ITexture* rts[] = { pDestTexture };
			size_t index[] = { 0 };
			SetRenderTarget(rts, index, 1, 0, 0);

			SetTexture(pSrcTexture, BINDING_SHADER_PS, 0);
			DrawFullscreenQuad(mStarGlareShader, false);

			// Setup next expansion
			vtStepUV *= nSamples;
			attnPowScale *= nSamples;

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
	textures.reserve(mStarDef.m_nStarLines);
	for (int i = 0; i < mStarDef.m_nStarLines; i++)
	{
		textures.push_back(mStarTextures[i + 4]);
		avSampleWeights[i] = vWhite.GetVec4() * (1.0f / (FLOAT)mStarDef.m_nStarLines);
	}

	{
		ITexture* rts[] = { pDestTexture };
		size_t index[] = { 0 };
		SetRenderTarget(rts, index, 1, 0, 0); 
	}
	SetTextures(&textures[0], textures.size(), BINDING_SHADER_PS, 0);
	
	switch (mStarDef.m_nStarLines)
	{
	case 2:
		DrawFullscreenQuad(mMergeTexture2, false);
		break;

	default:
		assert(0);
	}
}

//---------------------------------------------------------------------------
void Renderer::ToneMapping()
{
	if (!mToneMappingPS)
	{
		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mToneMappingPS = CreateShader("code/engine/renderer/shaders/tonemapping.hlsl", BINDING_SHADER_PS, shaderDefines);
	}
	D3DEventMarker mark("ToneMapping");
	ITexture* textures[] = { mHDRTarget, mLuminanceMap[0], mBloomTexture[0], mStarTextures[0] };
	RestoreRenderTarget();
	SetTextures(textures, 4, BINDING_SHADER_PS, 0); 
	Viewport vp = { 0, 0, (float)GetWidth(), (float)GetHeight(), 0.f, 1.f };
	gFBEnv->pRenderer->SetViewports(&vp, 1);

	DrawFullscreenQuad(mToneMappingPS, false);
	RestoreDepthStencilState();
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
	else if (pCVar->mName == "r_hdrmiddlegray")
	{
		mMiddleGray = gFBEnv->pConsole->GetEngineCommand()->r_HDRMiddleGray;
	}
	else if (pCVar->mName == "r_bloompower")
	{
		mBloomPower = gFBEnv->pConsole->GetEngineCommand()->r_BloomPower;
	}
	else if (pCVar->mName == "r_starpower")
	{
		mStarPower = gFBEnv->pConsole->GetEngineCommand()->r_StarPower;
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
}

ITexture* Renderer::FindRenderTarget(const Vec2I& size)
{
	assert(0);

	return 0;
}

//---------------------------------------------------------------------------
void Renderer::SetDepthRenderTarget(bool clear)
{
	int width = int(mWidth*.5f);
	int height = int(mHeight*.5f);
	if (!mDepthTarget)
	{ 
		mDepthTarget = CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);

		/*BLEND_DESC bdesc;
		bdesc.RenderTarget[0].BlendEnable = true;
		bdesc.RenderTarget[0].BlendOp = BLEND_OP_MIN;
		bdesc.RenderTarget[0].DestBlend = BLEND_ONE;
		bdesc.RenderTarget[0].SrcBlend= BLEND_ONE;
		mMinBlendState = CreateBlendState(bdesc);*/
	}
	ITexture* rts[] = { mDepthTarget };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, mTempDepthBufferHalf, 0);
	Viewport vp = { 0, 0, (float)width, (float)height, 0.f, 1.f };
	SetViewports(&vp, 1);
	if (clear)
		Clear(1.f, 1.f, 1.f, 1.f, 1.f, 0);
	//mMinBlendState->Bind();
	LockBlendState();
}

void Renderer::UnsetDepthRenderTarget()
{
	RestoreRenderTarget();
	UnlockBlendState();
	RestoreViewports();
}

void Renderer::SetGodRayRenderTarget()
{
	if (!mGodRayTarget[0])
	{
		for (int i = 0; i < 2; i++)
		{
			mGodRayTarget[i] = CreateTexture(0, mWidth / 2, mHeight / 2, PIXEL_FORMAT_R8G8B8A8_UNORM,
				BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			/*mGodRayTarget[i]->SetSamplerDesc(SAMPLER_DESC());*/
		}
		mNoMSDepthStencil = CreateTexture(0, mWidth / 2, mHeight / 2, PIXEL_FORMAT_D24_UNORM_S8_UINT,
			BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);

		const char* godray = "code/engine/renderer/shaders/GodRayPS.hlsl";
		const char* occpre = "code/engine/renderer/shaders/OccPrePass.hlsl";
		const char* occpregs = "code/engine/renderer/shaders/OccPrePassGS.hlsl";
		mGodRayPS = CreateShader(godray, BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
		mOccPrePassShader = CreateShader(occpre, BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
		mOccPrePassGSShader = CreateShader(occpregs, BINDING_SHADER_VS | BINDING_SHADER_PS | BINDING_SHADER_GS,
			IMaterial::SHADER_DEFINES());
	}

	
	ITexture* rts[] = { mGodRayTarget[1] };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, mNoMSDepthStencil, 0);
	Viewport vp = { 0, 0, (float)mWidth*.5f, (float)mHeight*.5f, 0.f, 1.f };
	SetViewports(&vp, 1);
	Clear();
}

void Renderer::GodRay()
{
	ILight* pLight = GetDirectionalLight(0);
	assert(pLight);
	Vec4 lightPos(GetCamera()->GetPos() + pLight->GetPosition(), 1.f);
	lightPos = GetCamera()->GetViewProjMat() * lightPos; // only x,y nee
	lightPos.x = lightPos.x*.5f + .5f;
	lightPos.y = .5f - lightPos.y*.5f;
	
	ITexture* rts[] = { mGodRayTarget[0] };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, 0, 0);
	SetTexture(mGodRayTarget[1], BINDING_SHADER_PS, 0);
	Vec4* pData = (Vec4*)MapMaterialParameterBuffer();
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
		UnmapMaterialParameterBuffer();
	}
	DrawFullscreenQuad(mGodRayPS, false);
	

	
	Viewport vp = { 0, 0, (float)mWidth, (float)mHeight, 0.f, 1.f };
	SetViewports(&vp, 1);
	RestoreRenderTarget();
}

//---------------------------------------------------------------------------
void Renderer::BlendGodRay()
{
	D3DEventMarker mark("GodRay Blending");
	assert(mGodRayTarget[0]);
	SetTexture(mGodRayTarget[0], BINDING_SHADER_PS, 0);		
	SetAdditiveBlendState();
	mNoDepthStencilState->Bind(0);
	DrawFullscreenQuad(mCopyPS, false);
	mDefaultBlendState->Bind();
}

//---------------------------------------------------------------------------
void Renderer::SetGlowRenderTarget()
{
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
		mGlowTarget = CreateTexture(0, (int)(mWidth), (int)(mHeight), PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTarget, "GlowTarget");

		mGlowTexture[0] = CreateTexture(0, (int)(mWidth * 0.25f), (int)(mHeight * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTexture[0], "GlowTexture0");

		mGlowTexture[1] = CreateTexture(0, (int)(mWidth * 0.25f), (int)(mHeight * 0.25f), PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV | TEXTURE_TYPE_MULTISAMPLE);
		FB_SET_DEVICE_DEBUG_NAME(mGlowTexture[1], "GlowTexture1");

		//mGlowTarget->SetSamplerDesc(SAMPLER_DESC());
		//mGlowTexture[0]->SetSamplerDesc(SAMPLER_DESC());
		//mGlowTexture[1]->SetSamplerDesc(SAMPLER_DESC());

		IMaterial::SHADER_DEFINES shaderDefines;
		if (GetMultiSampleCount() != 1)
		{
			shaderDefines.push_back(IMaterial::ShaderDefine());
			shaderDefines.back().name = "_MULTI_SAMPLE";
			shaderDefines.back().value = "1";
		}
		mGlowPS = CreateShader("code/engine/renderer/shaders/BloomPS.hlsl", BINDING_SHADER_PS, shaderDefines);
		FB_SET_DEVICE_DEBUG_NAME(mGlowPS, "GlowPS");
	}
}

//---------------------------------------------------------------------------
void Renderer::BlendGlow()
{
	{
		D3DEventMarker mark("Glowing");
		assert(mGlowTarget);
		SetTexture(mGlowTarget, BINDING_SHADER_PS, 0);
		ITexture* rt[] = { mGlowTexture[1] };
		size_t index[] = { 0 };
		SetRenderTarget(rt, index, 1, 0, 0);
		mDefaultBlendState->Bind();		
		Vec2I resol = mGlowTexture[0]->GetSize();
		Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
		SetViewports(&vp, 1);

		// Horizontal Blur
		BIG_BUFFER* pData = (BIG_BUFFER*)MapBigBuffer();
		Vec4* avSampleOffsets = pData->gSampleOffsets;
		Vec4* avSampleWeights = pData->gSampleWeights;
		if (mGaussianDistGlowCalculated)
		{
			memcpy(avSampleOffsets, mGaussianDistGlowOffsetX, sizeof(Vec4)* 15);
			memcpy(avSampleWeights, mGaussianDistGlowWeightX, sizeof(Vec4)* 15);
		}
		else
		{
			float afSampleOffsets[15];
			GetSampleOffsets_Bloom(resol.x, afSampleOffsets, mGaussianDistGlowWeightX, 3.f, 1.25f);
			for (int i = 0; i < 15; i++)
			{
				avSampleWeights[i] = mGaussianDistGlowWeightX[i];
				avSampleOffsets[i]  = mGaussianDistGlowOffsetX[i] = Vec4(afSampleOffsets[i], 0.0f, 0.0f, 0.0f);
			}
		}
		UnmapBigBuffer();
		// mGlowPS is same with BloomPS except it has the _MULTI_SAMPLE shader define.
		DrawFullscreenQuad(mGlowPS, false);



		// Vertical Blur
		pData = (BIG_BUFFER*)MapBigBuffer();
		avSampleOffsets = pData->gSampleOffsets;
		avSampleWeights = pData->gSampleWeights;
		if (mGaussianDistGlowCalculated)
		{
			memcpy(avSampleOffsets, mGaussianDistGlowOffsetY, sizeof(Vec4)* 15);
			memcpy(avSampleWeights, mGaussianDistGlowWeightY, sizeof(Vec4)* 15);
		}
		else
		{
			float afSampleOffsets[15];
			GetSampleOffsets_Bloom(mGlowTexture[0]->GetSize().y, afSampleOffsets, mGaussianDistGlowWeightY, 3.f, 1.25f);
			for (int i = 0; i < 15; i++)
			{
				avSampleOffsets[i] = mGaussianDistGlowOffsetY[i] = Vec4(0.f, afSampleOffsets[i], 0.0f, 0.0f);
				avSampleWeights[i] = mGaussianDistGlowWeightY[i];
			}
			mGaussianDistGlowCalculated = true;
		}
		UnmapBigBuffer();
		rt[0] = mGlowTexture[0];
		SetRenderTarget(rt, index, 1, 0, 0);
		SetTexture(mGlowTexture[1], BINDING_SHADER_PS, 0);
		DrawFullscreenQuad(mGlowPS, false);
	}

	{
		D3DEventMarker mark("Glow Blend");

		RestoreViewports();
		if (gFBEnv->pConsole->GetEngineCommand()->r_HDR)
		{
			SetHDRTarget();
		}
		else
		{
			RestoreRenderTarget();
		}
		SetTexture(mGlowTexture[0], BINDING_SHADER_PS, 0);
		SetAdditiveBlendState();
		mNoDepthStencilState->Bind(0);
		if (GetMultiSampleCount()==1)
			DrawFullscreenQuad(mCopyPS, false);
		else
			DrawFullscreenQuad(mCopyPSMS, false);
		mDefaultBlendState->Bind();
	}
}

//---------------------------------------------------------------------------
void Renderer::SetDepthWriteShader()
{
	if (mCloudRendering)
	{
		if (!mCloudDepthWriteShader)
		{
			mCloudDepthWriteShader = CreateShader("code/engine/renderer/shaders/depth_cloud.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
				IMaterial::SHADER_DEFINES());
			if (!mPositionInputLayout)
				mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mCloudDepthWriteShader);
		}
		mPositionInputLayout->Bind();
		mCloudDepthWriteShader->Bind();
			
	}
	else
	{
		if (!mDepthWriteShader)
		{
			mDepthWriteShader = CreateShader("code/engine/renderer/shaders/depth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
				IMaterial::SHADER_DEFINES());
			if (!mPositionInputLayout)
				mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, mDepthWriteShader);
		}
		mPositionInputLayout->Bind();
		mDepthWriteShader->Bind();
	}
	
}

//---------------------------------------------------------------------------
void Renderer::SetOccPreShader()
{
	if (mOccPrePassShader)
		mOccPrePassShader->Bind();
}
void Renderer::SetOccPreGSShader()
{
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
	if (set)
	{
		SetTexture(mDepthTarget, BINDING_SHADER_PS, 5);
	}
	else 
		SetTexture(0, BINDING_SHADER_PS, 5);
}

void Renderer::SetCloudVolumeTarget()
{
	if (!mCloudVolumeDepth)
	{
		mCloudVolumeDepth = CreateTexture(0, mWidth / 2, mHeight / 2, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
		/*SAMPLER_DESC sdesc;
		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		mCloudVolumeDepth->SetSamplerDesc(sdesc);*/
	}
	assert(mTempDepthBufferHalf);
	ITexture* rts[] = { mCloudVolumeDepth };
	size_t index[] = { 0 };
	// mTempDepthBufferHalf already filled with scene objects. while writing the depth texture;
	SetRenderTarget(rts, index, 1, mTempDepthBufferHalf, 0);
	Viewport vp = { 0, 0, mWidth*.5f, mHeight*.5f, 0.f, 1.f };
	SetViewports(&vp, 1);
	Clear(0.0f, 0.0f, 0.0f, 0.f, 1, 0);
}

void Renderer::SetCloudVolumeTexture(bool set)
{
	if (set)
		SetTexture(mCloudVolumeDepth, BINDING_SHADER_PS, 6);
	else
		SetTexture(0, BINDING_SHADER_PS, 6);
}


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
		/*SAMPLER_DESC sdesc;
		sdesc.AddressU = TEXTURE_ADDRESS_WRAP;
		sdesc.AddressV = TEXTURE_ADDRESS_WRAP;
		mNoiseMap->SetSamplerDesc(sdesc);*/
	}
	mNoiseMap->Bind();
}

void Renderer::SetCloudRendering(bool rendering)
{
	mCloudRendering = rendering;
}

//---------------------------------------------------------------------------
void Renderer::PrepareShadowMapRendering()
{
	if (!mShadowMap)
	{
		mShadowMap = CreateTexture(0, 2048, 2048, PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL_SRV);
	}

	ITexture* rts[] = { 0 };
	size_t index[] = { 0 };
	SetRenderTarget(rts, index, 1, mShadowMap, 0);
	gFBEnv->mRenderPass = PASS_SHADOW;
	SetShadowMapShader();
	Viewport vp = { 0, 0, 2048, 2048, 0.f, 1.f };
	SetViewports(&vp, 1);
	mCameraBackup = GetCamera();
	SetCamera(GetDirectionalLight(0)->GetCamera());
	Clear(0, 0, 0, 0, 1.0f, 0);
}

//---------------------------------------------------------------------------
void Renderer::EndShadowMapRendering()
{
	assert(mShadowMap);
	RestoreRenderTarget();
	gFBEnv->mRenderPass = PASS_NORMAL;
	SetCamera(mCameraBackup);
	UpdateRareConstantsBuffer();
	RestoreViewports();
}

void Renderer::BindShadowMap(bool bind)
{
	if (bind)
		SetTexture(mShadowMap, BINDING_SHADER_PS, 8);
	else
		SetTexture(0, BINDING_SHADER_PS, 8);
}

void Renderer::SetShadowMapShader()
{
	if (!mShadowMapShader)
	{
		mShadowMapShader = CreateShader("code/engine/renderer/shaders/shadowdepth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
	}
	mShadowMapShader->Bind();
}

void Renderer::SetSilouetteShader()
{
	assert(0);
	/*if (!mSilouetteShader)
	{
		mSilouetteShader = CreateShader("code/engine/renderer/shaders/silouette.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
	}

	mSilouetteShader->Bind();*/
}

void Renderer::SetSamllSilouetteBuffer()
{
	int width = int(mWidth*0.5f);
	int height = int(mHeight*0.5f);
	if (!mSmallSilouetteBuffer)
	{
		mSmallSilouetteBuffer = CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}

	ITexture* rts[] = { mSmallSilouetteBuffer };
	unsigned index[] = { 0 };
	SetRenderTarget(rts, index, 1, mTempDepthBufferHalf, 0);
	Viewport vps = { 0.f, 0.f, (float)width, (float)height, 0.0f, 1.0f };
	SetViewports(&vps, 1);
	if (!gFBEnv->mSilouetteRendered)
		Clear(1, 1, 1, 1, 1.f, 0);
}
void Renderer::SetBigSilouetteBuffer()
{
	int width = int(mWidth);
	int height = int(mHeight);
	if (!mBigSilouetteBuffer)
	{
		mBigSilouetteBuffer = CreateTexture(0, width, height, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
	}

	ITexture* rts[] = { mBigSilouetteBuffer };
	unsigned index[] = { 0 };
	SetRenderTarget(rts, index, 1, mTempDepthBuffer, 0);
	Viewport vps = { 0.f, 0.f, (float)width, (float)height, 0.0f, 1.0f };
	SetViewports(&vps, 1);
	if (!gFBEnv->mSilouetteRendered)
		Clear(1, 1, 1, 1, 1.f, 0);
}

void Renderer::DrawSilouette()
{
	if (!mSilouetteShader)
	{
		mSilouetteShader = CreateShader("code/engine/renderer/shaders/silouette.hlsl", BINDING_SHADER_PS,
			IMaterial::SHADER_DEFINES());
	}

	if (gFBEnv->pConsole->GetEngineCommand()->r_HDR)
		SetHDRTarget();
	else
		RestoreRenderTarget();
	RestoreViewports();
	ITexture* ts[] = { mSmallSilouetteBuffer, mBigSilouetteBuffer, mDepthTarget };
	SetTextures(ts, 3, BINDING_SHADER_PS, 0);
	
	DrawFullscreenQuad(mSilouetteShader, false);
}


void Renderer::RenderDebugRenderTargets()
{
	for (int i = 0; i < MaxDebugRenderTargets; i++)
	{
		if (mDebugRenderTargets[i].mTexture)
		{
			Vec2 pixelPos = mDebugRenderTargets[i].mPos * Vec2((float)mWidth, (float)mHeight);
			Vec2 pixelSize = mDebugRenderTargets[i].mSize * Vec2((float)mWidth, (float)mHeight);
			DrawQuadWithTexture(Round(pixelPos), Round(pixelSize), Color(1, 1, 1, 1),
				mDebugRenderTargets[i].mTexture);
		}
	}
}
void Renderer::SetDebugRenderTarget(unsigned idx, const char* textureName)
{
	assert(idx < MaxDebugRenderTargets);
	if (stricmp(textureName, "Shadow") == 0)
		mDebugRenderTargets[idx].mTexture = mShadowMap;
	else
		mDebugRenderTargets[idx].mTexture = 0;
}

IRenderToTexture* Renderer::CreateRenderToTexture(bool everyframe, Vec2I size, PIXEL_FORMAT format, 
	bool srv, bool miplevel, bool cubeMap, bool needDepth)
{
	for (auto rt : mRenderToTexturePool)
	{
		if (rt->CheckOptions(size, format, srv, miplevel, cubeMap, needDepth))
		{
			if (everyframe)
				mRenderToTextures.push_back(rt);
			return rt;
		}
			
	}
	return 0;
}

void Renderer::DeleteRenderToTexture(IRenderToTexture* rt)
{
	if (ValueNotExistInVector(mRenderToTexturePool, rt))
	{
		mRenderToTexturePool.push_back(rt);
	}
}

}
