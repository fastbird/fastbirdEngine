#include <Engine/StdAfx.h>
#include <Engine/Renderer/Shader.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/GlobalEnv.h>

using namespace fastbird;

std::vector<Shader*> Shader::mShaders;
//----------------------------------------------------------------------------
void IShader::ReloadShader(const char* name)
{
	return Shader::ReloadShader(name);
}

//----------------------------------------------------------------------------
void IShader::ReloadShader(const char* filepath, const IMaterial::SHADER_DEFINES& shaderDefines)
{
	return Shader::ReloadShader(filepath, shaderDefines);
}

//----------------------------------------------------------------------------
IShader* IShader::FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines)
{
	return Shader::FindShader(name, shaderDefines);
}

//----------------------------------------------------------------------------
Shader::Shader(const char* name)
	: mName(name) // always lower case
{
	mShaders.push_back(this);
}

//----------------------------------------------------------------------------
Shader::~Shader()
{
	mShaders.erase(std::remove(mShaders.begin(), mShaders.end(), this), mShaders.end());
}

//----------------------------------------------------------------------------
void Shader::ReloadShader(const char* filepath)
{
	Profiler profile("Reloading Shaders");
	std::string path = filepath;
	ToLowerCase(path);
	auto it = mShaders.begin(), itEnd = mShaders.end();
	for (; it != itEnd; it++)
	{
		
		if (strcmp(path.c_str(), (*it)->GetName()) == 0 || (*it)->CheckIncludes(path))
		{
			bool failed = gFBEnv->pEngine->GetRenderer()->CreateShader((*it)->GetName(), (*it)->GetBindingShaders(),
				(*it)->GetShaderDefines(), (*it)) == 0;
			if (failed)
			{
				Log("Reloading shader is stopped due to the failure of compilation.");
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------
void Shader::ReloadShader(const char* filepath, const IMaterial::SHADER_DEFINES& shaderDefines)
{
	auto it = mShaders.begin(), itEnd = mShaders.end();
	for (; it!=itEnd; it++)
	{
		if (stricmp(filepath, (*it)->GetName())==0)
		{
			if ((*it)->GetShaderDefines() == shaderDefines)
			{
				bool failed = gFBEnv->pEngine->GetRenderer()->CreateShader(filepath, (*it)->GetBindingShaders(), 
					shaderDefines, (*it))==0;
				if (failed)
				{
					Log("Reloading shader is stopped due to the failure of compilation.");
					break;
				}
					
			}			
		}
	}
}

//----------------------------------------------------------------------------
IShader* Shader::FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines)
{
	std::string filepath(name);
	ToLowerCase(filepath);
	auto it = mShaders.begin(), itEnd = mShaders.end();
	for (; it!=itEnd; it++)
	{
		if (strcmp(filepath.c_str(), (*it)->GetName())==0)
		{
			if ((*it)->GetShaderDefines() == shaderDefines)
				return *it;
		}
	}
	return 0;
}

void Shader::SetShaderDefines(const IMaterial::SHADER_DEFINES& defines)
{
	mDefines = defines;
}

bool Shader::CheckIncludes(const std::string& inc)
{
	return mIncludeFiles.find(inc) != mIncludeFiles.end();
}