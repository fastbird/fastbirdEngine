#include <Engine/StdAfx.h>
#include <Engine/Material.h>
#include <Engine/IShader.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/Renderer.h>
#include <Engine/ITexture.h>
#include <Engine/Shader.h>
#include <Engine/RendererStructs.h>
#include <Engine/RenderTarget.h>
#include <FreeImage.h>

using namespace fastbird;

//---------------------------------------------------------------------------
IMaterial* IMaterial::CreateMaterial(const char* file)
{
	if (gFBEnv && gFBEnv->_pInternalRenderer)
	{
		IMaterial* pMat = gFBEnv->_pInternalRenderer->CreateMaterial(file);
		assert(pMat);
		return pMat;
	}

	return 0;
}

void IMaterial::ReloadMaterial(const char* file)
{
	Material::ReloadMaterial(file);
}

void IMaterial::ReloadShader(const char* shader)
{
	Material::ReloadShader(shader);
}

//----------------------------------------------------------------------------
Material::Materials Material::mMaterials;
FB_READ_WRITE_CS Material::mRWCSMaterial;

//----------------------------------------------------------------------------
Material::Material()
	: mReloading(false)
	, mReloadingTryCount(0)
	, mAdamMaterial(0)
	, mShaders(0)
	, mTransparent(false)
	, mRenderPass(RENDER_PASS::PASS_NORMAL)
	, mGlow(false)
	, mNoShadowCast(false), mDoubleSided(false), mRenderStatesCloned(false)
{
	WRITE_LOCK wl(mRWCSMaterial);
	mMaterials.push_back(this);
	mRenderStates = FB_NEW(RenderStates);
}

Material::Material(const Material& mat)
	: mReloading(false)
	, mReloadingTryCount(0)
	, mAdamMaterial(0)
	, mRenderStatesCloned(false)
{
	{
		WRITE_LOCK wl(mRWCSMaterial);
		mMaterials.push_back(this);
	}
	mMaterialConstants =	mat.mMaterialConstants;
	mShader =				mat.mShader;
	mName =					mat.mName;
	mMaterialParameters =	mat.mMaterialParameters;
	mColorRampMap =			mat.mColorRampMap;
	mShaderDefines =		mat.mShaderDefines;
	mInputElementDescs =	mat.mInputElementDescs;
	mInputLayout =			mat.mInputLayout;
	
	mRenderStates = mat.mRenderStates;
	mShaders = mat.mShaders;

	for (auto it : mat.mTextures){
		if (it->GetType() == TEXTURE_TYPE_COLOR_RAMP){
			SetColorRampTexture(mColorRampMap[it], it->GetShaderStage(), it->GetSlot());
			auto colorIt = mColorRampMap.find(it);
			mColorRampMap.erase(colorIt);			
		}
		else{
			mTextures.push_back(it);
		}
		
	}
	
	mTransparent = mat.mTransparent;
	mGlow = mat.mGlow;
	mNoShadowCast = mat.mNoShadowCast;
	mDoubleSided = mat.mDoubleSided;
	mRenderPass = mat.mRenderPass;
	size_t num = mat.mSubMaterials.size();
	for (size_t i = 0; i < num; ++i)
	{
		mSubMaterials.push_back(mat.mSubMaterials[i]->Clone());
	}
}

IMaterial* Material::Clone()
{
	Material* pMat = FB_NEW(Material)(*this);
	pMat->SetAdam(mAdamMaterial ? mAdamMaterial : const_cast<Material*>(this));

	mInstances.push_back(pMat);

	return pMat;
}

IMaterial* Material::GetAdam() const
{
	return mAdamMaterial;
}

//----------------------------------------------------------------------------
Material::~Material()
{
	{
		WRITE_LOCK wl(mRWCSMaterial);
		mMaterials.erase(std::find(mMaterials.begin(), mMaterials.end(), this));
	}
	if (mAdamMaterial)
	{
		mAdamMaterial->RemoveInstance(this);
	}
}

void Material::FinishSmartPtr(){
	FB_DELETE(this);
}

//---------------------------------------------------------------------------
//// only need if you don't use shared ptr
void Material::Delete()
{
	FB_DELETE(this);
}

