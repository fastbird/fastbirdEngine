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
#include "ResourceProvider.h"
#include "ResourceTypes.h"
#include "Renderer.h"
#include "Texture.h"
#include "Shader.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBStringLib/StringLib.h"
using namespace fb;

static const int NumToneMaps = 5;
static const int NumLuminanceMaps = 3;
class ResourceProvider::Impl
{
public:
	VectorMap<int, std::vector<TexturePtr> > mTextures;
	VectorMap<int, ShaderPtr> mShaders;
	VectorMap<int, MaterialPtr> mMaterials;
	VectorMap<int, RasterizerStatePtr> mRasterizerStates;
	VectorMap<int, BlendStatePtr> mBlendStates;
	VectorMap<int, DepthStencilStatePtr> mDepthStencilStates;
	VectorMap<int, SamplerStatePtr> mSamplerStates;	

	//---------------------------------------------------------------------------
	std::vector<TexturePtr> CreateTexture(int ResourceTypes_Textures){
		std::vector<TexturePtr> ret;
		auto& renderer = Renderer::GetInstance();
		switch (ResourceTypes_Textures){		
		case ResourceTypes::Textures::Noise:
		{
			auto texture = renderer.CreateTexture("EssentialEngineData/textures/pnoise.dds");
			if (!texture){
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create noise texture.");				
			}
			else{
				ret.push_back(texture);
			}
			return ret;
		}
		case ResourceTypes::Textures::GGXGenTarget:
		{
			auto texture = renderer.CreateTexture(0, 512, 128, PIXEL_FORMAT_R16G16B16A16_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
			if (!texture){
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create ggx gen target.");				
			}
			else{
				ret.push_back(texture);				
			}
			return ret;
		}	
		case ResourceTypes::Textures::GGX:
		{
			auto texture = renderer.CreateTexture("EssentialEngineData/textures/ggx.dds");
			if (!texture){
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create ggx texture.");
			}
			else{
				ret.push_back(texture);
			}
			return ret;
		}
		case ResourceTypes::Textures::ToneMap:
		{
			int nSampleLen = 1;
			// 1, 3, 9, 27, 81
			for (int i = 0; i < NumToneMaps; ++i){
				auto texture = renderer.CreateTexture(0, nSampleLen, nSampleLen,
					PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
					BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
				if (!texture){
					Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to create a tone map with size (%d)", nSampleLen).c_str());
				}
				else{
					char buff[255];
					sprintf_s(buff, "ToneMap(%d)", nSampleLen);
					texture->SetDebugName(buff);
					nSampleLen *= 3;
					ret.push_back(texture);
				}
			}
			return ret;
		}
		case ResourceTypes::Textures::LuminanceMap:
		{
			for (int i = 0; i < NumLuminanceMaps; ++i){
				auto texture = renderer.CreateTexture(0, 1, 1, PIXEL_FORMAT_R16_FLOAT, BUFFER_USAGE_DEFAULT,
					BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
				if (!texture){
					Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to create luminance map(%d)", i).c_str());
				}
				else{
					ret.push_back(texture);
				}
			}
			return ret;
		}
		default:
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider cannot create the texture(%d)", ResourceTypes_Textures).c_str());
			return ret;
		}
		
		}
	}

	TexturePtr GetTexture(int ResourceTypes_Textures){
		return GetTexture(ResourceTypes_Textures, 0);
	}

	TexturePtr GetTexture(int ResourceTypes_Textures, int index){
		auto it = mTextures.Find(ResourceTypes_Textures);
		if (it == mTextures.end() || index >= (int)it->second.size()){
			auto textures = CreateTexture(ResourceTypes_Textures);
			mTextures[ResourceTypes_Textures] = textures;
			if (index < (int)textures.size()){
				return textures[index];
			}
		}
		else{
			if (index < (int)it->second.size())
				return it->second[index];
		}
		return 0;
	}

	void SwapTexture(int ResourceTypes_Textures, int one, int two){
		auto texture1 = GetTexture(ResourceTypes_Textures, one);
		auto texture2 = GetTexture(ResourceTypes_Textures, two);
		if (texture1 && texture2){
			std::swap(mTextures[ResourceTypes_Textures][one], mTextures[ResourceTypes_Textures][two]);
		}
	}

	ShaderPtr CreateShader(int ResourceTypes_Shaders){
		auto& renderer = Renderer::GetInstance();
		SHADER_DEFINES shaderDefines;
		switch (ResourceTypes_Shaders){
		case ResourceTypes::Shaders::FullscreenQuadNearVS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS);
		}
		case ResourceTypes::Shaders::FullscreenQuadFarVS:
		{
			shaderDefines.push_back(ShaderDefine("_FAR_SIDE_QUAD", "1"));
			return renderer.CreateShader("EssentialEnginedata/shaders/fullscreenquadvs.hlsl", BINDING_SHADER_VS, shaderDefines);			
		}
		case ResourceTypes::Shaders::CopyPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/copyps.hlsl", BINDING_SHADER_PS);			
		}
		case ResourceTypes::Shaders::CopyPSMS:
		{
			shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			return renderer.CreateShader("EssentialEnginedata/shaders/copyps.hlsl", BINDING_SHADER_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::ShadowMapShader:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/shadowdepth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::DepthWriteVSPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/depth.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::DepthOnlyVSPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/DepthOnly.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::CloudDepthWriteVSPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/depth_cloud.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::SampleLumInitialPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));				
			}
			return renderer.CreateShader("EssentialEnginedata/shaders/SampleLumInitialNew.hlsl", BINDING_SHADER_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::SampleLumIterativePS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/SampleLumIterativeNew.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::SampleLumFinalPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/SampleLumFinalNew.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::CalcAdaptedLumPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/CalculateAdaptedLum.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::ToneMappingPS:
		{
			if (renderer.GetMultiSampleCount() != 1)
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));

