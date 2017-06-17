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
#include "Material.h"
#include "Shader.h"
#include "InputLayout.h"
#include "RenderStates.h"
#include "Renderer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "EssentialEngineData/shaders/Constants.h"
#include "FBCommonHeaders/CowPtr.h"
#include "FBDebugLib/DebugLib.h"
#include "FBMathLib/ColorRamp.h"
#include "FBStringLib/StringLib.h"
#include "FBStringLib/StringConverter.h"
#include "TinyXmlLib/tinyxml2.h"
#include "FBFileSystem/FileSystem.h"


using namespace fb;
class Material::Impl{
public:
	struct TextureSignature
	{
		TextureSignature(int texture_type, const char* filepath, const ColorRamp* cr)
			: mTextureType(texture_type), mColorRamp(cr)
		{
			if (filepath){
				mFilepath = filepath;
				ToLowerCase(mFilepath);
			}
		}
		int mTextureType;
		std::string mFilepath;
		const ColorRamp* mColorRamp;
	};
	
	//---------------------------------------------------------------------------
	// Data - Copy On Write.
	//---------------------------------------------------------------------------
	struct SharedData {
		SharedData()			
			: mRenderPass(RENDER_PASS::PASS_NORMAL)			
		{
		}
		std::string mName;		
		std::vector<MaterialPtr> mSubMaterials;
		RENDER_PASS mRenderPass;		
	};

	typedef std::map<TexturePtr, ColorRamp> ColorRampsByTexture;
	struct MaterialData{
		MaterialData() {
			memset(&mMaterialConstants, 0, sizeof(mMaterialConstants));
		}
		bool operator != (const MaterialData& other) const {
			return mMaterialConstants != other.mMaterialConstants ||
				mShaderConstants != other.mShaderConstants ||
				mTextures != other.mTextures ||
				mTextureByBinding != other.mTextureByBinding ||
				mColorRampMap != other.mColorRampMap ||
				mSystemTextures != other.mSystemTextures;
		}
		MATERIAL_CONSTANTS mMaterialConstants;
		Parameters mShaderConstants;
		Textures mTextures;
		TextureByBinding mTextureByBinding;
		std::vector<SystemTextures::Enum> mSystemTextures;
		ColorRampsByTexture mColorRampMap;		
	};

	struct RenderStatesData{
		RenderStatesData()
			: mRenderStates(RenderStates::Create())
			, mTransparent(false)
			, mGlow(false)
			, mNoShadowCast(false), mDoubleSided(false)
			, mPrimitiveTopology(PRIMITIVE_TOPOLOGY_UNKNOWN)
			, mResetRasterizer(false)
			, mResetDepthStencil(false)
			, mResetBlend(false)
			, mResetPrimitiveTopology(false)			
		{
		}

		bool operator != (const RenderStatesData& other) const {
			return mTransparent != other.mTransparent ||
				mGlow != other.mGlow ||
				mNoShadowCast != other.mNoShadowCast ||
				mDoubleSided != other.mDoubleSided ||
				*mRenderStates.const_get() != *other.mRenderStates.const_get() ||
				mPrimitiveTopology != other.mPrimitiveTopology ||
				mResetRasterizer != other.mResetRasterizer ||
				mResetDepthStencil != other.mResetDepthStencil ||
				mResetBlend != other.mResetBlend ||
				mResetPrimitiveTopology != other.mResetPrimitiveTopology;
		}

		CowPtr<RenderStates> mRenderStates;
		PRIMITIVE_TOPOLOGY mPrimitiveTopology;		
		bool mTransparent;
		bool mGlow;
		bool mNoShadowCast;
		bool mDoubleSided;
		bool mResetRasterizer;
		bool mResetDepthStencil;
		bool mResetBlend;
		bool mResetPrimitiveTopology;
	};

	struct ShaderData{
		ShaderData() : mShaders(0), mShaderDefinesChanged(true), mInputDescChanged(true)
		{
		}
		ShaderData(const ShaderData& other)
			: mInputElementDescs(other.mInputElementDescs)
			, mShaders(other.mShaders)
			, mShaderFile(other.mShaderFile)
			, mShaderDefines(other.mShaderDefines)
			, mShader(other.mShader)
			, mShaderDefinesChanged(other.mShaderDefinesChanged)
			, mInputLayout(other.mInputLayout)
			, mInputDescChanged(other.mInputDescChanged)
		{
		}

		bool operator != (const ShaderData& other) const {
			return mInputElementDescs != other.mInputElementDescs ||
				mInputLayout != other.mInputLayout ||
				mShaders != other.mShaders ||
				mShaderFile != other.mShaderFile ||
				mShaderDefines != other.mShaderDefines ||
				mShader != other.mShader ||
				mShaderDefinesChanged != other.mShaderDefinesChanged ||
				mInputDescChanged != other.mInputDescChanged;
		}

		INPUT_ELEMENT_DESCS mInputElementDescs;
		InputLayoutPtr mInputLayout;
		int mShaders; // combination of enum SHADER_TYPE;
		// INTEGRATED_SHADER,SHADER_TYPE_VS,SHADER_TYPE_HS,SHADER_TYPE_DS,SHADER_TYPE_GS,SHADER_TYPE_PS,SHADER_TYPE_CS		
		std::string mShaderFile;		
		SHADER_DEFINES mShaderDefines;
		ShaderPtr mShader;
		bool mShaderDefinesChanged;
		bool mInputDescChanged;
	};

	// shared all across the cloned materials but its unique. 
	// i.e data is the same in the all across the cloned materials.
	std::shared_ptr<SharedData> mUniqueData; 
	CowPtr<MaterialData> mMaterialData;
	CowPtr<RenderStatesData> mRenderStatesData;
	CowPtr<ShaderData> mShaderData;
	std::vector<MaterialWeakPtr> mClonedMaterials;	
	
	bool mCloned;
	int mDebug;
	//bool mMarkNoShaderDefineChanges;

	//---------------------------------------------------------------------------
	Impl()
		: mUniqueData(new SharedData)
		, mMaterialData(new MaterialData)
		, mRenderStatesData(new RenderStatesData)
		, mShaderData(new ShaderData)				
		, mCloned(false)
		, mDebug(0)
		//, mMarkNoShaderDefineChanges(false)
	{
	}

	Impl(const Impl& other)
		: mUniqueData(other.mUniqueData)
		, mMaterialData(other.mMaterialData)
		, mRenderStatesData(other.mRenderStatesData)
		, mShaderData(other.mShaderData)		
		, mCloned(true)
		, mDebug(other.mDebug)
		//, mMarkNoShaderDefineChanges(false)
	{
	}

	bool operator==(const Impl& other) const {
		return !(mUniqueData != other.mUniqueData ||
			mMaterialData.const_get() != other.mMaterialData.const_get() ||
			mRenderStatesData.const_get() != other.mRenderStatesData.const_get() ||
			mShaderData.const_get() != other.mShaderData.const_get() );
	}