//----------------------------------------------------------------------------
// IMaterial Interfaces
//----------------------------------------------------------------------------
bool Material::LoadFromFile(const char* filepath)
{
	if (!filepath)
		return false;
	mName = filepath;
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filepath);
	if (doc.Error())
	{
		Error(FB_DEFAULT_DEBUG_ARG, FormatString("Error while parsing material(%s)!", filepath));
		if (doc.ErrorID()==tinyxml2::XML_ERROR_FILE_NOT_FOUND)
		{
			Log("Material %s is not found!", filepath);
			if (mReloading)
				return true;
		}
		const char* errMsg = doc.GetErrorStr1();
		if (errMsg)
			Log("\t%s", errMsg);
		errMsg = doc.GetErrorStr2();
		if (errMsg)
			Log("\t%s", errMsg);

		Log("\t loading missing material.");
		doc.LoadFile("es/materials/missing.material");
		if (doc.Error())
		{
			Log("Loading missing material is also failed!");
			return false;
		}
	}
	
	tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Material");
	if (!pRoot)
	{
		assert(0);
		return false;
	}

	return LoadFromXml(pRoot);

	
}

bool Material::LoadFromXml(tinyxml2::XMLElement* pRoot)
{
	const char* sz = pRoot->Attribute("transparent");
	if (sz)
	{
		mTransparent = StringConverter::parseBool(sz);
	}

	sz = pRoot->Attribute("glow");
	if (sz)
	{
		mGlow = StringConverter::parseBool(sz);
	}

	sz = pRoot->Attribute("noShadowCast");
	if (sz)
	{
		mNoShadowCast = StringConverter::parseBool(sz);
	}

	sz = pRoot->Attribute("doubleSided");
	if (sz)
	{
		mDoubleSided = StringConverter::parseBool(sz);
	}

	//-----------------------------------------------------------------------------
	// BlendDesc
	//-----------------------------------------------------------------------------
	BLEND_DESC bdesc;
	auto blendDescElem = pRoot->FirstChildElement("BlendDesc");
	if (blendDescElem)
	{
		sz = blendDescElem->Attribute("BlendEnalbe");
		if (sz)
			bdesc.RenderTarget[0].BlendEnable = StringConverter::parseBool(sz);
		if (bdesc.RenderTarget[0].BlendEnable)
		{
			mTransparent = true;
		}
		sz = blendDescElem->Attribute("BlendOp");
		if (sz)
			bdesc.RenderTarget[0].BlendOp = BlendOpFromString(sz);
		sz = blendDescElem->Attribute("SrcBlend");
		if (sz)
			bdesc.RenderTarget[0].SrcBlend = BlendFromString(sz);
		sz = blendDescElem->Attribute("DestBlend");
		if (sz)
			bdesc.RenderTarget[0].DestBlend = BlendFromString(sz);
		sz = blendDescElem->Attribute("RenderTargetWriteMask");
		if (sz)
		{
			auto options = Split(sz, "|");
			for (auto& it : options)
			{
				it = StripBoth(it.c_str());
			}
			int mask = 0;
			for (auto& it : options)
			{
				mask += ColorWriteMaskFromString(it.c_str());
			}
			bdesc.RenderTarget[0].RenderTargetWriteMask = mask;			
		}
	}
	else
	{
		if (mTransparent)
		{
			bdesc.RenderTarget[0].BlendEnable = true;
			bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
		}
	}
	SetBlendState(bdesc);
	//-----------------------------------------------------------------------------
	// DepthStencilDesc
	//-----------------------------------------------------------------------------
	DEPTH_STENCIL_DESC ddesc;
	auto depthDescElem = pRoot->FirstChildElement("DepthStencilDesc");
	if (depthDescElem)
	{
		sz = depthDescElem->Attribute("DepthEnable");
		if (sz)
			ddesc.DepthEnable = StringConverter::parseBool(sz);
		sz = depthDescElem->Attribute("DepthWriteMask");
		if (sz)
			ddesc.DepthWriteMask = DepthWriteMaskFromString(sz);
		sz = depthDescElem->Attribute("DepthFunc");
		if (sz)
			ddesc.DepthFunc = ComparisonFuncFromString(sz);
	}
	else
	{
		ddesc.DepthWriteMask = mTransparent ? DEPTH_WRITE_MASK_ZERO : DEPTH_WRITE_MASK_ALL;
	}
	SetDepthStencilState(ddesc);

	//-----------------------------------------------------------------------------
	// RasterizerDesc
	//-----------------------------------------------------------------------------
	RASTERIZER_DESC rdesc;
	auto rasterizerDescElem = pRoot->FirstChildElement("RasterizerDesc");
	if (rasterizerDescElem)
	{
		sz = rasterizerDescElem->Attribute("FillMode");
		if (sz)
			rdesc.FillMode = FillModeFromString(sz);
		sz = rasterizerDescElem->Attribute("CullMode");
		if (sz)
			rdesc.CullMode = CullModeFromString(sz);
		sz = rasterizerDescElem->Attribute("ScissorEnable");
		if (sz)
			rdesc.ScissorEnable = StringConverter::parseBool(sz);
		sz = rasterizerDescElem->Attribute("DepthBias");
		if (sz)
			rdesc.DepthBias = StringConverter::parseInt(sz);
	}
	else
	{
		rdesc.CullMode = mDoubleSided ? CULL_MODE_NONE : CULL_MODE_BACK;
	}
	SetRasterizerState(rdesc);
	

	sz = pRoot->Attribute("pass");
	if (sz)
		mRenderPass = (RENDER_PASS)RenderPassFromString(sz);

	tinyxml2::XMLElement* pMaterialConstants = pRoot->FirstChildElement("MaterialConstants");
	if (pMaterialConstants)
	{
		tinyxml2::XMLElement* pAmbientElem = pMaterialConstants->FirstChildElement("AmbientColor");
		if (pAmbientElem)
		{
			const char* szAmbient = pAmbientElem->GetText();
			Vec4 ambient(szAmbient);
			SetAmbientColor(ambient);
		}
		tinyxml2::XMLElement* pDiffuseElem = pMaterialConstants->FirstChildElement("DiffuseColor_Alpha");
		if (pDiffuseElem)
		{
			const char* szDiffuse = pDiffuseElem->GetText();
			Vec4 diffuse(szDiffuse);
			SetDiffuseColor(diffuse);
		}
		tinyxml2::XMLElement* pSpecularElem = pMaterialConstants->FirstChildElement("SpecularColor_Shine");
		if (pSpecularElem)
		{
			const char* szSpecular = pSpecularElem->GetText();
			Vec4 specular(szSpecular);
			SetSpecularColor(specular);
		}
		tinyxml2::XMLElement* pEmissiveElem = pMaterialConstants->FirstChildElement("EmissiveColor_Strength");
		if (pEmissiveElem)
		{
			const char* szEmissive = pEmissiveElem->GetText();
			Vec4 emissive(szEmissive);
			SetEmissiveColor(emissive);
		}
	}

	tinyxml2::XMLElement* pMaterialParameters = pRoot->FirstChildElement("MaterialParameters");
	mMaterialParameters.clear();
	if (pMaterialParameters)
	{
		tinyxml2::XMLElement* pElem = pMaterialParameters->FirstChildElement();
		int i = 0;
		while (pElem)
		{
			const char* szVector = pElem->GetText();
			if (szVector)
			{
				Vec4 v(szVector);
				mMaterialParameters.Insert(PARAMETER_VECTOR::value_type(i++, v));
			}
			pElem = pElem->NextSiblingElement();
		}
	}

	tinyxml2::XMLElement* pDefines = pRoot->FirstChildElement("ShaderDefines");
	mShaderDefines.clear();
	if (pDefines)
	{
		tinyxml2::XMLElement* pElem = pDefines->FirstChildElement();
		int i = 0;
		while (pElem)
		{
			mShaderDefines.push_back(ShaderDefine());
			const char* pname = pElem->Attribute("name");
			if (pname)
				mShaderDefines.back().name = pname;
			const char* pval = pElem->Attribute("val");
			if (pval)
				mShaderDefines.back().value = pval;

			pElem = pElem->NextSiblingElement();
		}
	}

	tinyxml2::XMLElement* pTexturesElem = pRoot->FirstChildElement("Textures");
	if (pTexturesElem)
	{
		tinyxml2::XMLElement* pTexElem = pTexturesElem->FirstChildElement("Texture");
		while (pTexElem)
		{
			const char* filepath = pTexElem->GetText();
			int slot = 0;
			BINDING_SHADER shader = BINDING_SHADER_PS;
			SAMPLER_DESC samplerDesc;
			pTexElem->QueryIntAttribute("slot", &slot);
			const char* szShader = pTexElem->Attribute("shader");
			shader = BindingShaderFromString(szShader);
			const char* szAddressU = pTexElem->Attribute("AddressU");
			samplerDesc.AddressU = AddressModeFromString(szAddressU);
			const char* szAddressV = pTexElem->Attribute("AddressV");
			samplerDesc.AddressV = AddressModeFromString(szAddressV);
			const char* szFilter = pTexElem->Attribute("Filter");
			samplerDesc.Filter = FilterFromString(szFilter);
			ITexture* pTextureInTheSlot = 0;
			const char* szType = pTexElem->Attribute("type");
			TEXTURE_TYPE type = TextureTypeFromString(szType);
			ColorRamp cr;
			if (type == TEXTURE_TYPE_COLOR_RAMP)
			{
				tinyxml2::XMLElement* barElem = pTexElem->FirstChildElement("Bar");
				while (barElem)
				{
					float pos = barElem->FloatAttribute("pos");
					const char* szColor = barElem->GetText();
					Vec4 color(szColor);
					cr.InsertBar(pos, color);
					barElem = barElem->NextSiblingElement();
				}
				SetColorRampTexture(cr, shader, slot, samplerDesc);
			}
			else
			{
				SetTexture(filepath, shader, slot, samplerDesc);
			}
			pTexElem = pTexElem->NextSiblingElement("Texture");
		}
	}

	mShaders = BINDING_SHADER_VS | BINDING_SHADER_PS;
	tinyxml2::XMLElement* pShaders = pRoot->FirstChildElement("Shaders");
	if (pShaders)
	{
		const char* shaders = pShaders->GetText();
		if (shaders)
		{
			mShaders = 0;
			std::string strShaders = shaders;
			ToLowerCase(strShaders);
			if (strShaders.find("vs") != std::string::npos)
			{
				mShaders |= BINDING_SHADER_VS;
			}
			if (strShaders.find("hs") != std::string::npos)
			{
				mShaders |= BINDING_SHADER_HS;
			}
			if (strShaders.find("ds") != std::string::npos)
			{
				mShaders |= BINDING_SHADER_DS;
			}
			if (strShaders.find("gs") != std::string::npos)
			{
				mShaders |= BINDING_SHADER_GS;
			}
			if (strShaders.find("ps") != std::string::npos)
			{
				mShaders |= BINDING_SHADER_PS;
			}
		}
	}

	tinyxml2::XMLElement* pShaderFileElem = pRoot->FirstChildElement("ShaderFile");
	if (pShaderFileElem)
	{
		const char* shaderFile = pShaderFileElem->GetText();
		if (shaderFile)
		{
			mShader = gFBEnv->pEngine->GetRenderer()->CreateShader(shaderFile, mShaders, mShaderDefines);
			mShaderFile = mShader->GetName();
		}
	}

	tinyxml2::XMLElement* pInputLayoutElem = pRoot->FirstChildElement("InputLayout");
	mInputElementDescs.clear();
	if (pInputLayoutElem)
	{
		tinyxml2::XMLElement* pElem = pInputLayoutElem->FirstChildElement();
		int i = 0;
		while (pElem)
		{
			mInputElementDescs.push_back(INPUT_ELEMENT_DESC());
			const char* pbuffer = pElem->Attribute("semantic");
			if (pbuffer)
				strcpy_s(mInputElementDescs.back().mSemanticName, pbuffer);
			mInputElementDescs.back().mSemanticIndex = pElem->IntAttribute("index");

			pbuffer = pElem->Attribute("format");
			if (pbuffer)
			{
				mInputElementDescs.back().mFormat = InputElementFromString(pbuffer);
			}

			mInputElementDescs.back().mInputSlot = pElem->IntAttribute("slot");

			mInputElementDescs.back().mAlignedByteOffset = pElem->IntAttribute("alignedByteOffset");

			pbuffer = pElem->Attribute("inputSlotClass");
			if (pbuffer)
				mInputElementDescs.back().mInputSlotClass = InputClassificationFromString(pbuffer);

			mInputElementDescs.back().mInstanceDataStepRate = pElem->IntAttribute("stepRate");

			pElem = pElem->NextSiblingElement();
		}
	}

	tinyxml2::XMLElement* subMat = pRoot->FirstChildElement("Material");
	while (subMat)
	{
		Material* pMat = FB_NEW(Material);
		mSubMaterials.push_back(pMat);
		pMat->LoadFromXml(subMat);

		subMat = subMat->NextSiblingElement("Material");
	}

	return true;
}

