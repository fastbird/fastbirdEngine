#pragma once
#include <Engine/IShader.h>
#include <Engine/IMaterial.h>
namespace fastbird
{
	class Shader : public IShader
	{
	public:
		static void ReloadShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);
		static IShader* FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);

		virtual const char* GetName() const { return mName.c_str(); }
		virtual const IMaterial::SHADER_DEFINES& GetShaderDefines() const { return mDefines; }
		virtual void SetShaderDefines(const IMaterial::SHADER_DEFINES& defines);
		virtual void SetBindingShaders(int shaders) { mBindingShaders = shaders; }
		virtual int GetBindingShaders() const { return mBindingShaders; }


	protected:
		Shader(const char* name);
		virtual ~Shader();

	protected:
		std::string mName;
		IMaterial::SHADER_DEFINES mDefines;
		int mBindingShaders; // combination of BINDING_SHADER

	private:
		static std::vector<Shader*> mShaders;
		
	};
}