			if (renderer.GetLuminanaceOnCPU())
				shaderDefines.push_back(ShaderDefine("_CPU_LUMINANCE", "1"));

			if (renderer.GetFilmicToneMapping())
				shaderDefines.push_back(ShaderDefine("_FILMIC_TONEMAP", "1"));
			return renderer.CreateShader("EssentialEnginedata/shaders/tonemapping.hlsl", BINDING_SHADER_PS, shaderDefines);

		}
		case ResourceTypes::Shaders::BrightPassPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEnginedata/shaders/brightpassps.hlsl", BINDING_SHADER_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::BloomPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/bloomps.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::Blur5x5PS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEnginedata/shaders/gaussblur5x5.hlsl", BINDING_SHADER_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::StarGlarePS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/starglare.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::MergeTextures2PS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/mergetextures2ps.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::GodRayPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/GodRayPS.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::OcclusionPrePassVSPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/OccPrePass.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::OcclusionPrePassVSGSPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/OccPrePassGS.hlsl", BINDING_SHADER_VS | BINDING_SHADER_GS | BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::GlowPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEnginedata/shaders/BloomPS.hlsl", BINDING_SHADER_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::SilouettePS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/silouette.hlsl", BINDING_SHADER_PS);
		}
		case ResourceTypes::Shaders::GGXGenPS:
		{
			return renderer.CreateShader("EssentialEnginedata/shaders/GGXGen.hlsl", BINDING_SHADER_PS);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Unknown shader type");
			return 0;
		}
	}

	ShaderPtr GetShader(int ResourceTypes_Shaders){
		auto it = mShaders.Find(ResourceTypes_Shaders);
		if (it == mShaders.end()){
			auto shader = CreateShader(ResourceTypes_Shaders);
			if (!shader){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Failed to create the shader (%d)", ResourceTypes_Shaders).c_str());
				return 0;
			}
			mShaders[ResourceTypes_Shaders] = shader;
			return shader;
		}
		return it->second;
	}

	void BindShader(int ResourceTypes_Shaders){
		auto shader = GetShader(ResourceTypes_Shaders);
		if (shader)
			shader->Bind();
	}

	MaterialPtr CreateMaterial(int ResourceTypes_Materials){
		auto& renderer = Renderer::GetInstance();
		switch (ResourceTypes_Materials){
		case ResourceTypes::Materials::Missing:{
			return renderer.CreateMaterial("EssentialEngineData/materials/missing.material");
		}
		case ResourceTypes::Materials::Quad:{
			return renderer.CreateMaterial("EssentialEngineData/materials/quad.material");
		}
		case ResourceTypes::Materials::QuadTextured:{
			return renderer.CreateMaterial("EssentialEngineData/materials/QuadWithTexture.material");
		}
		case ResourceTypes::Materials::BillboardQuad:{
			return renderer.CreateMaterial("EssentialEngineData/materials/billboardQuad.material");
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Unknown resource type.");
			return 0;
		}
	}

	MaterialPtr GetMaterial(int ResourceTypes_Materials){
		auto it = mMaterials.Find(ResourceTypes_Materials);
		if (it == mMaterials.end()){
			auto material = CreateMaterial(ResourceTypes_Materials);
			if (!material){
				Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Failed to create a material(%d)", ResourceTypes_Materials);
				return 0;
			}
			mMaterials[ResourceTypes_Materials] = material;
			return material;
		}

		return it->second;
	}

	RasterizerStatePtr CreateRasterizerState(int ResourceTypes_RasterizerStates){
		auto& renderer = Renderer::GetInstance();
		RASTERIZER_DESC desc;
		switch (ResourceTypes_RasterizerStates){
		case ResourceTypes::RasterizerStates::Default:
		{
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::CullFrontFace:
		{
			desc.CullMode = CULL_MODE_FRONT;
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::OneBiased:
		{
			desc.DepthBias = 1;
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::WireFrame:
		{
			desc.FillMode = FILL_MODE_WIREFRAME;
			desc.CullMode = CULL_MODE_NONE;
			desc.FrontCounterClockwise = false;
			desc.DepthBias = 0;
			desc.DepthBiasClamp = 0.0f;
			desc.SlopeScaledDepthBias = 0.0f;
			desc.DepthClipEnable = true;
			desc.ScissorEnable = false;
			desc.MultisampleEnable = false;
			desc.AntialiasedLineEnable = false;
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::ShadowMapRS:
		{
			desc.CullMode = CULL_MODE_NONE;
			desc.AntialiasedLineEnable = false;
			return renderer.CreateRasterizerState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown rasterizer states (%d)", ResourceTypes_RasterizerStates).c_str());
			return 0;
		}
	}

	RasterizerStatePtr GetRasterizerState(int ResourceTypes_RasterizerStates){
		auto it = mRasterizerStates.Find(ResourceTypes_RasterizerStates);
		if (it == mRasterizerStates.end()){
			auto rasterizerState = CreateRasterizerState(ResourceTypes_RasterizerStates);
			if (!rasterizerState){
				Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Failed to create a rasterizer state(%d)", ResourceTypes_RasterizerStates);
				return 0;
			}
			mRasterizerStates[ResourceTypes_RasterizerStates] = rasterizerState;
			return rasterizerState;
		}

		return it->second;
	}

	BlendStatePtr CreateBlendState(int ResourceTypes_BlendStates){
		auto& renderer = Renderer::GetInstance();
		BLEND_DESC desc;
		switch (ResourceTypes_BlendStates){
		case ResourceTypes::BlendStates::Default:{
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::Additive:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::AlphaBlend:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::MaxBlend:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_MAX;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::RedAlphaMask:{
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::GreenAlphaMask:{
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::GreenAlphaMaskMax:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_MAX;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::RedAlphaMaskAddMinus:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;

			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_REV_SUBTRACT;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::GreenAlphaMaskAddAdd:{
			BLEND_DESC bdesc;
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;

			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::RedAlphaMaskAddAdd:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;

			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}

		case ResourceTypes::BlendStates::GreenAlphaMaskAddMinus:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;

			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_REV_SUBTRACT;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN | COLOR_WRITE_MASK_ALPHA;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::GreenMask:{
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_GREEN;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::BlueMask:{
			desc.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_MASK_BLUE;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::NoColorWrite:{
			desc.RenderTarget[0].RenderTargetWriteMask = 0;
			return renderer.CreateBlendState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown blend states (%d)", ResourceTypes_BlendStates).c_str());
			return 0;
		}
	}

	BlendStatePtr GetBlendState(int ResourceTypes_BlendStates){
		auto it = mBlendStates.Find(ResourceTypes_BlendStates);
		if (it == mBlendStates.end()){
			auto state = CreateBlendState(ResourceTypes_BlendStates);
			if (!state){
				Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Failed to create a rasterizer state(%d)", ResourceTypes_BlendStates);
				return 0;
			}
			mBlendStates[ResourceTypes_BlendStates] = state;
			return state;
		}

		return it->second;
	}

	DepthStencilStatePtr CreateDepthStencilState(int ResourceTypes_DepthStencilStates){
		auto& renderer = Renderer::GetInstance();
		DEPTH_STENCIL_DESC desc;
		switch (ResourceTypes_DepthStencilStates){
		case ResourceTypes::DepthStencilStates::Default:{
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::NoDepthStencil:{
			desc.DepthEnable = false;
			desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::NoDepthWrite_LessEqual:{
			desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
			desc.DepthFunc = COMPARISON_LESS_EQUAL;
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::LessEqual:{
			desc.DepthFunc = COMPARISON_LESS_EQUAL;
			return renderer.CreateDepthStencilState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown depth stencil states (%d)", ResourceTypes_DepthStencilStates).c_str());
			return 0;
		}
	}

	DepthStencilStatePtr GetDepthStencilState(int ResourceTypes_DepthStencilStates){
		auto it = mDepthStencilStates.Find(ResourceTypes_DepthStencilStates);
		if (it == mDepthStencilStates.end()){
			auto state = CreateDepthStencilState(ResourceTypes_DepthStencilStates);
			if (!state){
				Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Failed to create a depth stencil state(%d)", ResourceTypes_DepthStencilStates);
				return 0;
			}
			mDepthStencilStates[ResourceTypes_DepthStencilStates] = state;
			return state;
		}

		return it->second;
	}

	SamplerStatePtr CreateSamplerState(int ResourceTypes_SamplerStates){
		auto& renderer = Renderer::GetInstance();
		SAMPLER_DESC desc;
		switch (ResourceTypes_SamplerStates){
		case ResourceTypes::SamplerStates::Point:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Linear:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Anisotropic:
		{
			desc.Filter = TEXTURE_FILTER_ANISOTROPIC;
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Shadow:
		{
			desc.Filter = TEXTURE_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;			
			desc.AddressU = TEXTURE_ADDRESS_BORDER;
			desc.AddressV = TEXTURE_ADDRESS_BORDER;
			desc.AddressW = TEXTURE_ADDRESS_BORDER;			
			desc.MaxAnisotropy = 0.f;
			desc.MinLOD = 0.f;
			desc.MaxLOD = 0.f;
			desc.ComparisonFunc = COMPARISON_LESS;				
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::PointWrap:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = TEXTURE_ADDRESS_WRAP;
			desc.AddressV = TEXTURE_ADDRESS_WRAP;
			desc.AddressW = TEXTURE_ADDRESS_WRAP;
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::LinearWrap:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = TEXTURE_ADDRESS_WRAP;
			desc.AddressV = TEXTURE_ADDRESS_WRAP;
			desc.AddressW = TEXTURE_ADDRESS_WRAP;			
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::BlackBorder:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = TEXTURE_ADDRESS_BORDER;
			desc.AddressV = TEXTURE_ADDRESS_BORDER;
			desc.AddressW = TEXTURE_ADDRESS_BORDER;
			for (int i = 0; i < 4; i++)
				desc.BorderColor[i] = 0;
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::PointBlackBorder:
		{
			desc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = TEXTURE_ADDRESS_BORDER;
			desc.AddressV = TEXTURE_ADDRESS_BORDER;
			desc.AddressW = TEXTURE_ADDRESS_BORDER;
			for (int i = 0; i < 4; i++)
				desc.BorderColor[i] = 0;
			return renderer.CreateSamplerState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown sampler states (%d)", ResourceTypes_SamplerStates).c_str());
			return 0;
		}
	}

	SamplerStatePtr GetSamplerState(int ResourceTypes_SamplerStates){
		auto it = mSamplerStates.Find(ResourceTypes_SamplerStates);
		if (it == mSamplerStates.end()){
			auto state = CreateSamplerState(ResourceTypes_SamplerStates);
			if (!state){
				Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Failed to create a depth stencil state(%d)", ResourceTypes_SamplerStates);
				return 0;
			}
			mSamplerStates[ResourceTypes_SamplerStates] = state;
			return state;
		}

		return it->second;
	}

	void BindRasterizerState(int ResourceTypes_RasterizerStates){
		auto state = GetRasterizerState(ResourceTypes_RasterizerStates);
		if (state){
			state->Bind();
		}
	}

	void BindBlendState(int ResourceTypes_BlendStates){
		auto state = GetBlendState(ResourceTypes_BlendStates);
		if (state){
			state->Bind();
		}
	}

	void BindDepthStencilState(int ResourceTypes_DepthStencilStates){
		auto state = GetDepthStencilState(ResourceTypes_DepthStencilStates);
		if (state){
			state->Bind();
		}
	}

	int GetNumToneMaps() const{
		return NumToneMaps;
	}

	int GetNumLuminanceMaps() const{
		return NumLuminanceMaps;
	}

	void DeleteTexture(int ResourceTypes_Textures){
		auto it = mTextures.Find(ResourceTypes_Textures);
		if (it != mTextures.end())
			mTextures.erase(it);
	}

	void DeleteShader(int ResourceTypes_Shaders){
		auto it = mShaders.Find(ResourceTypes_Shaders);
		if (it != mShaders.end())
			mShaders.erase(it);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(ResourceProvider);

ResourceProvider::ResourceProvider()
	:mImpl(new Impl)
{

}

ResourceProvider::~ResourceProvider(){

}

TexturePtr ResourceProvider::GetTexture(int ResourceTypes_Textures) {
	return mImpl->GetTexture(ResourceTypes_Textures);
}

TexturePtr ResourceProvider::GetTexture(int ResourceTypes_Textures, int index) {
	return mImpl->GetTexture(ResourceTypes_Textures, index);
}

void ResourceProvider::SwapTexture(int ResourceTypes_Textures, int one, int two) {
	mImpl->SwapTexture(ResourceTypes_Textures, one, two);
}

ShaderPtr ResourceProvider::GetShader(int ResourceTypes_Shaders) {
	return mImpl->GetShader(ResourceTypes_Shaders);
}

void ResourceProvider::BindShader(int ResourceTypes_Shaders){
	return mImpl->BindShader(ResourceTypes_Shaders);
}

MaterialPtr ResourceProvider::GetMaterial(int ResourceTypes_Materials) {
	return mImpl->GetMaterial(ResourceTypes_Materials);
}

RasterizerStatePtr ResourceProvider::GetRasterizerState(int ResourceTypes_RasterizerStates) {
	return mImpl->GetRasterizerState(ResourceTypes_RasterizerStates);
}

BlendStatePtr ResourceProvider::GetBlendState(int ResourceTypes_BlendStates) {
	return mImpl->GetBlendState(ResourceTypes_BlendStates);
}

DepthStencilStatePtr ResourceProvider::GetDepthStencilState(int ResourceTypes_DepthStencilStates) {
	return mImpl->GetDepthStencilState(ResourceTypes_DepthStencilStates);
}

SamplerStatePtr ResourceProvider::GetSamplerState(int ResourceTypes_SamplerStates) {
	return mImpl->GetSamplerState(ResourceTypes_SamplerStates);
}

void ResourceProvider::BindRasterizerState(int ResourceTypes_RasterizerStates){
	mImpl->BindRasterizerState(ResourceTypes_RasterizerStates);
}

void ResourceProvider::BindBlendState(int ResourceTypes_BlendStates){
	mImpl->BindBlendState(ResourceTypes_BlendStates);
}

void ResourceProvider::BindDepthStencilState(int ResourceTypes_DepthStencilStates){
	mImpl->BindDepthStencilState(ResourceTypes_DepthStencilStates);
}

int ResourceProvider::GetNumToneMaps() const {
	return mImpl->GetNumToneMaps();
}

int ResourceProvider::GetNumLuminanceMaps() const {
	return mImpl->GetNumLuminanceMaps();
}

void ResourceProvider::DeleteTexture(int ResourceTypes_Textures){
	mImpl->DeleteTexture(ResourceTypes_Textures);
}

void ResourceProvider::DeleteShader(int ResourceTypes_Shaders){
	mImpl->DeleteShader(ResourceTypes_Shaders);
}