void Material::RemoveInstance(Material* pInstance)
{
	mInstances.erase(std::remove(mInstances.begin(), mInstances.end(), pInstance),
		mInstances.end());
}

//----------------------------------------------------------------------------
void Material::ReloadMaterial(const char* name)
{
	std::string filepath(name);
	ToLowerCase(filepath);
	READ_LOCK rl(mRWCSMaterial);
	for (auto mat : mMaterials)
	{
		if (strcmp(mat->GetName(), filepath.c_str()) == 0)
		{
			// not reloading instances.
			if (mat->GetAdam())
				mat->GetAdam()->RegisterReloading();
			else
				mat->RegisterReloading();
			return;
		}
	}
}

//----------------------------------------------------------------------------
void Material::ReloadShader(const char* shader)
{
	std::string shaderPath(shader);
	ToLowerCase(shaderPath);
	typedef VectorMap<std::string, std::vector<SHADER_DEFINES>> RELOAD_CACHE;
	RELOAD_CACHE reloaded;
	READ_LOCK rl(mRWCSMaterial);
	for (auto mat : mMaterials)
	{
		if (mat->IsRelatedShader(shaderPath.c_str()))
		{
			IMaterial* adam = mat->GetAdam();
			// child don't need reload.
			if (adam)
				continue;
			auto cache = reloaded.Find(mat->GetShaderFile());
			bool alreadyReloaded = false;
			if (cache != reloaded.end())
			{
				auto defines = std::find(cache->second.begin(), cache->second.end(), mat->GetShaderDefines());
				if (defines != cache->second.end())
					alreadyReloaded = true;
			}
			else
			{
				cache = reloaded.Insert(std::make_pair(mat->GetShaderFile(), std::vector<SHADER_DEFINES>()));
			}
			
			if (!alreadyReloaded)
			{
				Log("Reloading a shader(%s) using by the material(%s)", mat->GetShaderFile(), mat->GetName());
				Shader::ReloadShader(mat->GetShaderFile(), mat->GetShaderDefines());
				cache->second.push_back(mat->GetShaderDefines());
			}
		}
	}
}

