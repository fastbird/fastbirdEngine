#pragma once
#include <Engine/IMaterial.h>
#include <CommonLib/Math/Vec4.h>
#include <CommonLib/VectorMap.h>
#include <Engine/Renderer/Shaders/Constants.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class IShader;
	class ITexture;
	class Material : public IMaterial
	{
	public:
		static void ReloadMaterial(const char* name);
		static void ReloadShader(const char* shaderPath);

		Material();
		explicit Material(const Material& mat);
		virtual ~Material();

		// only need if you don't use shared ptr
		virtual void Delete();

		//--------------------------------------------------------------------
		// IMaterial Interfaces
		//--------------------------------------------------------------------
		virtual bool LoadFromFile(const char* filepath);
		bool LoadFromXml(tinyxml2::XMLElement* pRoot);
		virtual const char* GetName() const { return mName.c_str(); }

		virtual IMaterial* Clone();
		virtual IMaterial* GetAdam() const;

		virtual void SetAmbientColor(float r, float g, float b, float a);
		virtual void SetAmbientColor(const Vec4& ambient);
		virtual void SetDiffuseColor(float r, float g, float b, float a);
		virtual void SetDiffuseColor(const Vec4& diffuse);
		virtual void SetSpecularColor(float r, float g, float b, float shine);
		virtual void SetSpecularColor(const Vec4& specular);
		virtual void SetEmissiveColor(float r, float g, float b, float strength);
		virtual void SetEmissiveColor(const Vec4& emissive);

		virtual void SetTexture(const char* filepath, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());
		virtual void SetTexture(ITexture* pTexture, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());

		virtual void SetColorRampTexture(ColorRamp& cr, BINDING_SHADER shader, int slot, 
			const SAMPLER_DESC& samplerDesc = SAMPLER_DESC());

		virtual void RemoveTexture(ITexture* pTexture);
		virtual void RemoveTexture(BINDING_SHADER shader, int slot);

		virtual ColorRamp& GetColorRamp(int slot, BINDING_SHADER shader);
		virtual void RefreshColorRampTexture(int slot, BINDING_SHADER shader);

		virtual void AddShaderDefine(const char* def, const char* val);
		virtual void RemoveShaderDefine(const char* def);
		virtual void ApplyShaderDefines();

		virtual void SetMaterialParameters(unsigned index, const Vec4& value);
		virtual const SHADER_DEFINES& GetShaderDefines() const {return mShaderDefines; }

		virtual const Vec4& GetAmbientColor() const;
		virtual const Vec4& GetDiffuseColor() const;
		virtual const Vec4& GetSpecularColor() const;
		virtual const Vec4& GetEmissiveColor() const;

		virtual const char* GetShaderFile() const;
		virtual void* GetShaderByteCode(unsigned& size) const;
		virtual const Vec4& GetMaterialParameters(unsigned index) const;
		virtual bool IsRelatedShader(const char* shaderFile);

		virtual void Bind(bool inputLayout);
		IMaterial* GetSubPassMaterial(RENDER_PASS p) const;
		virtual bool BindSubPass(RENDER_PASS p, bool includeInputLayout);
		virtual void BindMaterialParams();
		virtual void RegisterReloading();

		virtual bool IsTransparent() const { return mTransparent; }
		virtual void ReloadShader();

		virtual int GetBindingShaders() const { return mShaders; }
		virtual void CopyMaterialParamFrom(const IMaterial* src);

	private:
		void LoadSubMaterial(tinyxml2::XMLElement* mat);

	private:
		ITexture* CreateColorRampTexture(ColorRamp& cr);
		struct TextureSignature
		{
			TextureSignature(TEXTURE_TYPE type, const char* filepath, const ColorRamp* cr)
				:mType(type), mColorRamp(cr)
			{
				if (filepath)
					mFilepath = filepath;
			}
			TEXTURE_TYPE mType;
			std::string mFilepath;
			const ColorRamp* mColorRamp;
		};
		bool FindTextureIn(BINDING_SHADER shader, int slot, ITexture** pTextureInTheSlot,
			// additional parameters. if match this function returns true.
			TextureSignature* pSignature = 0) const;
		void SetAdam(Material* pAdam) { mAdamMaterial = pAdam; }
		void RemoveInstance(Material* pInstance);

	protected:
		typedef std::vector< Material* > Materials;
		static Materials mMaterials;
		static FB_READ_WRITE_CS mRWCSMaterial;
		std::vector< Material* > mInstances;
		SmartPtr<Material> mAdamMaterial;

		MATERIAL_CONSTANTS mMaterialConstants;
		SmartPtr<IShader> mShader;
		std::string mName;
		std::string mShaderFile;
		typedef VectorMap<unsigned, Vec4> PARAMETER_VECTOR;
		PARAMETER_VECTOR mMaterialParameters;
		std::vector< SmartPtr<ITexture> > mTextures;
		bool mReloading;
		int mReloadingTryCount;
		typedef std::map<ITexture*, ColorRamp> COLOR_RAMP_MAP_TYPE;
		COLOR_RAMP_MAP_TYPE mColorRampMap;

		SHADER_DEFINES mShaderDefines;
		INPUT_ELEMENT_DESCS mInputElementDescs;
		SmartPtr<IInputLayout> mInputLayout;
		int mShaders; // combination of enum BINDING_SHADER;
		bool mTransparent;

		std::vector<SmartPtr<IMaterial>> mSubMaterials;

		RENDER_PASS mRenderPass;

	};
}