	bool LoadFromFile(const char* filepath)
	{
		if (!filepath)
			return false;
		mUniqueData->mName = filepath;
		auto pdoc = FileSystem::LoadXml(filepath);		
		if (pdoc->Error())
		{			
			if (pdoc->ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Material(%s) not found.", filepath).c_str());
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to load a Material(%s)", filepath).c_str());
				const char* errMsg = pdoc->GetErrorStr1();
				if (errMsg)
					Logger::Log("\t%s\n", errMsg);
				errMsg = pdoc->GetErrorStr2();
				if (errMsg)
					Logger::Log("\t%s\n", errMsg);
			}			
			pdoc = FileSystem::LoadXml("EssentialEngineData/materials/missing.material");									
			if (pdoc->Error())
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Loading the fallback material is also failed.");
				return false;
			}
		}

		tinyxml2::XMLElement* pRoot = pdoc->FirstChildElement("Material");
		if (!pRoot)
		{
			assert(0);
			return false;
		}

		return LoadFromXml(pRoot, filepath);
	}

	void ParseDepthStencilFace(tinyxml2::XMLElement* elem, DEPTH_STENCILOP_DESC& desc) {
		const char* sz;
		sz = elem->Attribute("StencilPassOp");
		if (sz){
			desc.SetStencilPassOp(StencilOpFromString(sz));
		}
		sz = elem->Attribute("StencilFailOp");
		if (sz){
			desc.SetStencilFailOp(StencilOpFromString(sz));
		}

		sz = elem->Attribute("StencilDepthFailOp");
		if (sz) {
			desc.SetStencilDepthFailOp(StencilOpFromString(sz));
		}

		sz = elem->Attribute("StencilFunc");
		if (sz){
			desc.SetStencilFunc(ComparisonFuncFromString(sz));
		}
	}

	bool LoadFromXml(tinyxml2::XMLElement* pRoot, const char* filepath)
	{
		const char* sz = pRoot->Attribute("transparent");
		auto renderStatesData = const_cast<RenderStatesData*>(mRenderStatesData.const_get());
		if (sz)
		{
			renderStatesData->mTransparent = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("glow");
		if (sz)
		{
			renderStatesData->mGlow = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("noShadowCast");
		if (sz)
		{
			renderStatesData->mNoShadowCast = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("doubleSided");
		if (sz)
		{
			renderStatesData->mDoubleSided = StringConverter::ParseBool(sz);
		}

		auto ptElem = pRoot->FirstChildElement("PrimitiveTopology");
		if (ptElem) {
			sz = ptElem->GetText();
			if (sz) {
				renderStatesData->mPrimitiveTopology = PrimitiveTopologyFromString(sz);
			}
			auto sz = ptElem->Attribute("ResetAtUnbind");
			if (sz) {
				renderStatesData->mResetPrimitiveTopology = StringConverter::ParseBool(sz);
			}
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
				bdesc.RenderTarget[0].BlendEnable = StringConverter::ParseBool(sz);
			if (bdesc.RenderTarget[0].BlendEnable)
			{
				renderStatesData->mTransparent = true;
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

			sz = blendDescElem->Attribute("ResetAtUnbind");
			if (sz) {
				renderStatesData->mResetBlend= StringConverter::ParseBool(sz);
			}
		}
		else
		{
			if (renderStatesData->mTransparent)
			{
				bdesc.RenderTarget[0].BlendEnable = true;
				bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
				bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
				bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
			}
		}
		auto renderStates = const_cast<RenderStates*>(renderStatesData->mRenderStates.const_get());
		renderStates->CreateBlendState(bdesc);
		//-----------------------------------------------------------------------------
		// DepthStencilDesc
		//-----------------------------------------------------------------------------
		DEPTH_STENCIL_DESC ddesc;
		auto depthDescElem = pRoot->FirstChildElement("DepthStencilDesc");
		if (depthDescElem)
		{
			sz = depthDescElem->Attribute("DepthEnable");
			if (sz)
				ddesc.SetDepthEnable(StringConverter::ParseBool(sz));

			sz = depthDescElem->Attribute("DepthWriteMask");
			if (sz)
				ddesc.SetDepthWriteMask(DepthWriteMaskFromString(sz));

			sz = depthDescElem->Attribute("DepthFunc");
			if (sz)
				ddesc.SetDepthFunc(ComparisonFuncFromString(sz));

			sz = depthDescElem->Attribute("ResetAtUnbind");
			if (sz) {
				renderStatesData->mResetDepthStencil = StringConverter::ParseBool(sz);
			}

			sz = depthDescElem->Attribute("StencilEnable");
			if (sz) {
				ddesc.SetStencilEnable(StringConverter::ParseBool(sz));
			}

			sz = depthDescElem->Attribute("StencilReadMask");
			if (sz) {
				ddesc.SetStencilReadMask( StringConverter::ParseInt(sz));
			}

			sz = depthDescElem->Attribute("StencilWriteMask");
			if (sz) {
				ddesc.SetStencilWriteMask(StringConverter::ParseInt(sz));
			}
			auto frontFaceElem = depthDescElem->FirstChildElement("FrontFace");
			if (frontFaceElem) {
				ParseDepthStencilFace(frontFaceElem, ddesc.GetFrontFace());
			}
			auto backFaceElem = depthDescElem->FirstChildElement("BackFace");
			if (backFaceElem) {
				ParseDepthStencilFace(backFaceElem, ddesc.GetBackFace());
			}			
		}
		else
		{
			ddesc.SetDepthWriteMask(renderStatesData->mTransparent ? DEPTH_WRITE_MASK_ZERO : DEPTH_WRITE_MASK_ALL);
		}
		renderStates->CreateDepthStencilState(ddesc);

		//-----------------------------------------------------------------------------
		auto debugElem = pRoot->FirstChildElement("Debug");
		if (debugElem) {
			auto szValue = debugElem->Attribute("value");
			if (szValue) {
				mDebug = StringConverter::ParseInt(szValue);
			}
		}

		//-----------------------------------------------------------------------------
		// RasterizerDesc
		//-----------------------------------------------------------------------------
		RASTERIZER_DESC rdesc;
		auto rasterizerDescElem = pRoot->FirstChildElement("RasterizerDesc");
		if (rasterizerDescElem)
		{
			sz = rasterizerDescElem->Attribute("FillMode");
			if (sz)
				rdesc.SetFillMode(FillModeFromString(sz));

			sz = rasterizerDescElem->Attribute("CullMode");
			if (sz)
				rdesc.SetCullMode(CullModeFromString(sz));

			sz = rasterizerDescElem->Attribute("ScissorEnable");
			if (sz)
				rdesc.SetScissorEnable(StringConverter::ParseBool(sz));

			sz = rasterizerDescElem->Attribute("DepthBias");
			if (sz)
				rdesc.SetDepthBias(StringConverter::ParseInt(sz));

			sz = rasterizerDescElem->Attribute("SlopeScaledDepthBias");
			if (sz)
				rdesc.SetSlopeScaledDepthBias(StringConverter::ParseReal(sz));

			sz = rasterizerDescElem->Attribute("FrontCounterClockwise");
			if (sz)
				rdesc.SetFrontCounterClockwise(StringConverter::ParseBool(sz));

			sz = rasterizerDescElem->Attribute("ResetAtUnbind");
			if (sz) {
				renderStatesData->mResetRasterizer = StringConverter::ParseBool(sz);
			}
		}
		else
		{
			rdesc.SetCullMode(renderStatesData->mDoubleSided ? CULL_MODE_NONE : CULL_MODE_BACK);
		}
		renderStates->CreateRasterizerState(rdesc);

		sz = pRoot->Attribute("pass");
		if (sz)
			mUniqueData->mRenderPass = (RENDER_PASS)RenderPassFromString(sz);

		auto materialData = const_cast<MaterialData*>(mMaterialData.const_get());
		tinyxml2::XMLElement* pMaterialConstants = pRoot->FirstChildElement("MaterialConstants");
		if (pMaterialConstants)
		{
			tinyxml2::XMLElement* pAmbientElem = pMaterialConstants->FirstChildElement("AmbientColor");
			if (pAmbientElem)
			{
				const char* szAmbient = pAmbientElem->GetText();
				Vec4 ambient(szAmbient);
				materialData->mMaterialConstants.gAmbientColor = ambient;
			}
			tinyxml2::XMLElement* pDiffuseElem = pMaterialConstants->FirstChildElement("DiffuseColor_Alpha");
			if (pDiffuseElem)
			{
				const char* szDiffuse = pDiffuseElem->GetText();
				Vec4 diffuse(szDiffuse);
				materialData->mMaterialConstants.gDiffuseColor = diffuse;
			}
			tinyxml2::XMLElement* pSpecularElem = pMaterialConstants->FirstChildElement("SpecularColor_Shine");
			if (pSpecularElem)
			{
				const char* szSpecular = pSpecularElem->GetText();
				Vec4 specular(szSpecular);
				materialData->mMaterialConstants.gSpecularColor = specular;				
			}
			tinyxml2::XMLElement* pEmissiveElem = pMaterialConstants->FirstChildElement("EmissiveColor_Strength");
			if (pEmissiveElem)
			{
				const char* szEmissive = pEmissiveElem->GetText();
				Vec4 emissive(szEmissive);
				materialData->mMaterialConstants.gEmissiveColor = emissive;				
			}
		}

		tinyxml2::XMLElement* pShaderConstants = pRoot->FirstChildElement("ShaderConstants");		
		if (pShaderConstants)
		{
			materialData->mShaderConstants.clear();
			tinyxml2::XMLElement* pElem = pShaderConstants->FirstChildElement();
			int i = 0;
			while (pElem)
			{
				const char* szVector = pElem->GetText();
				if (szVector)
				{
					Vec4 v(szVector);
					materialData->mShaderConstants.insert(Parameters::value_type(i++, v));
				}
				pElem = pElem->NextSiblingElement();
			}
		}

		tinyxml2::XMLElement* pDefines = pRoot->FirstChildElement("ShaderDefines");		
		auto shaderData = const_cast<ShaderData*>(mShaderData.const_get());		
		if (pDefines)
		{
			shaderData->mShaderDefines.clear();
			shaderData->mShaderDefinesChanged = true;
			tinyxml2::XMLElement* pElem = pDefines->FirstChildElement();
			int i = 0;
			while (pElem)
			{
				std::string strName, strValue;
				const char* pname = pElem->Attribute("name");
				if (pname)
					strName = pname;
				const char* pval = pElem->Attribute("val");
				if (pval)
					strValue = pval;
				
				shaderData->mShaderDefines.push_back(ShaderDefine(strName.c_str(), strValue.c_str()));
				pElem = pElem->NextSiblingElement();
			}
		}

		tinyxml2::XMLElement* pTexturesElem = pRoot->FirstChildElement("Textures");
		materialData->mSystemTextures.clear();
		if (pTexturesElem)
		{
			tinyxml2::XMLElement* pTexElem = pTexturesElem->FirstChildElement("Texture");
			while (pTexElem)
			{
				auto sz = pTexElem->Attribute("SystemTexture");
				if (sz) {
					auto systemTexture = SystemTextures::ConvertToEnum(sz);
					if (!ValueExistsInVector(materialData->mSystemTextures, systemTexture))
						materialData->mSystemTextures.push_back(systemTexture);
				}
				else {
					sz = pTexElem->GetText();
					std::string filepath = sz ? sz : "";
					int slot = 0;
					SHADER_TYPE shader = SHADER_TYPE_PS;
					SAMPLER_DESC samplerDesc;
					pTexElem->QueryIntAttribute("slot", &slot);
					const char* szShader = pTexElem->Attribute("shader");
					shader = BindingShaderFromString(szShader);
					const char* szAddressU = pTexElem->Attribute("AddressU");
					samplerDesc.SetAddressU(AddressModeFromString(szAddressU));
					const char* szAddressV = pTexElem->Attribute("AddressV");
					samplerDesc.SetAddressV(AddressModeFromString(szAddressV));
					const char* szFilter = pTexElem->Attribute("Filter");
					samplerDesc.SetFilter(FilterFromString(szFilter));
					TexturePtr pTextureInTheSlot;
					const char* szType = pTexElem->Attribute("type");
					int texture_type = TEXTURE_TYPE_DEFAULT;
					if (szType)
						texture_type = TextureTypeFromString(szType);
					ColorRamp cr;
					if (texture_type & TEXTURE_TYPE_COLOR_RAMP)
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
						SetColorRampTexture(cr, shader, slot, samplerDesc, true);
					}
					else
					{
						if (!filepath.empty() && !FileSystem::ResourceExists(filepath.c_str())) {
							auto textureFileName = FileSystem::GetFileName(filepath.c_str());
							std::string materialParentPath = FileSystem::GetParentPath(mUniqueData->mName.c_str());
							bool stripped = true;
							do{
								filepath = FileSystem::StripFirstDirectoryPath(filepath.c_str(), &stripped);
								std::string alternativePath = materialParentPath + "/" + filepath;
								if (FileSystem::ResourceExists(alternativePath.c_str())) {
									filepath = alternativePath;
									break;
								}
							} while (stripped && filepath != textureFileName);
						}
						SetTexture(filepath.c_str(), shader, slot, samplerDesc, texture_type, true);
					}
				}
				pTexElem = pTexElem->NextSiblingElement("Texture");
			}
		}

		shaderData->mShaders = SHADER_TYPE_VS | SHADER_TYPE_PS;
		tinyxml2::XMLElement* pShaders = pRoot->FirstChildElement("Shaders");
		if (pShaders)
		{
			const char* szShaders = pShaders->GetText();
			if (szShaders)
			{
				auto& shaders = shaderData->mShaders;
				shaders = 0;
				std::string strShaders = szShaders;
				ToLowerCase(strShaders);
				if (strShaders.find("vs") != std::string::npos)
				{
					shaders |= SHADER_TYPE_VS;
				}
				if (strShaders.find("hs") != std::string::npos)
				{
					shaders |= SHADER_TYPE_HS;
				}
				if (strShaders.find("ds") != std::string::npos)
				{
					shaders |= SHADER_TYPE_DS;
				}
				if (strShaders.find("gs") != std::string::npos)
				{
					shaders |= SHADER_TYPE_GS;
				}
				if (strShaders.find("ps") != std::string::npos)
				{
					shaders |= SHADER_TYPE_PS;
				}
			}
		}

		auto& renderer = Renderer::GetInstance();
		tinyxml2::XMLElement* pShaderFileElem = pRoot->FirstChildElement("ShaderFile");
		if (pShaderFileElem)
		{
			const char* shaderFile = pShaderFileElem->GetText();
			if (shaderFile)
			{
				// INTEGRATED_SHADER,SHADER_TYPE_VS,SHADER_TYPE_HS,SHADER_TYPE_DS,SHADER_TYPE_GS,SHADER_TYPE_PS,SHADER_TYPE_CS		
				shaderData->mShaderFile = shaderFile;
				shaderData->mShaderFile += ",,,,,,";
			}
		}
		else {
			auto& shaders = shaderData->mShaders;
			shaders = 0;
			std::string vs, hs, ds, gs, ps, cs;
			auto shaderFilesElem = pRoot->FirstChildElement("SeperatedShaderFiles");
			if (shaderFilesElem) {
				auto elem = shaderFilesElem->FirstChildElement("VSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						vs = sz;
						shaders |= SHADER_TYPE_VS;
					}
				}
				elem = shaderFilesElem->FirstChildElement("HSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						hs = sz;
						shaders |= SHADER_TYPE_HS;
					}
				}
				elem = shaderFilesElem->FirstChildElement("DSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						ds = sz;
						shaders |= SHADER_TYPE_DS;
					}
				}
				elem = shaderFilesElem->FirstChildElement("GSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						gs = sz;
						shaders |= SHADER_TYPE_GS;
					}
				}
				elem = shaderFilesElem->FirstChildElement("PSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						ps = sz;
						shaders |= SHADER_TYPE_PS;
					}
				}
				elem = shaderFilesElem->FirstChildElement("CSFile");
				if (elem) {
					sz = elem->GetText();
					if (sz) {
						cs = sz;
						shaders |= SHADER_TYPE_CS;
					}
				}
				shaderData->mShaderFile = FormatString("%s,%s,%s,%s,%s,%s,%s",
					"",
					vs.c_str(), hs.c_str(), ds.c_str(),
					gs.c_str(), ps.c_str(), cs.c_str());
			}
		}		

		tinyxml2::XMLElement* pInputLayoutElem = pRoot->FirstChildElement("InputLayout");
		auto& inputElementDesc = shaderData->mInputElementDescs;
		inputElementDesc.clear();		
		if (pInputLayoutElem)
		{
			shaderData->mInputDescChanged = true;
			tinyxml2::XMLElement* pElem = pInputLayoutElem->FirstChildElement();
			int i = 0;
			while (pElem)
			{
				inputElementDesc.push_back(INPUT_ELEMENT_DESC());
				const char* pbuffer = pElem->Attribute("semantic");
				if (pbuffer)
					strcpy_s(inputElementDesc.back().mSemanticName, pbuffer);
				inputElementDesc.back().mSemanticIndex = pElem->IntAttribute("index");

				pbuffer = pElem->Attribute("format");
				if (pbuffer)
				{
					inputElementDesc.back().mFormat = InputElementFromString(pbuffer);
				}

				inputElementDesc.back().mInputSlot = pElem->IntAttribute("slot");

				inputElementDesc.back().mAlignedByteOffset = pElem->IntAttribute("alignedByteOffset");

				pbuffer = pElem->Attribute("inputSlotClass");
				if (pbuffer)
					inputElementDesc.back().mInputSlotClass = InputClassificationFromString(pbuffer);

				inputElementDesc.back().mInstanceDataStepRate = pElem->IntAttribute("stepRate");

				pElem = pElem->NextSiblingElement();
			}
		}
		ApplyShaderDefines();

		tinyxml2::XMLElement* subMat = pRoot->FirstChildElement("Material");
		auto& subMaterials = mUniqueData->mSubMaterials;
		subMaterials.clear();
		while (subMat)
		{
			MaterialPtr pMat = Material::Create();
			subMaterials.push_back(pMat);
			pMat->LoadFromXml(subMat);

			subMat = subMat->NextSiblingElement("Material");
		}

		return true;
	}

	void Reload() {
		// cloned materials same with its parent also be affected because of the CowPtr.
		LoadFromFile(mUniqueData->mName.c_str());
	}

	const char* GetName() const { 
		return mUniqueData->mName.c_str(); 
	}

	void SetAmbientColor(float r, float g, float b, float a)
	{
		mMaterialData->mMaterialConstants.gAmbientColor = Vec4(r, g, b, a);
	}

	//----------------------------------------------------------------------------
	void SetAmbientColor(const Vec4& ambient)
	{
		mMaterialData->mMaterialConstants.gAmbientColor = ambient;
	}

	//----------------------------------------------------------------------------
	void SetDiffuseColor(float r, float g, float b, float a)
	{
		mMaterialData->mMaterialConstants.gDiffuseColor = Vec4(r, g, b, a);
	}

	//----------------------------------------------------------------------------
	void SetDiffuseColor(const Vec4& diffuse)
	{
		mMaterialData->mMaterialConstants.gDiffuseColor = diffuse;
	}

	//----------------------------------------------------------------------------
	void SetSpecularColor(float r, float g, float b, float shine)
	{
		mMaterialData->mMaterialConstants.gSpecularColor = Vec4(r, g, b, shine);
	}

	//----------------------------------------------------------------------------
	void SetSpecularColor(const Vec4& specular)
	{
		mMaterialData->mMaterialConstants.gSpecularColor = specular;
	}

	//----------------------------------------------------------------------------
	void SetEmissiveColor(float r, float g, float b, float strength)
	{
		mMaterialData->mMaterialConstants.gEmissiveColor = Vec4(r, g, b, strength);
	}

	//----------------------------------------------------------------------------
	void SetEmissiveColor(const Vec4& emissive)
	{
		mMaterialData->mMaterialConstants.gEmissiveColor = emissive;
	}

	//----------------------------------------------------------------------------
	void SetTexture(const char* filepath, SHADER_TYPE shader, int slot,
		const SAMPLER_DESC& samplerDesc, int texture_type, bool loading)
	{
		MaterialData* materialData;
		if (loading) // loading or reloading
			materialData = const_cast<MaterialData*>(mMaterialData.const_get());
		else
			materialData = mMaterialData.get();

		TexturePtr pTexture;
		TextureSignature signature(TEXTURE_TYPE_DEFAULT, filepath, 0);
		bool same = FindTextureIn(shader, slot, pTexture, &signature);

		if (same)
		{
			assert(pTexture);
			return;
		}

		auto& renderer = Renderer::GetInstance();
		TextureCreationOption option;
		option.textureType = texture_type;
		pTexture = renderer.CreateTexture(filepath, option);
		if (pTexture)
		{			
			materialData->mTextures.push_back(pTexture);
			TextureBinding binding = { shader, slot };
			materialData->mTextureByBinding[binding] = pTexture;
		}
	}

	void SetTexture(TexturePtr pTexture, SHADER_TYPE shader, int slot,
		const SAMPLER_DESC& samplerDesc, bool loading)
	{
		MaterialData* materialData;
		if (loading) // loading or reloading
			materialData = const_cast<MaterialData*>(mMaterialData.const_get());
		else
			materialData = mMaterialData.get();

		TexturePtr pTextureInSlot;
		bool same = FindTextureIn(shader, slot, pTextureInSlot);
		if (pTextureInSlot && pTextureInSlot != pTexture)
		{
			RemoveTexture(pTextureInSlot);
		}

		if (pTexture)
		{			
			materialData->mTextures.push_back(pTexture);
			TextureBinding binding = { shader, slot };
			materialData->mTextureByBinding[binding] = pTexture;
		}
	}

	TexturePtr GetTexture(SHADER_TYPE shader, int slot) const
	{
		TextureBinding binding{ shader, slot };
		auto it = mMaterialData->mTextureByBinding.find(binding);
		if (it != mMaterialData->mTextureByBinding.end()){
			return it->second;
		}
		return 0;
	}

	void SetColorRampTexture(ColorRamp& cr, SHADER_TYPE shader, int slot,
		const SAMPLER_DESC& samplerDesc, bool loading)
	{
		MaterialData* materialData;
		if (loading) // loading or reloading
			materialData = const_cast<MaterialData*>(mMaterialData.const_get());
		else
			materialData = mMaterialData.get();

		TexturePtr pTexture = 0;
		TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP & TEXTURE_TYPE_1D, 0, &cr);
		bool same = FindTextureIn(shader, slot, pTexture, &signature);

		if (same)
		{
			assert(pTexture);
			materialData->mColorRampMap[pTexture] = cr;
			RefreshColorRampTexture(slot, shader);
		}

		if (!pTexture)
		{
			pTexture = CreateColorRampTexture(cr);
			TextureBinding binding{ shader, slot };						
			materialData->mTextures.push_back(pTexture);
			materialData->mTextureByBinding[binding] = pTexture;
		}
	}

	//----------------------------------------------------------------------------
	ColorRamp& GetColorRamp(int slot, SHADER_TYPE shader)
	{
		TexturePtr texture = 0;
		TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP | TEXTURE_TYPE_1D, 0, 0);
		FindTextureIn(shader, slot, texture, &signature);
		assert(texture->GetType() & TEXTURE_TYPE_COLOR_RAMP);

		return mMaterialData->mColorRampMap[texture];
	}

	//----------------------------------------------------------------------------
	void RefreshColorRampTexture(int slot, SHADER_TYPE shader)
	{
		TexturePtr pTexture;
		FindTextureIn(shader, slot, pTexture);
		assert(pTexture != 0 && pTexture->GetType() & TEXTURE_TYPE_COLOR_RAMP);

		ColorRamp cr = mMaterialData->mColorRampMap[pTexture];

		MapData data = pTexture->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (data.pData)
		{
			// bar position is already updated. generate ramp texture data.
			cr.GenerateColorRampTextureData(128, 0.f);

			unsigned int *pixels = (unsigned int*)data.pData;
			for (unsigned x = 0; x < 128; x++)
			{
				pixels[127 - x] = cr[x].Get4Byte();
			}
			pTexture->Unmap(0);
		}	

		mMaterialData->mColorRampMap[pTexture] = cr;
	}
	//----------------------------------------------------------------------------
	void RemoveTexture(TexturePtr pTexture)
	{
		if (!pTexture)
			return;
		auto& textures = mMaterialData->mTextures;
		DeleteValuesInVector(textures, pTexture);
		mMaterialData->mColorRampMap.erase(pTexture);

		auto& textureByBinding = mMaterialData->mTextureByBinding;
		for (auto it = textureByBinding.begin(); it != textureByBinding.end(); /**/){
			if (it->second == pTexture){
				it = textureByBinding.erase(it);
			}
			else{
				++it;
			}
		}		
	}

	void RemoveTexture(SHADER_TYPE shader, int slot)
	{
		auto it = mMaterialData->mTextureByBinding.find({ shader, slot });
		if (it != mMaterialData->mTextureByBinding.end()){
			RemoveTexture(it->second);
			return;
		}
		TexturePtr pTexture;
		FindTextureIn(shader, slot, pTexture);
		RemoveTexture(pTexture);
		Logger::Log(FB_ERROR_LOG_ARG, "A texture is removed in the fallback method.");
	}

	bool AddShaderDefine(const char* name, const char* val)
	{
		if (name == 0 || val == 0)
			return false;
		
		{
			auto& shaderDefines = mShaderData->mShaderDefines;
			auto it = std::find_if(shaderDefines.cbegin(), shaderDefines.cend(), [&](const ShaderDefine& sd){
				return sd.GetName() == name;
			});
			if (it != shaderDefines.end()){
				if (it->GetValue() == val){
					return false; // already exists
				}
				else{
					shaderDefines.erase(it);					
				}				
			}			
		}

		auto& shaderDefines = mShaderData->mShaderDefines;
		shaderDefines.push_back(ShaderDefine(name, val));
		mShaderData->mShaderDefinesChanged = true;
		return true;
	}

	bool RemoveShaderDefine(const char* def)
	{
		if (def == 0)
			return false;
		// exists?
		{
			const auto& currentDefines = mShaderData.const_get()->mShaderDefines;
			auto found = std::find_if(currentDefines.cbegin(), currentDefines.cend(), [&def](const ShaderDefine& define){
				return define.GetName() == def;
			});
			if (found == currentDefines.end())
				return false;
		}
		// clone here.
		auto& shaderDefines = mShaderData->mShaderDefines;
		shaderDefines.erase(
			std::remove_if(shaderDefines.begin(), shaderDefines.end(), [&def](ShaderDefine& define){
			return define.GetName() == def;			
		}), shaderDefines.end());		
		mShaderData->mShaderDefinesChanged = true;

		return true;
	}

	bool HasShaderDefines(const char* def) {
		const auto& currentDefines = mShaderData.const_get()->mShaderDefines;
		auto found = std::find_if(currentDefines.cbegin(), currentDefines.cend(), [&def](const ShaderDefine& define) {
			return define.GetName() == def;
		});
		return found == currentDefines.end();
	}

	void ApplyShaderDefines()
	{
		auto shaderData = const_cast<ShaderData*>(mShaderData.const_get());
		if (shaderData->mShaderDefinesChanged){
			// Retrieveing with out cloning.
			// When mShaderDefinesChanged is changed it is already cloned.			
			shaderData->mShaderDefinesChanged = false;
			auto& renderer = Renderer::GetInstance();			
			std::sort(shaderData->mShaderDefines.begin(), shaderData->mShaderDefines.end());
			auto shaderFiles = Split(shaderData->mShaderFile.c_str());
			if (!shaderFiles.empty()) {
				if (!shaderFiles[0].empty()) {
					shaderData->mShader = renderer.CreateShader(
						shaderFiles[0].c_str(), shaderData->mShaders, shaderData->mShaderDefines);
				}
				else {
					shaderFiles.erase(shaderFiles.begin());
					shaderData->mShader = renderer.CreateShader(shaderFiles, shaderData->mShaderDefines);
				}
				if (!shaderData->mInputLayout) {
					shaderData->mInputDescChanged = true;
				}
			}
			else {
				Logger::Log(FB_ERROR_LOG_ARG, "No shader file.");
			}
		}
		if (shaderData->mInputDescChanged){	
			shaderData->mInputDescChanged = false;
			auto& renderer = Renderer::GetInstance();
			if (!shaderData->mInputElementDescs.empty())
				mShaderData->mInputLayout = renderer.CreateInputLayout(shaderData->mInputElementDescs, shaderData->mShader);
		}
	}


	const SHADER_DEFINES& GetShaderDefines() const { 
		return mShaderData->mShaderDefines; 
	}

	/*void MarkNoShaderDefineChanges(){
		mMarkNoShaderDefineChanges = true;
	}*/

	void DebugPrintShaderDefines() const{
		Logger::Log(FormatString("(info) DebugPrintShaderDefines for %s\n", mUniqueData->mName.c_str()).c_str());
		for (auto& it : mShaderData.const_get()->mShaderDefines){
			Logger::Log(FormatString("%s : %s\n", it.GetName().c_str(), it.GetValue().c_str()).c_str());
		}
		Logger::Log("---- End ----\n");
	}

	void DebugPrintRenderStates(const char* prefix, const Material* self) const{
		if (!prefix)
			return;
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) [%s] address(0x%x) name(%s), cloned(%d)", 
			prefix, self, mUniqueData->mName.c_str(), mCloned ? 1 : 0).c_str());
		mRenderStatesData->mRenderStates->DebugPrint();
	}

	//----------------------------------------------------------------------------
	const Vec4f& GetAmbientColor() const
	{
		return mMaterialData->mMaterialConstants.gAmbientColor;
	}

	//----------------------------------------------------------------------------
	const Vec4f& GetDiffuseColor() const
	{
		return mMaterialData->mMaterialConstants.gDiffuseColor;
	}

	//----------------------------------------------------------------------------
	const Vec4f& GetSpecularColor() const
	{
		return mMaterialData->mMaterialConstants.gSpecularColor;
	}

	//----------------------------------------------------------------------------
	const Vec4f& GetEmissiveColor() const
	{
		return mMaterialData->mMaterialConstants.gEmissiveColor;
	}

	//----------------------------------------------------------------------------
	const char* GetShaderFile() const
	{
		return mShaderData->mShaderFile.c_str();
	}

	//----------------------------------------------------------------------------
	void* GetShaderByteCode(unsigned& size) const
	{
		if (!mShaderData->mShader)
		{
			size = 0;
			return 0;
		}
		void* p = mShaderData->mShader->GetVSByteCode(size);
		return p;
	}

	//----------------------------------------------------------------------------
	void SetShaderParameter(unsigned index, const Vec4& value)
	{
		mMaterialData->mShaderConstants[index] = value;
	}

	const Vec4f& GetShaderParameter(unsigned index) const
	{
		auto& params = mMaterialData->mShaderConstants;
		auto it = params.find(index);
		if (it == params.end())
		{
			return Vec4f::ZERO;
		}
		return it->second;
	}

	const Parameters& GetShaderParameters() const
	{
		return mMaterialData->mShaderConstants;		
	}

	const MATERIAL_CONSTANTS& GetMaterialConstants() const {
		return mMaterialData->mMaterialConstants;
	}

	const Textures& GetTextures() const {
		return mMaterialData->mTextures;
	}

	const TextureByBinding& GetTextureByBinding() const {
		return mMaterialData->mTextureByBinding;
	}

	bool IsUsingMaterialParameter(unsigned index) const{
		auto it = mMaterialData->mShaderConstants.find(index);
		return it != mMaterialData->mShaderConstants.end();
	}

	//----------------------------------------------------------------------------
	bool IsRelatedShader(const char* shaderFile) const
	{
		auto shader = mShaderData->mShader;
		if (shader) {
			if (shader->IsRelatedFile(shaderFile)) {
				return true;
			}
		}
		return false;
	}

	//----------------------------------------------------------------------------
	bool Bind(bool inputLayout, int stencilRef)
	{
		auto shaderData = mShaderData.const_get();
		if (!shaderData->mShader || shaderData->mShaderDefinesChanged || shaderData->mInputDescChanged ||
			(inputLayout && !shaderData->mInputLayout))
		{
			ApplyShaderDefines();
		}

		auto renderStatesData = mRenderStatesData.const_get();
		renderStatesData->mRenderStates->Bind(stencilRef);
		
		auto& renderer = Renderer::GetInstance();
		if (renderStatesData->mPrimitiveTopology != PRIMITIVE_TOPOLOGY_UNKNOWN) {
			renderer.SetPrimitiveTopology(renderStatesData->mPrimitiveTopology);
		}

		if (shaderData->mShader)
		{
			if (shaderData->mShader->GetCompileFailed())
				return false;

			shaderData->mShader->Bind(true);
		}

		if (shaderData->mInputLayout && inputLayout)
		{
			shaderData->mInputLayout->Bind();
		}
		auto materialData = mMaterialData.const_get();
		renderer.UpdateMaterialConstantsBuffer(&materialData->mMaterialConstants);

		BindShaderConstants();

		auto& textures = materialData->mTextureByBinding;
		for (auto it : textures){
			it.second->Bind(it.first.mShader, it.first.mSlot);
		}		

		for (auto it : materialData->mSystemTextures) {
			renderer.BindSystemTexture(it);
		}

		
		if (renderStatesData->mGlow)
		{
			auto rt = renderer.GetCurrentRenderTarget();
			if (rt->IsGlowSupported())
				rt->GlowRenderTarget(true);
		}
		return true;
	}

	void Unbind()
	{
		auto& renderer = Renderer::GetInstance();
		auto renderStatesData = mRenderStatesData.const_get();
		if (renderStatesData->mGlow)
		{			
			auto rt = renderer.GetCurrentRenderTarget();
			if (rt->IsGlowSupported())
				rt->GlowRenderTarget(false);
		}

		if (renderStatesData->mResetBlend) {
			renderer.RestoreBlendState();
		}

		if (renderStatesData->mResetDepthStencil) {
			renderer.RestoreDepthStencilState();
		}

		if (renderStatesData->mResetRasterizer) {
			renderer.RestoreRasterizerState();
		}

		if (renderStatesData->mResetPrimitiveTopology) {
			renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}

	//------------------------------------------------------
	MaterialPtr GetSubPassMaterial(RENDER_PASS p) const
	{
		for (auto mat : mUniqueData->mSubMaterials){
			if (mat->GetRenderPass() == p)
				return mat;
		}
		return 0;
	}

	//------------------------------------------------------
	bool BindSubPass(RENDER_PASS p, bool includeInputLayout) const
	{
		auto pmat = GetSubPassMaterial(p);
		if (pmat){
			pmat->Bind(includeInputLayout);
			BindShaderConstants(); // use parent material param
			return true;
		}

		return false;
	}

	void BindShaderConstants() const
	{
		const auto& materialParams = mMaterialData->mShaderConstants;
		if (!materialParams.empty())
		{
			auto& renderer = Renderer::GetInstance();
			Vec4f* pDest = (Vec4f*)renderer.MapShaderConstantsBuffer();
			if (pDest)
			{
				auto it = materialParams.begin(), itEnd = materialParams.end();
				for (; it != itEnd; it++)
				{
					Vec4f* pCurDest = pDest + it->first;
					const auto& src = it->second;
					*pCurDest = src;
				}
				renderer.UnmapShaderConstantsBuffer();
			}
		}
	}

	void CopyMaterialData(MaterialConstPtr src)
	{
		if (!src)
			return;
		mMaterialData = src->mImpl->mMaterialData;
	}

	void SetTransparent(bool trans)
	{
		mRenderStatesData->mTransparent = trans;
	}

	void SetGlow(bool glow) { 
		mRenderStatesData->mGlow = glow; 
	}

	void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY to) {
		mRenderStatesData->mPrimitiveTopology = to;
	}

	bool IsTransparent() const { 
		return mRenderStatesData->mTransparent;
	}

	bool IsGlow() const { 
		return mRenderStatesData->mGlow;
	}

	bool IsNoShadowCast() const { 
		return mRenderStatesData->mNoShadowCast;
	}
	bool IsDoubleSided() const { 
		return mRenderStatesData->mDoubleSided;
	}

	int GetBindingShaders() const { 
		return mShaderData->mShaders; 
	}

	void CopyMaterialParamFrom(MaterialConstPtr src) {
		mMaterialData->mShaderConstants = src->GetShaderParameters();
	}

	void CopyMaterialConstFrom(MaterialConstPtr src) {
		mMaterialData->mMaterialConstants = src->GetMaterialConstants();
	}

	void CopyTexturesFrom(MaterialConstPtr src) {
		mMaterialData->mTextures = src->GetTextures();
		mMaterialData->mTextureByBinding = src->GetTextureByBinding();
	}

	void CopyShaderDefinesFrom(const MaterialConstPtr src) {
		mShaderData->mShaderDefines = src->GetShaderDefines();
		mShaderData->mShaderDefinesChanged = true;
	}

	void SetRasterizerState(const RASTERIZER_DESC& desc)
	{
		mRenderStatesData->mRenderStates->CreateRasterizerState(desc);
	}

	void SetBlendState(const BLEND_DESC& desc)
	{
		mRenderStatesData->mRenderStates->CreateBlendState(desc);
	}

	void SetDepthStencilState(const DEPTH_STENCIL_DESC& desc)
	{
		mRenderStatesData->mRenderStates->CreateDepthStencilState(desc);
	}

	RenderStatesPtr GetRenderStates() const { 
		return mRenderStatesData->mRenderStates.data();
	}

	RENDER_PASS GetRenderPass() const {
		return mUniqueData->mRenderPass;
	}

	void ClearRasterizerState()
	{
		mRenderStatesData->mRenderStates->ResetRasterizerState();
	}
	void ClearBlendState(const BLEND_DESC& desc)
	{
		mRenderStatesData->mRenderStates->ResetBlendState();
	}
	void ClearDepthStencilState(const DEPTH_STENCIL_DESC& desc)
	{
		mRenderStatesData->mRenderStates->ResetDepthStencilState();
	}

	void SetInputLayout(const INPUT_ELEMENT_DESCS& desc){
		
		auto& constInputLayoutDesc = mShaderData.const_get()->mInputElementDescs;
		if (constInputLayoutDesc == desc){
			return;
		}

		mShaderData->mInputElementDescs = desc;
		mShaderData->mInputDescChanged = true;
	}

	void BindInputLayoutOnly() {
		auto inputLayout = mShaderData.const_get()->mInputLayout;
		if (inputLayout) {
			inputLayout->Bind();
		}
	}

	unsigned IGetMaxMaterialParameter(){
		return 5;
	}

	TexturePtr CreateColorRampTexture(ColorRamp& cr){
		cr.GenerateColorRampTextureData(128, 0.f);
		unsigned imgBuf[128];
		unsigned *pixels = (unsigned int *)imgBuf;
		for (unsigned x = 0; x < 128; x++)
		{
			pixels[127 - x] = cr[x].Get4Byte();
		}
		auto& renderer = Renderer::GetInstance();
		TexturePtr pTexture = renderer.CreateTexture(pixels, 128, 1, PIXEL_FORMAT_R8G8B8A8_UNORM,
			1, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_COLOR_RAMP | TEXTURE_TYPE_1D);
		mMaterialData->mColorRampMap.insert(ColorRampsByTexture::value_type(pTexture, cr));
		return pTexture;
	}

	

	bool FindTextureIn(SHADER_TYPE shader, int slot, TexturePtr& outTextureInTheSlot,
		// additional parameters. if match this function returns true.
		TextureSignature* pSignature = 0) const{
		if (outTextureInTheSlot){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		TextureBinding binding{ shader, slot };
		auto it = mMaterialData->mTextureByBinding.find(binding);
		if (it != mMaterialData->mTextureByBinding.end()){
			outTextureInTheSlot = it->second;
		}		
		if (!outTextureInTheSlot)
			return false;

		if (pSignature)
		{
			if (pSignature->mTextureType != TEXTURE_TYPE_COUNT &&
				pSignature->mTextureType != outTextureInTheSlot->GetType())
				return false;

			std::string targetPath = outTextureInTheSlot->GetFilePath();
			ToLowerCase(targetPath);
			if (pSignature->mFilepath != targetPath)
				return false;

			if (pSignature->mColorRamp)
			{
				auto itFound = mMaterialData->mColorRampMap.find(outTextureInTheSlot);
				if (itFound == mMaterialData->mColorRampMap.end() ||
					!(itFound->second == *pSignature->mColorRamp))
				{
					return false; // not same
				}
			}
		}

		return true; // same
	}
};