//----------------------------------------------------------------------------
bool Material::FindTextureIn(BINDING_SHADER shader, int slot, ITexture** pTextureInTheSlot,
	TextureSignature* pSignature/*=0*/) const
{
	*pTextureInTheSlot = 0;
	auto it = mTextures.begin(), itEnd = mTextures.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetSlot() == slot && (*it)->GetShaderStage() == shader)
		{
			*pTextureInTheSlot = (*it);
			break;
		}
	}
	if (!(*pTextureInTheSlot))
		return false;
		
	if (pSignature)
	{
		ITexture* pTexture = *pTextureInTheSlot;
		if (pSignature->mType != TEXTURE_TYPE_COUNT &&
			pSignature->mType != pTexture->GetType())
			return false;

		if (pSignature->mFilepath != pTexture->GetName())
			return false;

		if (pSignature->mColorRamp)
		{
			auto itFound = mColorRampMap.find(pTexture);
			if ( itFound==mColorRampMap.end() || 
				!(itFound->second == *pSignature->mColorRamp) )
			{
				return false; // not same
			}
		}
	}

	return true; // same
}

//----------------------------------------------------------------------------
void Material::SetAmbientColor(float r, float g, float b, float a)
{
	mMaterialConstants.gAmbientColor = Vec4(r, g, b, a);
}

