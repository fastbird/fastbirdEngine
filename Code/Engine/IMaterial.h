#pragma once
#include <CommonLib/SmartPtr.h>
#include <Engine/Renderer/RendererEnums.h>
#include <Engine/Renderer/RendererStructs.h>
#include <Engine/Misc/ColorRamp.h>
#include <Engine/Renderer/RendererEnums.h>
namespace fastbird
{
	class ITexture;
	class Vec4;
	class IMaterial : public ReferenceCounter
	{
	public:
		static IMaterial* CreateMaterial(const char* file);		

		static void ReloadMaterial(const char* file);
		static void ReloadShader(const char* shader);
		virtual ~IMaterial(){}

		// only need if you don't use shared ptr
		virtual void Delete() = 0;

		struct ShaderDefine
		{
			ShaderDefine()
			{

			}
			ShaderDefine(const char* _name, const char* _value)
				: name(_name), value(_value)
			{

			}
			bool operator==(const ShaderDefine& b) const
			{
				if (name == b.name && value == b.value)
					return true;

				return false;
			}
			bool operator!=(const ShaderDefine& b) const
			{
				return !operator==(b);
			}
			std::string name;
			std::string value;
		};
		typedef std::vector<ShaderDefine> SHADER_DEFINES;

		virtual bool LoadFromFile(const char* filepath) = 0;
		virtual const char* GetName() const = 0;

		virtual IMaterial* Clone() = 0;
		virtual IMaterial* GetAdam() const = 0;

		virtual void SetAmbientColor(float r, float g, float b, float a) = 0;
		virtual void SetAmbientColor(const Vec4& ambient) = 0;
		virtual void SetDiffuseColor(float r, float g, float b, float a) = 0;
		virtual void SetDiffuseColor(const Vec4& diffuse) = 0;
		virtual void SetSpecularColor(float r, float g, float b, float shine) = 0;
		virtual void SetSpecularColor(const Vec4& specular) = 0;
		virtual void SetEmissiveColor(float r, float g, float b, float strength) = 0;
		virtual void SetEmissiveColor(const Vec4& emissive) = 0;

		virtual void SetTexture(const char* filepath, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC()) = 0;
		virtual void SetTexture(ITexture* pTexture, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC()) = 0;
		virtual ITexture* GetTexture(BINDING_SHADER shader, int slot) = 0;

		virtual void SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC()) = 0;

		virtual ColorRamp& GetColorRamp(int slot, BINDING_SHADER shader) = 0;

		virtual void RemoveTexture(ITexture* pTexture) = 0;
		virtual void RemoveTexture(BINDING_SHADER shader, int slot) = 0;

		virtual void RefreshColorRampTexture(int slot, BINDING_SHADER shader) = 0;

		virtual void SetMaterialParameters(unsigned index, const Vec4& value) = 0;
		virtual const SHADER_DEFINES& GetShaderDefines() const = 0;
		virtual bool AddShaderDefine(const char* def, const char* val) = 0;
		virtual bool RemoveShaderDefine(const char* def) = 0;
		virtual void ApplyShaderDefines() = 0;

		
		virtual const Vec4& GetAmbientColor() const = 0;
		virtual const Vec4& GetDiffuseColor() const = 0;
		virtual const Vec4& GetSpecularColor() const = 0;
		virtual const Vec4& GetEmissiveColor() const = 0;

		virtual const char* GetShaderFile() const = 0;
		virtual void* GetShaderByteCode(unsigned& size) const = 0;
		virtual const Vec4& GetMaterialParameters(unsigned index) const = 0;
		virtual bool IsRelatedShader(const char* shaderFile) = 0;

		virtual void Bind(bool inputLayout) = 0;
		virtual bool BindSubPass(RENDER_PASS p, bool includeInputLayout) = 0;
		virtual void BindMaterialParams() = 0;
		virtual void RegisterReloading() = 0;
		virtual bool IsTransparent() const = 0;
		virtual bool IsGlow() const = 0;
		virtual void ReloadShader() = 0;

		virtual int GetBindingShaders() const = 0;
		virtual void CopyMaterialParamFrom(const IMaterial* src) = 0;
	};

	inline bool operator==(const IMaterial::SHADER_DEFINES& a,
		const IMaterial::SHADER_DEFINES& b)
	{
		if (a.size() == b.size())
		{
			DWORD size = a.size();
			for (DWORD i=0; i<size; i++)
			{
				if (a[i] != b[i])
					return false;
			}
			return true;
		}
		return false;
	}

}