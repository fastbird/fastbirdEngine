#pragma once
#include <Engine/IShader.h>
#include <Engine/IMaterial.h>
namespace fastbird
{
	class Shader : public IShader
	{
	public:
		Shader() : mCompileFailed(false) {}
		static void ReloadShader(const char* name);
		static void ReloadShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);
		static IShader* FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);

		virtual const char* GetName() const { return mName.c_str(); }
		virtual const IMaterial::SHADER_DEFINES& GetShaderDefines() const { return mDefines; }
		virtual void SetShaderDefines(const IMaterial::SHADER_DEFINES& defines);
		virtual void SetBindingShaders(int shaders) { mBindingShaders = shaders; }
		virtual int GetBindingShaders() const { return mBindingShaders; }
		virtual void SetCompileFailed(bool failed){ mCompileFailed = failed; }
		virtual bool GetCompileFailed() const { return mCompileFailed; }

		virtual void SetIncludeFiles(const INCLUDE_FILES& ifs) { mIncludeFiles = ifs; }
		virtual const INCLUDE_FILES& GetIncludeFiles() const { return mIncludeFiles; }
		virtual bool CheckIncludes(const std::string& inc);


	protected:
		Shader(const char* name);
		virtual ~Shader();

	protected:
		std::string mName;
		IMaterial::SHADER_DEFINES mDefines;
		std::set<std::string> mIncludeFiles;
		int mBindingShaders; // combination of BINDING_SHADER
		bool mCompileFailed;

	private:
		static std::vector<Shader*> mShaders;
		
	};
}