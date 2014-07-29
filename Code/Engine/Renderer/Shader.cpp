#include <Engine/StdAfx.h>
#include <Engine/Renderer/Shader.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/GlobalEnv.h>

using namespace fastbird;

std::vector<Shader*> Shader::mShaders;

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
void Shader::ReloadShader(const char* filepath, const IMaterial::SHADER_DEFINES& shaderDefines)
{
	auto it = mShaders.begin(), itEnd = mShaders.end();
	for (; it!=itEnd; it++)
	{
		if (stricmp(filepath, (*it)->GetName())==0)
		{
			if ((*it)->GetShaderDefines() == shaderDefines)
			{
				gFBEnv->pEngine->GetRenderer()->CreateShader(filepath, (*it)->GetBindingShaders(), 
					shaderDefines, (*it));
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