//----------------------------------------------------------------------------
MaterialPtr Material::Create(){
	auto p = MaterialPtr(FB_NEW(Material), [](Material* obj){ FB_DELETE(obj); });
	return p;
}

MaterialPtr Material::Create(const Material& other){
	auto p = MaterialPtr(FB_NEW(Material)(other), [](Material* obj){ FB_DELETE(obj); });
	return p;
}

Material::Material()
	: mImpl(new Impl)
{
}

Material::~Material(){
}

Material::Material(const Material& other)
 : mImpl(new Impl(*other.mImpl))
{
}

MaterialPtr Material::Clone() const{
	auto material = Material::Create(*this);
	mImpl->mClonedMaterials.push_back(material);
	return material;
}

bool Material::LoadFromFile(const char* filepath) {
	return mImpl->LoadFromFile(filepath);
}

bool Material::LoadFromXml(tinyxml2::XMLElement* pRoot) {
	return mImpl->LoadFromXml(pRoot, "");
}

const char* Material::GetName() const {
	return mImpl->GetName();
}

void Material::SetAmbientColor(float r, float g, float b, float a) {
	mImpl->SetAmbientColor(r, g, b, a);
}

void Material::SetAmbientColor(const Vec4& ambient) {
	mImpl->SetAmbientColor(ambient);
}

