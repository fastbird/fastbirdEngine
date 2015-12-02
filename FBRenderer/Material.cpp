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
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/CowPtr.h"
#include "FBMathLib/ColorRamp.h"
#include "FBStringLib/StringLib.h"
#include "FBStringLib/StringConverter.h"
#include "TinyXmlLib/tinyxml2.h"

using namespace fb;
extern BinaryData tempData;
extern unsigned tempSize;
class Material::Impl{
public:
	struct TextureSignature
	{
		TextureSignature(TEXTURE_TYPE type, const char* filepath, const ColorRamp* cr)
			:mType(type), mColorRamp(cr)
		{
			if (filepath){
				mFilepath = filepath;
				ToLowerCase(mFilepath);
			}
		}
		TEXTURE_TYPE mType;
		std::string mFilepath;
		const ColorRamp* mColorRamp;
	};
	typedef std::map<TexturePtr, ColorRamp> ColorRampsByTexture;

	std::mutex mMutex;

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

	struct MaterialData{
		MaterialData() {
			memset(&mMaterialConstants, 0, sizeof(mMaterialConstants));
		}
		MATERIAL_CONSTANTS mMaterialConstants;
		Parameters mMaterialParameters;
		Textures mTextures;
		TexturesByBinding mTexturesByBinding;
		BindingsByTexture mBindingsByTexture;
		ColorRampsByTexture mColorRampMap;		
	};

	struct RenderStatesData{
		RenderStatesData()
			: mRenderStates(RenderStates::Create())
			, mTransparent(false)
			, mGlow(false)
			, mNoShadowCast(false), mDoubleSided(false)
		{
		}
		CowPtr<RenderStates> mRenderStates;
		bool mTransparent;
		bool mGlow;
		bool mNoShadowCast;
		bool mDoubleSided;
	};

	struct ShaderData{
		ShaderData() : mShaders(0)
		{
		}
		ShaderData(const ShaderData& other)
			: mInputElementDescs(other.mInputElementDescs)
			, mShaders(other.mShaders)
			, mShaderFile(other.mShaderFile)
			, mShaderDefines(other.mShaderDefines)
		{
		}
		INPUT_ELEMENT_DESCS mInputElementDescs;
		InputLayoutPtr mInputLayout;
		int mShaders; // combination of enum BINDING_SHADER;
		std::string mShaderFile;
		SHADER_DEFINES mShaderDefines;
		ShaderPtr mShader;
	};

	// shared all across the cloned materials but its unique. 
	// i.e data is the same in the all across the cloned materials.
	std::shared_ptr<SharedData> mUniqueData; 
	CowPtr<MaterialData> mMaterialData;
	CowPtr<RenderStatesData> mRenderStatesData;
	CowPtr<ShaderData> mShaderData;
	bool mShaderDefinesChanged;
	bool mInputDescChanged;


	//---------------------------------------------------------------------------
	Impl()
		: mUniqueData(new SharedData)
		, mMaterialData(new MaterialData)
		, mRenderStatesData(new RenderStatesData)
		, mShaderData(new ShaderData)
		, mShaderDefinesChanged(false)
		, mInputDescChanged(false)
	{
	}

	Impl(const Impl& other)
		: mUniqueData(other.mUniqueData)
		, mMaterialData(other.mMaterialData)
		, mRenderStatesData(other.mRenderStatesData)
		, mShaderData(other.mShaderData)
		, mShaderDefinesChanged(other.mShaderDefinesChanged)
		, mInputDescChanged(other.mInputDescChanged)
	{
	}

