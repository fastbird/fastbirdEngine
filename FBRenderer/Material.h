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

#pragma once
#include "RendererEnums.h"
#include "RendererStructs.h"
#include "ShaderDefines.h"
#include "InputElementDesc.h"
#include "TextureBinding.h"
#include "FBMathLib/Math.h"
#include "FBCommonHeaders/VectorMap.h"
namespace tinyxml2{
	class XMLElement;
}
namespace fb{
	struct MATERIAL_CONSTANTS;
	FB_DECLARE_SMART_PTR(ColorRamp);
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(RenderStates);
	/** encapsulates material information(diffuse color, specular color...), 
	texture, shader, input layout and reder states.
	Use MaterialPtr Renderer::CreateMaterial(const char* filepath) to load 
	a material file.
	You can clone the material using Material::Clone(). Internal data will be
	shared among cloned materials on CopyOnWrite manner. i.e. At the time you set new data
	to the cloned material, the internal data of the material will be copied and be unique.
	*/
	class FB_DLL_RENDERER Material{
		FB_DECLARE_PIMPL(Material);
		Material();
		Material& Material::operator=(const Material& other) = delete;
		typedef std::vector< MaterialWeakPtr > Materials;
		static Materials mMaterials;

	public:
		
		typedef VectorMap<unsigned, Vec4f> Parameters;
		typedef std::vector< TexturePtr > Textures;
		typedef VectorMap<TextureBinding, TexturePtr> TexturesByBinding;
		typedef VectorMap<TexturePtr, TextureBinding> BindingsByTexture;

		static void ReloadMaterial(const char* name);
		
		static MaterialPtr Create();
		static MaterialPtr Create(const Material& other);
		Material(const Material& other);
		~Material();
		MaterialPtr Clone() const;
		bool LoadFromFile(const char* filepath);
		bool LoadFromXml(tinyxml2::XMLElement* pRoot);
		const char* GetName() const;
		void SetAmbientColor(float r, float g, float b, float a);
		void SetAmbientColor(const Vec4& ambient);
		void SetDiffuseColor(float r, float g, float b, float a);
		void SetDiffuseColor(const Vec4& diffuse);
		void SetSpecularColor(float r, float g, float b, float shine);
		void SetSpecularColor(const Vec4& specular);
		void SetEmissiveColor(float r, float g, float b, float strength);
		void SetEmissiveColor(const Vec4& emissive);
		void SetTexture(const char* filepath, BINDING_SHADER shader, int slot,
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());
		void SetTexture(TexturePtr pTexture, BINDING_SHADER shader, int slot,
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());
		TexturePtr GetTexture(BINDING_SHADER shader, int slot);
		void SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot,
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());
		void RemoveTexture(TexturePtr pTexture);
		void RemoveTexture(BINDING_SHADER shader, int slot);
		ColorRamp& GetColorRamp(int slot, BINDING_SHADER shader);
		void RefreshColorRampTexture(int slot, BINDING_SHADER shader);
		bool AddShaderDefine(const char* def, const char* val);
		bool RemoveShaderDefine(const char* def);
		void SetMaterialParameter(unsigned index, const Vec4& value);
		const SHADER_DEFINES& GetShaderDefines() const;
		/// Debugging features
		//void MarkNoShaderDefineChanges();
		void DebugPrintShaderDefines() const;

		const Vec4f& GetAmbientColor() const;
		const Vec4f& GetDiffuseColor() const;
		const Vec4f& GetSpecularColor() const;
		const Vec4f& GetEmissiveColor() const;
		const char* GetShaderFile() const;
		void* GetShaderByteCode(unsigned& size) const;
		const Vec4f& GetMaterialParameter(unsigned index) const;
		const Parameters& GetMaterialParameters() const;
		const MATERIAL_CONSTANTS& GetMaterialConstants() const;
		const Textures& GetTextures() const;
		const TexturesByBinding& GetTexturesByBinding() const;
		const BindingsByTexture& GetBindingsByTexture() const;

		bool IsUsingMaterialParameter(unsigned index);
		bool IsRelatedShader(const char* shaderFile);
		/// inputLayout: true, stencilRef: 0
		void Bind();
		/// stencilRef: 0
		void Bind(bool inputLayout); 
		void Bind(bool inputLayout, unsigned stencilRef);
		void Unbind();
		MaterialPtr GetSubPassMaterial(RENDER_PASS p) const;
		bool BindSubPass(RENDER_PASS p, bool includeInputLayout);
		void BindMaterialParams();
		void SetTransparent(bool trans);
		void SetGlow(bool glow);
		bool IsTransparent() const;
		bool IsGlow() const;
		bool IsNoShadowCast() const;
		bool IsDoubleSided() const;		
		int GetBindingShaders() const;
		void CopyMaterialParamFrom(MaterialConstPtr src);
		void CopyMaterialConstFrom(MaterialConstPtr src);
		void CopyTexturesFrom(MaterialConstPtr src);
		void CopyShaderDefinesFrom(MaterialConstPtr src);
		void SetRasterizerState(const RASTERIZER_DESC& desc);
		void SetBlendState(const BLEND_DESC& desc);
		void SetDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		RenderStatesPtr GetRenderStates() const;
		RENDER_PASS GetRenderPass() const;
		void ClearRasterizerState();
		void ClearBlendState(const BLEND_DESC& desc);
		void ClearDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		void SetInputLayout(const INPUT_ELEMENT_DESCS& desc);

		/// internal only.
		//void ApplyShaderDefines();
	};
}