//----------------------------------------------------------------------------
void Material::SetAmbientColor(const Vec4& ambient)
{
	mMaterialConstants.gAmbientColor = ambient;
}

//----------------------------------------------------------------------------
void Material::SetDiffuseColor(float r, float g, float b, float a)
{
	mMaterialConstants.gDiffuseColor = Vec4(r, g, b, a);
}

//----------------------------------------------------------------------------
void Material::SetDiffuseColor(const Vec4& diffuse)
{
	mMaterialConstants.gDiffuseColor = diffuse;
}

//----------------------------------------------------------------------------
void Material::SetSpecularColor(float r, float g, float b, float shine)
{
	mMaterialConstants.gSpecularColor = Vec4(r, g, b, shine);
}

//----------------------------------------------------------------------------
void Material::SetSpecularColor(const Vec4& specular)
{
	mMaterialConstants.gSpecularColor = specular;
}

//----------------------------------------------------------------------------
void Material::SetEmissiveColor(float r, float g, float b, float strength)
{
	mMaterialConstants.gEmissiveColor = Vec4(r, g, b, strength);
}

//----------------------------------------------------------------------------
void Material::SetEmissiveColor(const Vec4& emissive)
{
	mMaterialConstants.gEmissiveColor = emissive;
}

//----------------------------------------------------------------------------
void Material::SetTexture(const char* filepath, BINDING_SHADER shader, int slot, 
	const SAMPLER_DESC& samplerDesc)
{
	ITexture* pTexture = 0;
	TextureSignature signature(TEXTURE_TYPE_DEFAULT, filepath, 0);
	bool same = FindTextureIn(shader, slot, &pTexture, &signature);

	if (same)
	{
		assert(pTexture);
		return;
	}

	if (!pTexture)
	{
		pTexture = gFBEnv->pEngine->GetRenderer()->CreateTexture(filepath);
		if (pTexture)
		{
			pTexture->SetSlot(slot);
			pTexture->SetShaderStage(shader);
			pTexture->SetType(TEXTURE_TYPE_DEFAULT);
			mTextures.push_back(pTexture);
		}
	}
}

