#include <Engine/StdAfx.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/Font.h>
#include <Engine/Renderer/Material.h>
#include <Engine/RenderObjects/DebugHud.h>
#include <Engine/Renderer/D3DEventMarker.h>
#include <Engine/Renderer/RenderToTexture.h>
#include <Engine/ILight.h>
#include <Engine/IConsole.h>
#include <Engine/IScriptSystem.h>
#include <CommonLib/Unicode.h>

namespace fastbird
{

//----------------------------------------------------------------------------
Renderer::Renderer()
	: DEFAULT_DYN_VERTEX_COUNTS(100)
	, mDepthClear(1.f)
	, mStencilClear(0)
{
}
Renderer::~Renderer()
{
}

//----------------------------------------------------------------------------
bool Renderer::OnPrepared()
{
	mMaterials[DEFAULT_MATERIALS::QUAD] = fastbird::IMaterial::CreateMaterial(
		"es/materials/quad.material");

	assert(DEFAULT_INPUTS::COUNT == 6);
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
			INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 12, 
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

	//-----------------------------------------------------------------------
	mDynVBs[DEFAULT_INPUTS::POSITION] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_P), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PC), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_NORMAL] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PN), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_TEXCOORD] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PT), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES] = 
		CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PCTB), 
		DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

	struct PosUv
	{
		Vec3 mPos;
		Vec2 mUv;
	};

	PosUv quadData[] = { 
		{Vec3(-1, -1,0.99999f), Vec2(0, 1)},
		{Vec3(-1, 1, 0.99999f), Vec2(0, 0)},
		{Vec3(1, 1, 0.99999f), Vec2(1, 0)},

		{Vec3(1, 1, 0.99999f), Vec2(1, 0)},
		{Vec3(1, -1, 0.99999f), Vec2(1, 1)},
		{Vec3(-1, -1, 0.99999f), Vec2(0, 1)}
	};

	mVBQuadUV_Far = CreateVertexBuffer(quadData, sizeof(Vec3)+sizeof(Vec2), 6, BUFFER_USAGE_IMMUTABLE,
		BUFFER_CPU_ACCESS_NONE);

	PosUv quadData2[] = { 
		{Vec3(-1, -1, 0), Vec2(0, 1)},
		{Vec3(-1, 1, 0), Vec2(0, 0)},
		{Vec3(1, 1, 0), Vec2(1, 0)},

		{Vec3(1, 1, 0), Vec2(1, 0)},
		{Vec3(1, -1, 0), Vec2(1, 1)},
		{Vec3(-1, -1, 0), Vec2(0, 1)}
	};

	mVBQuadUV_Near = CreateVertexBuffer(quadData2, sizeof(Vec3)+sizeof(Vec2), 6, BUFFER_USAGE_IMMUTABLE,
		BUFFER_CPU_ACCESS_NONE);


	mFont = new Font();
	std::string fontName = gFBEnv->pScriptSystem->GetStringVariable("font");
	if (fontName.empty())
	{
		fontName = "es/fonts/nanum_pen.fnt";
	}
	mFont->Init(fontName.c_str());
	mFont->SetTextEncoding(IFont::UTF16);
	mDebugHud = new DebugHud;

	if (gFBEnv->pConsole)
		gFBEnv->pConsole->Init();

	UpdateRareConstantsBuffer();
	return true;
}

void Renderer::OnDeinit()
{
	mFont = 0;
}

//----------------------------------------------------------------------------
void Renderer::ProcessRenderToTexture()
{
	for each(RenderToTexture* pRT in mRenderToTextures)
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
	mCamera = pCamera;
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
	for each(auto ta in mTextureAtalsCache)
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
			pTextureAtlas = new TextureAtlas;
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

				TextureAtlasRegion* pRegion = new TextureAtlasRegion;
				pRegion->mName = szBuffer;
				pRegion->mID = pRegionElem->UnsignedAttribute("id");
				Vec2 start(0, 0), end(0, 0);
				start.x = (float)pRegionElem->IntAttribute("x");
				start.y = (float)pRegionElem->IntAttribute("y");
				end.x = start.x + pRegionElem->IntAttribute("width");
				end.y = start.y + pRegionElem->IntAttribute("height");			
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
		IMaterial* pNewMaterial = new Material;
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

void Renderer::SetDirectionalLight(ILight* pLight)
{
	mDirectionalLightOverride = pLight;
}

int Renderer::BindFullscreenQuadUV_VB(bool farSide)
{
	if (farSide)
	{
		mVBQuadUV_Far->Bind();
		return mVBQuadUV_Far->GetNumVertices();
	}
	else
	{
		mVBQuadUV_Near->Bind();
		return mVBQuadUV_Near->GetNumVertices();		
	}
}

void Renderer::SetEnvironmentTexture(ITexture* pTexture)
{
	mEnvironmentTexture = pTexture;
	mEnvironmentTexture->SetShaderStage(BINDING_SHADER_PS);
	SAMPLER_DESC desc;
	desc.AddressU = TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = TEXTURE_ADDRESS_CLAMP;
	desc.Filter = TEXTURE_FILTER_ANISOTROPIC;
	mEnvironmentTexture->SetSamplerDesc(desc);
	mEnvironmentTexture->SetSlot(4); // hardcoded environment slot.
	mEnvironmentTexture->Bind();
}

}