	bool LoadFromFile(const char* filepath)
	{
		if (!filepath)
			return false;
		mUniqueData->mName = filepath;
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filepath);
		if (doc.Error())
		{			
			if (doc.ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Material(%s) not found.", filepath).c_str());
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to load a Material(%s)", filepath).c_str());
				const char* errMsg = doc.GetErrorStr1();
				if (errMsg)
					Logger::Log("\t%s", errMsg);
				errMsg = doc.GetErrorStr2();
				if (errMsg)
					Logger::Log("\t%s", errMsg);
			}			
			doc.LoadFile("EssentialEngineData/materials/missing.material");
			if (doc.Error())
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Loading the fallback material is also failed.");
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

	bool LoadFromXml(tinyxml2::XMLElement* pRoot)
	{
		const char* sz = pRoot->Attribute("transparent");
		if (sz)
		{
			mRenderStatesData->mTransparent = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("glow");
		if (sz)
		{
			mRenderStatesData->mGlow = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("noShadowCast");
		if (sz)
		{
			mRenderStatesData->mNoShadowCast = StringConverter::ParseBool(sz);
		}

		sz = pRoot->Attribute("doubleSided");
		if (sz)
		{
			mRenderStatesData->mDoubleSided = StringConverter::ParseBool(sz);
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
				mRenderStatesData->mTransparent = true;
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
			if (mRenderStatesData->mTransparent)
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
				ddesc.DepthEnable = StringConverter::ParseBool(sz);
			sz = depthDescElem->Attribute("DepthWriteMask");
			if (sz)
				ddesc.DepthWriteMask = DepthWriteMaskFromString(sz);
			sz = depthDescElem->Attribute("DepthFunc");
			if (sz)
				ddesc.DepthFunc = ComparisonFuncFromString(sz);
		}
		else
		{
			ddesc.DepthWriteMask = mRenderStatesData->mTransparent ? DEPTH_WRITE_MASK_ZERO : DEPTH_WRITE_MASK_ALL;
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
				rdesc.ScissorEnable = StringConverter::ParseBool(sz);
			sz = rasterizerDescElem->Attribute("DepthBias");
			if (sz)
				rdesc.DepthBias = StringConverter::ParseInt(sz);
		}
		else
		{
			rdesc.CullMode = mRenderStatesData->mDoubleSided ? CULL_MODE_NONE : CULL_MODE_BACK;
		}
		SetRasterizerState(rdesc);


		sz = pRoot->Attribute("pass");
		if (sz)
			mUniqueData->mRenderPass = (RENDER_PASS)RenderPassFromString(sz);

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
		mMaterialData->mMaterialParameters.clear();
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
					mMaterialData->mMaterialParameters.Insert(Parameters::value_type(i++, v));
				}
				pElem = pElem->NextSiblingElement();
			}
		}

		mShaderDefinesChanged = true;
		tinyxml2::XMLElement* pDefines = pRoot->FirstChildElement("ShaderDefines");		
		mShaderData->mShaderDefines.clear();
		if (pDefines)
		{
			tinyxml2::XMLElement* pElem = pDefines->FirstChildElement();
			int i = 0;
			while (pElem)
			{
				mShaderData->mShaderDefines.push_back(ShaderDefine());
				const char* pname = pElem->Attribute("name");
				if (pname)
					mShaderData->mShaderDefines.back().name = pname;
				const char* pval = pElem->Attribute("val");
				if (pval)
					mShaderData->mShaderDefines.back().value = pval;

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
				TexturePtr pTextureInTheSlot;
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

		mShaderData->mShaders = BINDING_SHADER_VS | BINDING_SHADER_PS;
		tinyxml2::XMLElement* pShaders = pRoot->FirstChildElement("Shaders");
		if (pShaders)
		{
			const char* szShaders = pShaders->GetText();
			if (szShaders)
			{
				auto& shaders = mShaderData->mShaders;
				shaders = 0;
				std::string strShaders = szShaders;
				ToLowerCase(strShaders);
				if (strShaders.find("vs") != std::string::npos)
				{
					shaders |= BINDING_SHADER_VS;
				}
				if (strShaders.find("hs") != std::string::npos)
				{
					shaders |= BINDING_SHADER_HS;
				}
				if (strShaders.find("ds") != std::string::npos)
				{
					shaders |= BINDING_SHADER_DS;
				}
				if (strShaders.find("gs") != std::string::npos)
				{
					shaders |= BINDING_SHADER_GS;
				}
				if (strShaders.find("ps") != std::string::npos)
				{
					shaders |= BINDING_SHADER_PS;
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
				mShaderData->mShaderFile = shaderFile;				
			}
		}

		tinyxml2::XMLElement* pInputLayoutElem = pRoot->FirstChildElement("InputLayout");
		auto& inputElementDesc = mShaderData->mInputElementDescs;		
		mInputDescChanged = true;
		if (pInputLayoutElem)
		{
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
		while (subMat)
		{
			MaterialPtr pMat = Material::Create();
			subMaterials.push_back(pMat);
			pMat->LoadFromXml(subMat);

			subMat = subMat->NextSiblingElement("Material");
		}

		return true;
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
	void SetTexture(const char* filepath, BINDING_SHADER shader, int slot,
		const SAMPLER_DESC& samplerDesc)
	{
		TexturePtr pTexture;
		TextureSignature signature(TEXTURE_TYPE_DEFAULT, filepath, 0);
		bool same = FindTextureIn(shader, slot, pTexture, &signature);

		if (same)
		{
			assert(pTexture);
			return;
		}

		if (!pTexture)
		{
			auto& renderer = Renderer::GetInstance();
			pTexture = renderer.CreateTexture(filepath, true);
			if (pTexture)
			{
				pTexture->SetType(TEXTURE_TYPE_DEFAULT);
				mMaterialData->mTextures.push_back(pTexture);
				TextureBinding binding = { shader, slot };
				mMaterialData->mTexturesByBinding[binding] = pTexture;
				mMaterialData->mBindingsByTexture[pTexture] = binding;
			}
		}
	}

	void SetTexture(TexturePtr pTexture, BINDING_SHADER shader, int slot,
		const SAMPLER_DESC& samplerDesc)
	{
		TexturePtr pTextureInSlot;
		bool same = FindTextureIn(shader, slot, pTextureInSlot);
		if (pTextureInSlot && pTextureInSlot != pTexture)
		{
			RemoveTexture(pTextureInSlot);
		}

		if (pTexture)
		{
			pTexture->SetType(TEXTURE_TYPE_DEFAULT);
			mMaterialData->mTextures.push_back(pTexture);
			TextureBinding binding = { shader, slot };
			mMaterialData->mTexturesByBinding[binding] = pTexture;
			mMaterialData->mBindingsByTexture[pTexture] = binding;
		}
	}

	TexturePtr GetTexture(BINDING_SHADER shader, int slot) const
	{
		TextureBinding binding{ shader, slot };
		auto it = mMaterialData->mTexturesByBinding.Find(binding);
		if (it != mMaterialData->mTexturesByBinding.end()){
			return it->second;
		}
		return 0;
	}

	void SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot,
		const SAMPLER_DESC& samplerDesc)
	{
		TexturePtr pTexture = 0;
		TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP, 0, &cr);
		bool same = FindTextureIn(shader, slot, pTexture, &signature);

		if (same)
		{
			assert(pTexture);
			mMaterialData->mColorRampMap[pTexture] = cr;
			RefreshColorRampTexture(slot, shader);
		}

		if (!pTexture)
		{
			pTexture = CreateColorRampTexture(cr);
			TextureBinding binding{ shader, slot };			
			pTexture->SetType(TEXTURE_TYPE_COLOR_RAMP);
			mMaterialData->mTextures.push_back(pTexture);
			mMaterialData->mTexturesByBinding[binding] = pTexture;
			mMaterialData->mBindingsByTexture[pTexture] = binding;
		}
	}

	//----------------------------------------------------------------------------
	ColorRamp& GetColorRamp(int slot, BINDING_SHADER shader)
	{
		TexturePtr texture = 0;
		TextureSignature signature(TEXTURE_TYPE_COLOR_RAMP, 0, 0);
		FindTextureIn(shader, slot, texture, &signature);
		assert(texture->GetType() == TEXTURE_TYPE_COLOR_RAMP);

		return mMaterialData->mColorRampMap[texture];
	}

	//----------------------------------------------------------------------------
	void RefreshColorRampTexture(int slot, BINDING_SHADER shader)
	{
		TexturePtr pTexture;
		FindTextureIn(shader, slot, pTexture);
		assert(pTexture != 0 && pTexture->GetType() == TEXTURE_TYPE_COLOR_RAMP);

		ColorRamp cr = mMaterialData->mColorRampMap[pTexture];

		MapData data = pTexture->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (data.pData)
		{
			// bar position is already updated. generate ramp texture data.
			cr.GenerateColorRampTextureData(128);

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

		auto& bindings = mMaterialData->mBindingsByTexture;
		auto itBinding = bindings.Find(pTexture);
		if (itBinding != bindings.end()){
			auto binding = itBinding->second;
			bindings.erase(itBinding);
			auto& textures = mMaterialData->mTexturesByBinding;
			auto itTexture = textures.Find(binding);
			if (itTexture != textures.end()){
				textures.erase(itTexture);
			}
		}				
	}

	void RemoveTexture(BINDING_SHADER shader, int slot)
	{
		auto it = mMaterialData->mTexturesByBinding.Find({ shader, slot });
		if (it != mMaterialData->mTexturesByBinding.end()){
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
		// check
		{
			auto& shaderDefines = mShaderData.const_get()->mShaderDefines;
			auto it = std::find_if(shaderDefines.cbegin(), shaderDefines.cend(), [&](const ShaderDefine& sd){
				return sd.name == name && sd.value == val;
			});
			if (it != shaderDefines.end())
				return false; // already exists
			
		}
		auto& shaderDefines = mShaderData->mShaderDefines;
		for (auto& d : shaderDefines)
		{
			if (d.name == name)
			{
				if (d.value != val)
				{
					d.value = val;
					mShaderDefinesChanged = true;
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		shaderDefines.push_back(ShaderDefine());
		shaderDefines.back().name = name;
		shaderDefines.back().value = val;
		mShaderDefinesChanged = true;
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
				return define.name == def;
			});
			if (found == currentDefines.end())
				return false;
		}
		auto& shaderDefines = mShaderData->mShaderDefines;

		shaderDefines.erase(
			std::remove_if(shaderDefines.begin(), shaderDefines.end(), [&def](ShaderDefine& define){
			return define.name == def;			
		}), shaderDefines.end());
		mShaderDefinesChanged = true;
		return true;
	}

	void ApplyShaderDefines()
	{
		if (mShaderDefinesChanged){
			mShaderDefinesChanged = false;
			auto& renderer = Renderer::GetInstance();
			std::sort(mShaderData->mShaderDefines.begin(), mShaderData->mShaderDefines.end());
			mShaderData->mShader = renderer.CreateShader(
				mShaderData->mShaderFile.c_str(), mShaderData->mShaders, mShaderData->mShaderDefines);
			if (!mShaderData->mInputLayout){
				mInputDescChanged = true;
			}
		}
		if (mInputDescChanged){
			mInputDescChanged = false;
			auto& renderer = Renderer::GetInstance();
			if (!mShaderData->mInputElementDescs.empty())
				mShaderData->mInputLayout = renderer.CreateInputLayout(mShaderData->mInputElementDescs, mShaderData->mShader);
		}
	}


	const SHADER_DEFINES& GetShaderDefines() const { 
		return mShaderData->mShaderDefines; 
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
	void SetMaterialParameter(unsigned index, const Vec4& value)
	{
		mMaterialData->mMaterialParameters[index] = value;
	}

	const Vec4f& GetMaterialParameter(unsigned index) const
	{
		auto& params = mMaterialData->mMaterialParameters;
		auto it = params.Find(index);
		if (it == params.end())
		{
			return Vec4f::ZERO;
		}
		return it->second;
	}

	const Parameters& GetMaterialParameters() const
	{
		return mMaterialData->mMaterialParameters;		
	}

	const MATERIAL_CONSTANTS& GetMaterialConstants() const {
		return mMaterialData->mMaterialConstants;
	}

	const Textures& GetTextures() const {
		return mMaterialData->mTextures;
	}

	const TexturesByBinding& GetTexturesByBinding() const {
		return mMaterialData->mTexturesByBinding;
	}

	const BindingsByTexture& GetBindingsByTexture() const {
		return mMaterialData->mBindingsByTexture;
	}

	bool IsUsingMaterialParameter(unsigned index) const{
		auto it = mMaterialData->mMaterialParameters.Find(index);
		return it != mMaterialData->mMaterialParameters.end();
	}

	//----------------------------------------------------------------------------
	bool IsRelatedShader(const char* shaderFile) const
	{
		auto shader = mShaderData->mShader;
		if (shader)
		{
			if (strcmp(shaderFile, shader->GetPath()) == 0 || shader->CheckIncludes(shaderFile))
			{
				return true;
			}
		}
		return false;
	}

	//----------------------------------------------------------------------------
	void Bind(bool inputLayout, unsigned stencilRef)
	{
		auto shaderData = mShaderData.const_get();
		if (!shaderData->mShader || mShaderDefinesChanged || mInputDescChanged ||
			(inputLayout && !shaderData->mInputLayout))
		{
			ApplyShaderDefines();
		}

		auto renderStatesData = mRenderStatesData.const_get();
		renderStatesData->mRenderStates->Bind(stencilRef);

		auto& renderer = Renderer::GetInstance();				
		if (shaderData->mShader)
		{
			shaderData->mShader->Bind();
		}

		if (shaderData->mInputLayout && inputLayout)
		{
			shaderData->mInputLayout->Bind();
		}
		auto materialData = mMaterialData.const_get();
		renderer.UpdateMaterialConstantsBuffer(&materialData->mMaterialConstants);

		BindMaterialParams();

		auto& textures = materialData->mBindingsByTexture;
		for (auto it : textures){
			it.first->Bind(it.second.mShader, it.second.mSlot);
		}		

		
		if (renderStatesData->mGlow)
		{
			auto rt = renderer.GetCurrentRenderTarget();
			if (rt->IsGlowSupported())
				rt->GlowRenderTarget(true);
		}
	}

	void Unbind()
	{
		auto renderStatesData = mRenderStatesData.const_get();
		if (renderStatesData->mGlow)
		{
			auto& renderer = Renderer::GetInstance();
			auto rt = renderer.GetCurrentRenderTarget();
			if (rt->IsGlowSupported())
				rt->GlowRenderTarget(false);
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
			BindMaterialParams(); // use parent material param
			return true;
		}

		return false;
	}

	void BindMaterialParams() const
	{
		const auto& materialParams = mMaterialData->mMaterialParameters;
		if (!materialParams.empty())
		{
			auto& renderer = Renderer::GetInstance();
			Vec4f* pDest = (Vec4f*)renderer.MapMaterialParameterBuffer();
			if (pDest)
			{
				auto it = materialParams.begin(), itEnd = materialParams.end();
				for (; it != itEnd; it++)
				{
					Vec4f* pCurDest = pDest + it->first;
					const auto& src = it->second;
					*pCurDest = src;
				}
				renderer.UnmapMaterialParameterBuffer();
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
		mMaterialData->mMaterialParameters = src->GetMaterialParameters();
	}

	void CopyMaterialConstFrom(MaterialConstPtr src) {
		mMaterialData->mMaterialConstants = src->GetMaterialConstants();
	}

	void CopyTexturesFrom(MaterialConstPtr src) {
		mMaterialData->mTextures = src->GetTextures();
		mMaterialData->mBindingsByTexture = src->GetBindingsByTexture();
		mMaterialData->mTexturesByBinding = src->GetTexturesByBinding();
	}

	void CopyShaderDefinesFrom(const MaterialConstPtr src) {
		mShaderData->mShaderDefines = src->GetShaderDefines();
		mShaderDefinesChanged = true;
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
		mInputDescChanged = true;
	}

	unsigned IGetMaxMaterialParameter(){
		return 5;
	}

	TexturePtr CreateColorRampTexture(ColorRamp& cr){
		cr.GenerateColorRampTextureData(128);
		unsigned imgBuf[128];
		unsigned *pixels = (unsigned int *)imgBuf;
		for (unsigned x = 0; x < 128; x++)
		{
			pixels[127 - x] = cr[x].Get4Byte();
		}
		auto& renderer = Renderer::GetInstance();
		TexturePtr pTexture = renderer.CreateTexture(pixels, 128, 1, PIXEL_FORMAT_R8G8B8A8_UNORM,
			BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_DEFAULT); // default is right.
		mMaterialData->mColorRampMap.insert(ColorRampsByTexture::value_type(pTexture, cr));
		return pTexture;
	}

	

	bool FindTextureIn(BINDING_SHADER shader, int slot, TexturePtr& outTextureInTheSlot,
		// additional parameters. if match this function returns true.
		TextureSignature* pSignature = 0) const{
		if (outTextureInTheSlot){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		TextureBinding binding{ shader, slot };
		auto it = mMaterialData->mTexturesByBinding.Find(binding);
		if (it != mMaterialData->mTexturesByBinding.end()){
			outTextureInTheSlot = it->second;
		}		
		if (!outTextureInTheSlot)
			return false;

		if (pSignature)
		{
			if (pSignature->mType != TEXTURE_TYPE_COUNT &&
				pSignature->mType != outTextureInTheSlot->GetType())
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
void ReloadMaterial(const char* name)
{
	// Reloading shaders and textures are supported.
	assert(0 && "Not implemented");
}

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
	return material;
}

bool Material::LoadFromFile(const char* filepath) {
	return mImpl->LoadFromFile(filepath);
}

bool Material::LoadFromXml(tinyxml2::XMLElement* pRoot) {
	return mImpl->LoadFromXml(pRoot);
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

void Material::SetTexture(const char* filepath, BINDING_SHADER shader, int slot, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetTexture(filepath, shader, slot, samplerDesc);
}

void Material::SetTexture(TexturePtr pTexture, BINDING_SHADER shader, int slot, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetTexture(pTexture, shader, slot, samplerDesc);
}

TexturePtr Material::GetTexture(BINDING_SHADER shader, int slot) {
	return mImpl->GetTexture(shader, slot);
}

void Material::SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot, const SAMPLER_DESC& samplerDesc) {
	mImpl->SetColorRampTexture(cr, shader, slot, samplerDesc);
}

void Material::RemoveTexture(TexturePtr pTexture) {
	mImpl->RemoveTexture(pTexture);
}

void Material::RemoveTexture(BINDING_SHADER shader, int slot) {
	mImpl->RemoveTexture(shader, slot);
}

ColorRamp& Material::GetColorRamp(int slot, BINDING_SHADER shader) {
	return mImpl->GetColorRamp(slot, shader);
}

void Material::RefreshColorRampTexture(int slot, BINDING_SHADER shader) {
	mImpl->RefreshColorRampTexture(slot, shader);
}

bool Material::AddShaderDefine(const char* def, const char* val) {
	return mImpl->AddShaderDefine(def, val);
}

bool Material::RemoveShaderDefine(const char* def) {
	return mImpl->RemoveShaderDefine(def);
}

//void Material::ApplyShaderDefines() {
//	mImpl->ApplyShaderDefines();
//}

void Material::SetMaterialParameter(unsigned index, const Vec4& value) {
	mImpl->SetMaterialParameter(index, value);
}

const SHADER_DEFINES& Material::GetShaderDefines() const {
	return mImpl->GetShaderDefines();
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

const Vec4f& Material::GetMaterialParameter(unsigned index) const {
	return mImpl->GetMaterialParameter(index);
}

const Material::Parameters& Material::GetMaterialParameters() const {
	return mImpl->GetMaterialParameters();
}

const MATERIAL_CONSTANTS& Material::GetMaterialConstants() const {
	return mImpl->GetMaterialConstants();
}

const Material::Textures& Material::GetTextures() const {
	return mImpl->GetTextures();
}

const Material::TexturesByBinding& Material::GetTexturesByBinding() const {
	return mImpl->GetTexturesByBinding();
}

const Material::BindingsByTexture& Material::GetBindingsByTexture() const {
	return mImpl->GetBindingsByTexture();
}

bool Material::IsUsingMaterialParameter(unsigned index) {
	return mImpl->IsUsingMaterialParameter(index);
}

bool Material::IsRelatedShader(const char* shaderFile) {
	return mImpl->IsRelatedShader(shaderFile);
}

void Material::Bind(){
	mImpl->Bind(true, 0);
}

void Material::Bind(bool inputLayout){
	mImpl->Bind(inputLayout, 0);
}

void Material::Bind(bool inputLayout, unsigned stencilRef) {
	mImpl->Bind(inputLayout, stencilRef);
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

void Material::BindMaterialParams() {
	mImpl->BindMaterialParams();
}

void Material::SetTransparent(bool trans) {
	mImpl->SetTransparent(trans);
}

void Material::SetGlow(bool glow) {
	mImpl->SetGlow(glow);
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