void Material::SetDiffuseColor(float r, float g, float b, float a) {
	mImpl->SetDiffuseColor(r, g, b, a);
}

void Material::SetDiffuseColor(const Vec4& diffuse) {
	mImpl->SetDiffuseColor(diffuse);
}

void Material::SetSpecularColor(float r, float g, float b, float shine) {
	mImpl->SetSpecularColor(r, g, b, shine);
}

void Material::SetSpecularColor(const Vec4& specular) {
	mImpl->SetSpecularColor(specular);
}

void Material::SetEmissiveColor(float r, float g, float b, float strength) {
	mImpl->SetEmissiveColor(r, g, b, strength);
}

void Material::SetEmissiveColor(const Vec4& emissive) {
	mImpl->SetEmissiveColor(emissive);
}

void Material::SetTexture(const char* filepath, SHADER_TYPE shader, int slot, int texture_type, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetTexture(filepath, shader, slot, samplerDesc, texture_type, false);
}

void Material::SetTexture(TexturePtr pTexture, SHADER_TYPE shader, int slot, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetTexture(pTexture, shader, slot, samplerDesc, false);
}

TexturePtr Material::GetTexture(SHADER_TYPE shader, int slot) {
	return mImpl->GetTexture(shader, slot);
}

void Material::SetColorRampTexture(ColorRamp& cr, SHADER_TYPE shader, int slot, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetColorRampTexture(cr, shader, slot, samplerDesc, false);
}