void Material::SetTexture(ITexture* pTexture, BINDING_SHADER shader,  int slot,
			const SAMPLER_DESC& samplerDesc)
{	
	ITexture* pTextureInSlot = 0;
	bool same = FindTextureIn(shader, slot, &pTextureInSlot);
	if (pTextureInSlot!=pTexture)
	{
		RemoveTexture(pTextureInSlot);
	}

	if (pTexture)
	{
		pTexture->SetSlot(slot);
		pTexture->SetShaderStage(shader);
		//pTexture->SetSamplerDesc(samplerDesc);
		pTexture->SetType(TEXTURE_TYPE_DEFAULT);
		mTextures.push_back(pTexture);
	}
}

ITexture* Material::GetTexture(BINDING_SHADER shader, int slot)
{
	auto it = mTextures.begin(), itEnd = mTextures.end();
	for (; it != itEnd; it++)
	{
		if ((*it)->GetSlot() == slot && (*it)->GetShaderStage() == shader)
		{
			return  (*it);
		}
	}
	return 0;
}

void Material::SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc)
{
	ITexture* pTexture = 0;
	TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP, 0, &cr);
	bool same = FindTextureIn(shader, slot, &pTexture, &signature);

	if (same)
	{
		assert(pTexture);
		mColorRampMap[pTexture] = cr;
		RefreshColorRampTexture(slot, shader);
	}
	
	if (!pTexture)
	{
		pTexture = CreateColorRampTexture(cr);
		pTexture->SetSlot(slot);
		pTexture->SetShaderStage(shader);
		//pTexture->SetSamplerDesc(samplerDesc);
		pTexture->SetType(TEXTURE_TYPE_COLOR_RAMP);
		mTextures.push_back(pTexture);
	}
}

//----------------------------------------------------------------------------
ColorRamp& Material::GetColorRamp(int slot, BINDING_SHADER shader)
{
	ITexture* pTexture = 0;
	TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP, 0, 0);
	FindTextureIn(shader, slot, &pTexture, &signature);
	assert(pTexture->GetType() == TEXTURE_TYPE_COLOR_RAMP);

	return mColorRampMap[pTexture];

}

//----------------------------------------------------------------------------
void Material::RefreshColorRampTexture(int slot, BINDING_SHADER shader)
{
	ITexture* pTexture = 0;
	FindTextureIn(shader, slot, &pTexture);
	assert(pTexture!=0 && pTexture->GetType() == TEXTURE_TYPE_COLOR_RAMP);

	ColorRamp cr = mColorRampMap[pTexture];
	
	MapData data = pTexture->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
	if (data.pData)
	{
		// bar position is already updated. generate ramp texture data.
		cr.GenerateColorRampTextureData(128);
		
		unsigned int *pixels = (unsigned int*)data.pData;
		for(unsigned x = 0; x < 128; x++) 
		{
			pixels[127-x] = cr[x].Get4Byte();
		}
		pTexture->Unmap(0);
	}
	//RemoveTexture(pTexture);

	mColorRampMap[pTexture] = cr;
}

//----------------------------------------------------------------------------
ITexture* Material::CreateColorRampTexture(ColorRamp& cr)
{
	cr.GenerateColorRampTextureData(128);
	FIBITMAP* bitmap = FreeImage_Allocate(128, 1, 32);
	if (!bitmap)
	{
		gFBEnv->pEngine->Log(FB_DEFAULT_DEBUG_ARG, "Failed to create freeimage!");
		assert(0);
		return 0;
	}
	unsigned int *pixels = (unsigned int *)FreeImage_GetScanLine(bitmap, 0);
	for(unsigned x = 0; x < 128; x++) 
	{
		pixels[127-x] = cr[x].Get4Byte();
	}

	ITexture* pTexture = gFBEnv->pEngine->GetRenderer()->CreateTexture(pixels, 128, 1, PIXEL_FORMAT_R8G8B8A8_UNORM,
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_DEFAULT); // default is right.
	FreeImage_Unload(bitmap);

	mColorRampMap.insert(COLOR_RAMP_MAP_TYPE::value_type(pTexture, cr));
	return pTexture;
}

