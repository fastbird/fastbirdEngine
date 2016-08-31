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
#include "IndexBuffer.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBStringLib/StringLib.h"
#include "FBFileSystem/FileSystem.h"
#include "FBSerializationLib/Serialization.h"
#include "EssentialEngineData/shaders/CommonDefines.h"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
using namespace fb;

static const int NumToneMaps = 5;
static const int NumLuminanceMaps = 3;
class ResourceProvider::Impl
{
public:
	std::unordered_map<int, std::vector<TexturePtr> > mTextures;
	std::unordered_map<int, ShaderPtr> mShaders;
	std::unordered_map<int, MaterialPtr> mMaterials;
	std::unordered_map<int, RasterizerStatePtr> mRasterizerStates;
	std::unordered_map<int, BlendStatePtr> mBlendStates;
	std::unordered_map<int, DepthStencilStatePtr> mDepthStencilStates;
	std::unordered_map<int, SamplerStatePtr> mSamplerStates;
	std::unordered_map<ResourceTypes::IndexBuffer, IndexBufferPtr> mIndexBuffers;

	//---------------------------------------------------------------------------
	std::vector<TexturePtr> CreatePermutationTexture() {
		ByteArray p;
		FileSystem::Open file("EssentialEngineData/Permutation_256_extented.dat", "rb");
		if (file.IsOpen()) {
			auto& data = *file.GetBinaryData();
			if (!data.empty()) {
				typedef boost::iostreams::basic_array_source<char> Device;				
				boost::iostreams::stream_buffer<Device> buffer((char*)&data[0], data.size());
				std::vector<TexturePtr> ret;
				try {
					boost::archive::binary_iarchive ar(buffer);
					ar & p;					
					ret.push_back(Renderer::GetInstance().CreateTexture(&p[0], NUM_PERM + NUM_PERM + 2, 1, PIXEL_FORMAT_R8_UINT,
						1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_1D));
					ret.push_back(Renderer::GetInstance().CreateTexture(&p[0], NUM_PERM + NUM_PERM + 2, 1, PIXEL_FORMAT_R8_UINT,
						1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_1D | TEXTURE_TYPE_SECOND_DEVICE));
				}
				catch (const boost::archive::archive_exception& exc){
					Logger::Log(FB_ERROR_LOG_ARG, exc.what());
				}
				return ret;
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, "Failed to create Permutation_256_Extended texture.");
		return{};
	}

	std::vector<TexturePtr> CreateGradientTexture() {
		std::vector<Vec4> g;
		FileSystem::Open file("EssentialEngineData/Gradiants_256_extended.dat", "rb");
		if (file.IsOpen()) {
			auto& data = *file.GetBinaryData();
			if (!data.empty()) {
				typedef boost::iostreams::basic_array_source<char> Device;
				boost::iostreams::stream_buffer<Device> buffer((char*)&data[0], data.size());
				boost::archive::binary_iarchive ar(buffer);
				ar & g;
				std::vector<TexturePtr> ret;
				ret.push_back(Renderer::GetInstance().CreateTexture(&g[0], NUM_PERM + NUM_PERM + 2, 1, PIXEL_FORMAT_R32G32B32A32_FLOAT,
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_1D));
				ret.push_back(Renderer::GetInstance().CreateTexture(&g[0], NUM_PERM + NUM_PERM + 2, 1, PIXEL_FORMAT_R32G32B32A32_FLOAT,
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_1D | TEXTURE_TYPE_SECOND_DEVICE));
				return ret;
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, "Failed to create Gradiants_256_Extended texture.");
		return{};
	}

	TexturePtr CreatePermutationTextureFloat() {
		std::vector<float> data(256);
		for (int i = 0; i < 256; ++i) {
			data[i] = Random(-1.f, 1.f);
		}
		return Renderer::GetInstance().CreateTexture(&data[0], 256, 1, PIXEL_FORMAT_R32_FLOAT,
			1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_1D);
	}

	//---------------------------------------------------------------------------
	std::vector<TexturePtr> CreateTexture(int ResourceTypes_Textures){
		std::vector<TexturePtr> ret;
		auto& renderer = Renderer::GetInstance();
		TextureCreationOption option;
		switch (ResourceTypes_Textures){		
		case ResourceTypes::Textures::Noise:
		{
			auto texture = renderer.CreateTexture("EssentialEngineData/textures/pnoise.dds", option);
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
			auto texture = renderer.CreateTexture(0, 512, 128, PIXEL_FORMAT_R16G16B16A16_FLOAT, 
				1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
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
			option.generateMip = false;
			auto texture = renderer.CreateTexture("EssentialEngineData/textures/ggx.dds", option);
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
				auto texture = renderer.CreateTexture(0, nSampleLen, nSampleLen, PIXEL_FORMAT_R16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
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
				auto texture = renderer.CreateTexture(0, 1, 1, PIXEL_FORMAT_R16_FLOAT, 
					1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_RENDER_TARGET_SRV);
				if (!texture){
					Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to create luminance map(%d)", i).c_str());
				}
				else{
					ret.push_back(texture);
				}
			}
			return ret;
		}
		case ResourceTypes::Textures::Permutation_256_Extended:
		{
			return CreatePermutationTexture();
		}

		case ResourceTypes::Textures::Gradiants_256_Extended:
		{
			return CreateGradientTexture();			
		}
		case ResourceTypes::Textures::ValueNoise:
		{
			auto texture = CreatePermutationTextureFloat();
			if (!texture) {
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create noise texture.");
			}
			else {
				ret.push_back(texture);
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
		auto it = mTextures.find(ResourceTypes_Textures);
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
		switch (ResourceTypes_Shaders) {
		case ResourceTypes::Shaders::FullscreenQuadNearVS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/FullScreenQuadVS.hlsl", SHADER_TYPE_VS);
		}
		case ResourceTypes::Shaders::FullscreenQuadFarVS:
		{
			shaderDefines.push_back(ShaderDefine("_FAR_SIDE_QUAD", "1"));
			return renderer.CreateShader("EssentialEngineData/shaders/FullScreenQuadVS.hlsl", SHADER_TYPE_VS, shaderDefines);
		}
		case ResourceTypes::Shaders::CopyPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/CopyPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::CopyPSMS:
		{
			shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			return renderer.CreateShader("EssentialEngineData/shaders/CopyPS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::ShadowMapShader:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/ShadowDepthVS.hlsl", SHADER_TYPE_VS);
		}
		case ResourceTypes::Shaders::DepthWriteVSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/Depth.hlsl", SHADER_TYPE_VS | SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::DepthOnlyVSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/DepthOnly.hlsl", SHADER_TYPE_VS | SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::CloudDepthWriteVSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/DepthCloud.hlsl", SHADER_TYPE_VS | SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::LineVSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/Line.hlsl", SHADER_TYPE_VS | SHADER_TYPE_PS);
		}

		case ResourceTypes::Shaders::PointVSGSPS: {
			return renderer.CreateShader("EssentialEngineData/shaders/Point.hlsl", SHADER_TYPE_VS | SHADER_TYPE_GS| SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::SampleLumInitialPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEngineData/shaders/SampleLumInitialNewPS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::SampleLumIterativePS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/SampleLumIterativeNewPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::SampleLumFinalPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/SampleLumFinalNewPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::CalcAdaptedLumPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/CalculateAdaptedLumPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::ToneMappingPS:
		{
			if (renderer.GetMultiSampleCount() != 1)
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));

			if (renderer.GetLuminanaceOnCPU())
				shaderDefines.push_back(ShaderDefine("_CPU_LUMINANCE", "1"));

			if (renderer.GetFilmicToneMapping())
				shaderDefines.push_back(ShaderDefine("_FILMIC_TONEMAP", "1"));
			return renderer.CreateShader("EssentialEngineData/shaders/ToneMappingPS.hlsl", SHADER_TYPE_PS, shaderDefines);

		}

		case ResourceTypes::Shaders::SimpleToneMappingPS:
		{
			if (renderer.GetMultiSampleCount() != 1)
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));

			return renderer.CreateShader("EssentialEngineData/shaders/SimpleToneMappingPS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}

		case ResourceTypes::Shaders::BrightPassPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEngineData/shaders/BrightPassPS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::BloomPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/BloomPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::Blur5x5PS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEngineData/shaders/GaussianBlur5x5PS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::StarGlarePS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/StarGlarePS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::MergeTextures2PS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/MergeTextures2PS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::GodRayPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/GodRayPS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::OcclusionPrePassVSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/OccPrePass.hlsl", SHADER_TYPE_VS | SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::OcclusionPrePassVSGSPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/OccPrePassWithGS.hlsl", SHADER_TYPE_VS | SHADER_TYPE_GS | SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::GlowPS:
		{
			if (renderer.GetMultiSampleCount() != 1) {
				shaderDefines.push_back(ShaderDefine("_MULTI_SAMPLE", "1"));
			}
			return renderer.CreateShader("EssentialEngineData/shaders/BloomPS.hlsl", SHADER_TYPE_PS, shaderDefines);
		}
		case ResourceTypes::Shaders::SilouettePS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/SilouettePS.hlsl", SHADER_TYPE_PS);
		}
		case ResourceTypes::Shaders::GGXGenPS:
		{
			return renderer.CreateShader("EssentialEngineData/shaders/GGXGenPS.hlsl", SHADER_TYPE_PS);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Unknown shader type");
			return 0;
		}
	}

	ShaderPtr GetShader(int ResourceTypes_Shaders){
		auto it = mShaders.find(ResourceTypes_Shaders);
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

	void BindShader(int ResourceTypes_Shaders, bool unbindEmptySlot){
		auto shader = GetShader(ResourceTypes_Shaders);
		if (shader)
			shader->Bind(unbindEmptySlot);
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
		case ResourceTypes::Materials::Frustum: {
			return renderer.CreateMaterial("EssentialEngineData/materials/frustum.material");
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Resource Provider - Unknown resource type.");
			return 0;
		}
	}

	MaterialPtr GetMaterial(int ResourceTypes_Materials){
		auto it = mMaterials.find(ResourceTypes_Materials);
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
			desc.SetCullMode(CULL_MODE_FRONT);
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::NoCull:
		{
			desc.SetCullMode(CULL_MODE_NONE);
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::OneBiased:
		{
			desc.SetDepthBias(1);
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::WireFrame:
		{
			desc.SetFillMode(FILL_MODE_WIREFRAME);
			desc.SetCullMode(CULL_MODE_NONE);
			desc.SetFrontCounterClockwise(false);
			desc.SetDepthBias(0);
			desc.SetDepthBiasClamp(0.0f);
			desc.SetSlopeScaledDepthBias(0.0f);
			desc.SetDepthClipEnable(true);
			desc.SetScissorEnable(false);
			desc.SetMultisampleEnable(false);
			desc.SetAntialiasedLineEnable(false);
			return renderer.CreateRasterizerState(desc);
		}
		case ResourceTypes::RasterizerStates::ShadowMapRS:
		{
			desc.SetCullMode(CULL_MODE_NONE);
			desc.SetAntialiasedLineEnable(false);
			return renderer.CreateRasterizerState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown rasterizer states (%d)", ResourceTypes_RasterizerStates).c_str());
			return 0;
		}
	}

	RasterizerStatePtr GetRasterizerState(int ResourceTypes_RasterizerStates){
		auto it = mRasterizerStates.find(ResourceTypes_RasterizerStates);
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
		case ResourceTypes::BlendStates::DefaultKeepAlpha:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ZERO;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::Additive:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			return renderer.CreateBlendState(desc);
		}
		case ResourceTypes::BlendStates::AdditiveKeepAlpha:{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_ONE;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			desc.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = BLEND_ZERO;
			desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
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
		auto it = mBlendStates.find(ResourceTypes_BlendStates);
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
			desc.SetDepthEnable(false);
			desc.SetDepthWriteMask(DEPTH_WRITE_MASK_ZERO);
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::NoDepthWrite_LessEqual:{
			desc.SetDepthWriteMask(DEPTH_WRITE_MASK_ZERO);
			desc.SetDepthFunc(COMPARISON_LESS_EQUAL);
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::LessEqual:{
			desc.SetDepthFunc(COMPARISON_LESS_EQUAL);
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::Greater: {
			desc.SetDepthFunc(COMPARISON_GREATER);
			return renderer.CreateDepthStencilState(desc);
		}
		case ResourceTypes::DepthStencilStates::IncrementalStencilOpaqueIf: {
			desc.SetStencilEnable(true);
			desc.GetFrontFace().SetStencilPassOp(STENCIL_OP_REPLACE);
			desc.GetFrontFace().SetStencilFunc(COMPARISON_GREATER_EQUAL);
			return renderer.CreateDepthStencilState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown depth stencil states (%d)", ResourceTypes_DepthStencilStates).c_str());
			return 0;
		}
	}

	DepthStencilStatePtr GetDepthStencilState(int ResourceTypes_DepthStencilStates){
		auto it = mDepthStencilStates.find(ResourceTypes_DepthStencilStates);
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
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_POINT);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Linear:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_LINEAR);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Anisotropic:
		{
			desc.SetFilter(TEXTURE_FILTER_ANISOTROPIC);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::Shadow:
		{
			desc.SetFilter(TEXTURE_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT);			
			desc.SetAddressU(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressV(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressW(TEXTURE_ADDRESS_BORDER);			
			desc.SetMaxAnisotropy(0);
			desc.SetMinLOD(0.f);
			desc.SetMaxLOD(0.f);
			desc.SetComparisonFunc(COMPARISON_LESS);				
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::PointWrap:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_POINT);
			desc.SetAddressU(TEXTURE_ADDRESS_WRAP);
			desc.SetAddressV(TEXTURE_ADDRESS_WRAP);
			desc.SetAddressW(TEXTURE_ADDRESS_WRAP);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::LinearWrap:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_LINEAR);
			desc.SetAddressU(TEXTURE_ADDRESS_WRAP);
			desc.SetAddressV(TEXTURE_ADDRESS_WRAP);
			desc.SetAddressW(TEXTURE_ADDRESS_WRAP);			
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::BlackBorder:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_LINEAR);
			desc.SetAddressU(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressV(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressW(TEXTURE_ADDRESS_BORDER);
			float color[4] = { 0, 0, 0, 0 };
			desc.SetBorderColor(color);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::PointBlackBorder:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_POINT);
			desc.SetAddressU(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressV(TEXTURE_ADDRESS_BORDER);
			desc.SetAddressW(TEXTURE_ADDRESS_BORDER);
			float color[4] = { 0, 0, 0, 0 };
			desc.SetBorderColor(color);
			return renderer.CreateSamplerState(desc);
		}
		case ResourceTypes::SamplerStates::LinearMirror:
		{
			desc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_LINEAR);
			desc.SetAddressU(TEXTURE_ADDRESS_MIRROR);
			desc.SetAddressV(TEXTURE_ADDRESS_MIRROR);
			desc.SetAddressW(TEXTURE_ADDRESS_MIRROR);
			return renderer.CreateSamplerState(desc);
		}
		default:
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Resource Provider - Unknown sampler states (%d)", ResourceTypes_SamplerStates).c_str());
			return 0;
		}
	}

	IndexBufferPtr CreateIndexBuffer(ResourceTypes::IndexBuffer type) {
		auto& renderer = Renderer::GetInstance();
		switch (type) {
		case ResourceTypes::IndexBuffer::Frustum:
		{
			/// near (left, bottom), (left, top), (right, bottom), (right, top)
			/// far (left, bottom), (left, top), (right, bottom), (right, top)
			short indices[] = {
				// near
				0, 1, 2,
				2, 1, 3,
				// far
				4, 6, 5,
				6, 7, 5,
				// left
				4, 5, 0,
				0, 5, 1,
				// right
				2, 3, 6,
				6, 3, 7,
				// top
				1, 5, 3,
				3, 5, 7,
				// bottom
				0, 2, 4,
				2, 6, 4,
			};
			return renderer.CreateIndexBuffer(indices, ARRAYCOUNT(indices), INDEXBUFFER_FORMAT_16BIT);			
		}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Unknown index buffer type(%d)", type).c_str());
		return nullptr;
	}

	SamplerStatePtr GetSamplerState(int ResourceTypes_SamplerStates){
		auto it = mSamplerStates.find(ResourceTypes_SamplerStates);
		if (it == mSamplerStates.end()){
			auto state = CreateSamplerState(ResourceTypes_SamplerStates);
			if (!state){
				Logger::Log(FB_ERROR_LOG_ARG, 
					FormatString("Resource Provider - Failed to create a depth stencil state(%d)", ResourceTypes_SamplerStates).c_str());
				return 0;
			}
			mSamplerStates[ResourceTypes_SamplerStates] = state;
			return state;
		}

		return it->second;
	}

	IndexBufferPtr GetIndexBuffer(ResourceTypes::IndexBuffer type) {
		auto it = mIndexBuffers.find(type);
		if (it == mIndexBuffers.end()) {
			auto ib = CreateIndexBuffer(type);
			if (!ib)
			{
				Logger::Log(FB_ERROR_LOG_ARG, 
					FormatString("Failed to create index buffer type(%d)", type).c_str());
				return nullptr;
			}
			mIndexBuffers[type] = ib;
			return ib;
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

	void BindDepthStencilState(int ResourceTypes_DepthStencilStates, int stencilRef){
		auto state = GetDepthStencilState(ResourceTypes_DepthStencilStates);
		if (state){
			state->Bind(stencilRef);
		}
	}

	int GetNumToneMaps() const{
		return NumToneMaps;
	}

	int GetNumLuminanceMaps() const{
		return NumLuminanceMaps;
	}

	void DeleteTexture(int ResourceTypes_Textures){
		mTextures.erase(ResourceTypes_Textures);
	}

	void DeleteShader(int ResourceTypes_Shaders){
		mShaders.erase(ResourceTypes_Shaders);
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

void ResourceProvider::BindShader(int ResourceTypes_Shaders, bool unbindEmptySlot){
	return mImpl->BindShader(ResourceTypes_Shaders, unbindEmptySlot);
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

IndexBufferPtr ResourceProvider::GetIndexBuffer(ResourceTypes::IndexBuffer type) {
	return mImpl->GetIndexBuffer(type);
}

void ResourceProvider::BindRasterizerState(int ResourceTypes_RasterizerStates){
	mImpl->BindRasterizerState(ResourceTypes_RasterizerStates);
}

void ResourceProvider::BindBlendState(int ResourceTypes_BlendStates){
	mImpl->BindBlendState(ResourceTypes_BlendStates);
}

void ResourceProvider::BindDepthStencilState(int ResourceTypes_DepthStencilStates, int stencilRef){
	mImpl->BindDepthStencilState(ResourceTypes_DepthStencilStates, stencilRef);
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