void Material::RemoveTexture(TexturePtr pTexture) {
	mImpl->RemoveTexture(pTexture);
}

void Material::RemoveTexture(SHADER_TYPE shader, int slot) {
	mImpl->RemoveTexture(shader, slot);
}

ColorRamp& Material::GetColorRamp(int slot, SHADER_TYPE shader) {
	return mImpl->GetColorRamp(slot, shader);
}

void Material::RefreshColorRampTexture(int slot, SHADER_TYPE shader) {
	mImpl->RefreshColorRampTexture(slot, shader);
}

bool Material::AddShaderDefine(const char* def, const char* val) {
	return mImpl->AddShaderDefine(def, val);
}

bool Material::RemoveShaderDefine(const char* def) {
	return mImpl->RemoveShaderDefine(def);
}

bool Material::HasShaderDefines(const char* def) {
	return mImpl->HasShaderDefines(def);
}

//void Material::ApplyShaderDefines() {
//	mImpl->ApplyShaderDefines();
//}

void Material::SetShaderParameter(unsigned index, const Vec4& value) {
	mImpl->SetShaderParameter(index, value);
}

const SHADER_DEFINES& Material::GetShaderDefines() const {
	return mImpl->GetShaderDefines();
}

//void Material::MarkNoShaderDefineChanges(){
//	mImpl->MarkNoShaderDefineChanges();
//}