//----------------------------------------------------------------------------
void Material::RemoveTexture(ITexture* pTexture)
{
	if (!pTexture)
		return;

	mColorRampMap.erase(pTexture);
	mTextures.erase(std::remove(mTextures.begin(), mTextures.end(), pTexture), mTextures.end());
}

void Material::RemoveTexture(BINDING_SHADER shader, int slot)
{
	ITexture* pTexture = 0;
	FindTextureIn(shader, slot, &pTexture);
	RemoveTexture(pTexture);
}

//----------------------------------------------------------------------------
bool Material::AddShaderDefine(const char* name, const char* val)
{
	if (name==0 || val==0)
		return false;
	for (auto& d : mShaderDefines)
	{
		if (d.name==name)
		{
			if (d.value != val)
			{
				d.value = val;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	mShaderDefines.push_back(ShaderDefine());
	mShaderDefines.back().name = name;
	mShaderDefines.back().value = val;
	return true;
}

bool Material::RemoveShaderDefine(const char* def)
{
	if (def == 0)
		return false;

	FB_FOREACH(it, mShaderDefines)
	{
		if (strcmp(it->name.c_str(), def) == 0)
		{
			it = mShaderDefines.erase(it);
			return true;
		}		
	}	
	return false;
}

void Material::ApplyShaderDefines()
{
	assert(mAdamMaterial);
	mShader = gFBEnv->pEngine->GetRenderer()->CreateShader(
		mAdamMaterial->GetShaderFile(), mShaders, mShaderDefines);
}

//----------------------------------------------------------------------------
void Material::SetMaterialParameters(unsigned index, const Vec4& value)
{
	mMaterialParameters[index] = value;
}

//----------------------------------------------------------------------------
const Vec4& Material::GetAmbientColor() const
{
	return mMaterialConstants.gAmbientColor;
}

//----------------------------------------------------------------------------
const Vec4& Material::GetDiffuseColor() const
{
	return mMaterialConstants.gDiffuseColor;
}

//----------------------------------------------------------------------------
const Vec4& Material::GetSpecularColor() const
{
	return mMaterialConstants.gSpecularColor;
}

//----------------------------------------------------------------------------
const Vec4& Material::GetEmissiveColor() const
{
	return mMaterialConstants.gEmissiveColor;
}

//----------------------------------------------------------------------------
const char* Material::GetShaderFile() const
{
	if (mShader)
		return mShader->GetName();

	return 0;
}

//----------------------------------------------------------------------------
void* Material::GetShaderByteCode(unsigned& size) const
{
	if (!mShader)
	{
		size = 0;
		return 0;
	}
	void* p = mShader->GetVSByteCode(size);
	return p;
}

//----------------------------------------------------------------------------
const Vec4& Material::GetMaterialParameters(unsigned index)
{
	auto it = mMaterialParameters.Find(index);
	if (it == mMaterialParameters.end())
	{
		mMaterialParameters.Insert(std::make_pair(index, Vec4::ZERO));
		return Vec4::ZERO;
	}
	return it->second;
}

bool Material::IsUsingMaterialParameter(unsigned index){
	auto it = mMaterialParameters.Find(index);
	return it != mMaterialParameters.end();
}

//----------------------------------------------------------------------------
bool Material::IsRelatedShader(const char* shaderFile)
{
	if (mShader)
	{
		if (strcmp(shaderFile, mShader->GetName()) == 0 || mShader->CheckIncludes(shaderFile))
		{
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
void Material::Bind(bool inputLayout, unsigned stencilRef)
{
	mRenderStates->Bind(stencilRef);

	auto const renderer = gFBEnv->_pInternalRenderer;
	if (mReloading && mReloadingTryCount < 10)
	{
		if (LoadFromFile(mName.c_str()))
		{
			mReloading = false;
			mReloadingTryCount = 0;
		}
		else
		{
			mReloadingTryCount++;
		}
	}
	if (!mShader)
	{
		assert(0);
		mShader = renderer->CreateShader(
			mShaderFile.c_str(), mShaders, mShaderDefines);
	}

	if (mShader)
	{
		mShader->Bind();
	}

	if (!mInputLayout && inputLayout)
	{
		mInputLayout = renderer->GetInputLayout(mInputElementDescs, this);
	}
	if (mInputLayout && inputLayout)
	{
		mInputLayout->Bind();
	}
	renderer->UpdateMaterialConstantsBuffer(&mMaterialConstants);

	BindMaterialParams();	

	auto it = mTextures.begin(),
		itEnd = mTextures.end();
	for( ; it!=itEnd; it++)
	{
		(*it)->Bind();
	}

	if (mGlow)
	{
		auto rt = renderer->GetCurRenderTarget();
		if (rt->GetRenderPipeline().GetStep(RenderSteps::Glow))
			rt->SetGlowRenderTarget();
	}
}

void Material::Unbind()
{
	if (mGlow)
	{
		auto rt = gFBEnv->pRenderer->GetCurRenderTarget();
		if (rt->GetRenderPipeline().GetStep(RenderSteps::Glow))
			rt->UnSetGlowRenderTarget();
	}
}

//------------------------------------------------------
IMaterial* Material::GetSubPassMaterial(RENDER_PASS p) const
{
	FB_FOREACH(it, mSubMaterials)
	{
		Material* pmat = (Material*)(*it).get();
		if (pmat->mRenderPass == p)
			return pmat;
	}
	return 0;
}

//------------------------------------------------------
bool Material::BindSubPass(RENDER_PASS p, bool includeInputLayout)
{
	IMaterial* pmat = GetSubPassMaterial(p);
	if (pmat){
		pmat->Bind(includeInputLayout);
		BindMaterialParams(); // use parent material param
		return true;
	}

	return false;
}

void Material::BindMaterialParams()
{
	if (!mMaterialParameters.empty())
	{
		Vec4* pDest = (Vec4*)gFBEnv->pEngine->GetRenderer()->MapMaterialParameterBuffer();
		if (pDest)
		{
			auto it = mMaterialParameters.begin(), itEnd = mMaterialParameters.end();
			for (; it != itEnd; it++)
			{
				Vec4* pCurDest = pDest + it->first;
				memcpy(pCurDest, &(it->second), sizeof(Vec4));
			}
			gFBEnv->pEngine->GetRenderer()->UnmapMaterialParameterBuffer();
		}
	}
}

//----------------------------------------------------------------------------
void Material::RegisterReloading()
{
	if (mAdamMaterial == 0)
	{
		FB_FOREACH(it, mMaterials)
		{
			if ((*it)->GetAdam() == this)
				(*it)->RegisterReloading();
		}
	}
	mReloading = true;
	mReloadingTryCount = 0;
	mInputLayout = 0;
}

void Material::ReloadShader()
{
	if (!mShader)
		return;
	mShader = gFBEnv->pEngine->GetRenderer()->CreateShader(mShader->GetName(), mShaders, mShaderDefines);
}

void Material::CopyMaterialParamFrom(const IMaterial* src)
{
	assert(src);
	const Material* pMat = (const Material*)src;
	mMaterialParameters = pMat->mMaterialParameters;
}

void Material::CopyMaterialConstFrom(const IMaterial* src){
	const Material* srcMat = (const Material*)src;
	mMaterialConstants = srcMat->mMaterialConstants;
}

void Material::CopyTexturesFrom(const IMaterial* src){
	const Material* srcMat = (const Material*)src;
	mTextures = srcMat->mTextures;
}
void Material::CopyShaderDefinesFrom(const IMaterial* src){
	const Material* srcMat = (const Material*)src;
	if (mShaderDefines != srcMat->mShaderDefines){
		mShaderDefines = srcMat->mShaderDefines;
		ApplyShaderDefines();
	}
}

void Material::SetTransparent(bool trans)
{
	mTransparent = trans;
}

void Material::SetRasterizerState(const RASTERIZER_DESC& desc)
{
	mRenderStates->CreateRasterizerState(desc);
}

void Material::SetBlendState(const BLEND_DESC& desc)
{
	mRenderStates->CreateBlendState(desc);
}

void Material::SetDepthStencilState(const DEPTH_STENCIL_DESC& desc)
{
	mRenderStates->CreateDepthStencilState(desc);
}

void Material::ClearRasterizerState()
{
	mRenderStates->ResetRasterizerState();
}
void Material::ClearBlendState(const BLEND_DESC& desc)
{
	mRenderStates->ResetBlendState();
}
void Material::ClearDepthStencilState(const DEPTH_STENCIL_DESC& desc)
{
	mRenderStates->ResetDepthStencilState();
}

void Material::CloneRenderStates()
{
	if (!mRenderStatesCloned)
	{
		mRenderStates = mRenderStates->Clone();
		mRenderStatesCloned = true;
	}
}

void Material::SetInputLayout(const INPUT_ELEMENT_DESCS& desc){
	if (!(mInputElementDescs < desc || desc < mInputElementDescs))
		return;

	mInputElementDescs = desc;
	mInputLayout = 0;
}

unsigned IMaterial::GetMaxMaterialParameter(){
	return 5;
}