void Material::DebugPrintShaderDefines() const{
	mImpl->DebugPrintShaderDefines();
}

void Material::DebugPrintRenderStates(const char* prefix) const{
	mImpl->DebugPrintRenderStates(prefix, this);
}

const Vec4f& Material::GetAmbientColor() const {
	return mImpl->GetAmbientColor();
}

const Vec4f& Material::GetDiffuseColor() const {
	return mImpl->GetDiffuseColor();
}

const Vec4f& Material::GetSpecularColor() const {
	return mImpl->GetSpecularColor();
}

const Vec4f& Material::GetEmissiveColor() const {
	return mImpl->GetEmissiveColor();
}

const char* Material::GetShaderFile() const {
	return mImpl->GetShaderFile();
}

void* Material::GetShaderByteCode(unsigned& size) const {
	return mImpl->GetShaderByteCode(size);
}

const Vec4f& Material::GetShaderParameter(unsigned index) const {
	return mImpl->GetShaderParameter(index);
}

const Material::Parameters& Material::GetShaderParameters() const {
	return mImpl->GetShaderParameters();
}

const MATERIAL_CONSTANTS& Material::GetMaterialConstants() const {
	return mImpl->GetMaterialConstants();
}

const Material::Textures& Material::GetTextures() const {
	return mImpl->GetTextures();
}

const Material::TextureByBinding& Material::GetTextureByBinding() const {
	return mImpl->GetTextureByBinding();
}

bool Material::IsUsingMaterialParameter(unsigned index) {
	return mImpl->IsUsingMaterialParameter(index);
}

bool Material::IsRelatedShader(const char* shaderFile) {
	return mImpl->IsRelatedShader(shaderFile);
}

bool Material::Bind(){
	return mImpl->Bind(true, 0);
}

bool Material::Bind(bool inputLayout){
	return mImpl->Bind(inputLayout, 0);
}

bool Material::Bind(bool inputLayout, int stencilRef) {
	return mImpl->Bind(inputLayout, stencilRef);
}

bool Material::Bind(int stencilRef) {
	return mImpl->Bind(true, stencilRef);
}

void Material::Unbind() {
	mImpl->Unbind();
}

MaterialPtr Material::GetSubPassMaterial(RENDER_PASS p) const {
	return mImpl->GetSubPassMaterial(p);
}

bool Material::BindSubPass(RENDER_PASS p, bool includeInputLayout) {
	return mImpl->BindSubPass(p, includeInputLayout);
}

void Material::BindShaderConstants() {
	mImpl->BindShaderConstants();
}

void Material::SetTransparent(bool trans) {
	mImpl->SetTransparent(trans);
}

void Material::SetGlow(bool glow) {
	mImpl->SetGlow(glow);
}

void Material::SetPrimitiveTopology(PRIMITIVE_TOPOLOGY topology) {
	mImpl->SetPrimitiveTopology(topology);
}

bool Material::IsTransparent() const {
	return mImpl->IsTransparent();
}

bool Material::IsGlow() const {
	return mImpl->IsGlow();
}

bool Material::IsNoShadowCast() const {
	return mImpl->IsNoShadowCast();
}

bool Material::IsDoubleSided() const {
	return mImpl->IsDoubleSided();
}

int Material::GetBindingShaders() const {
	return mImpl->GetBindingShaders();
}

void Material::CopyMaterialParamFrom(MaterialConstPtr src) {
	mImpl->CopyMaterialParamFrom(src);
}

void Material::CopyMaterialConstFrom(MaterialConstPtr src) {
	mImpl->CopyMaterialConstFrom(src);
}

void Material::CopyTexturesFrom(MaterialConstPtr src) {
	mImpl->CopyTexturesFrom(src);
}

void Material::CopyShaderDefinesFrom(MaterialConstPtr src) {
	mImpl->CopyShaderDefinesFrom(src);
}

void Material::SetRasterizerState(const RASTERIZER_DESC& desc) {
	mImpl->SetRasterizerState(desc);
}

void Material::SetBlendState(const BLEND_DESC& desc) {
	mImpl->SetBlendState(desc);
}

void Material::SetDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
	mImpl->SetDepthStencilState(desc);
}

RenderStatesPtr Material::GetRenderStates() const {
	return mImpl->GetRenderStates();
}

RENDER_PASS Material::GetRenderPass() const {
	return mImpl->GetRenderPass();
}

void Material::ClearRasterizerState() {
	mImpl->ClearRasterizerState();
}

void Material::ClearBlendState(const BLEND_DESC& desc) {
	mImpl->ClearBlendState(desc);
}

void Material::ClearDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
	mImpl->ClearDepthStencilState(desc);
}

void Material::SetInputLayout(const INPUT_ELEMENT_DESCS& desc) {
	mImpl->SetInputLayout(desc);
}

void Material::BindInputLayoutOnly() {
	mImpl->BindInputLayoutOnly();
}

void* Material::GetParameterAddress(){
	return &mImpl->mMaterialData->mShaderConstants;
}

void Material::Reload() {
	mImpl->Reload();
}