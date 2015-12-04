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
#include "Shader.h"
#include "IPlatformShader.h"
#include "Renderer.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBTimer/Profiler.h"
#include "FBStringLib/StringLib.h"
using namespace fb;

static std::vector<ShaderWeakPtr> sAllShaders;

namespace fb{
	ShaderPtr GetShaderFromExistings(IPlatformShaderPtr platformTexture) {
		for (auto it = sAllShaders.begin(); it != sAllShaders.end(); /**/){
			IteratingWeakContainer(sAllShaders, it, shader);
			if (shader->GetPlatformShader() == platformTexture){
				return shader;
			}
		}
		return 0;
	}
}

class Shader::Impl{
public:
	// Do not acess member by this pointer
	static Impl* sLastBindedFullSetShader;
	Shader* mSelf;
	IPlatformShaderPtr mPlatformShader;
	SHADER_DEFINES mDefines;
	std::set<std::string> mIncludeFiles;
	std::string mPath;
	int mBindingShaders;
	
	//---------------------------------------------------------------------------
	Impl(Shader* shader) 
		: mBindingShaders(0)
		, mSelf(shader)
	{
	}

	void Bind(){
		if (mPlatformShader && sLastBindedFullSetShader != this){
			mPlatformShader->Bind();
			sLastBindedFullSetShader = this;
		}
	}

	void BindVS(){
		if (mPlatformShader){
			sLastBindedFullSetShader = 0;
			mPlatformShader->BindVS();
		}
	}

	void BindGS(){
		if (mPlatformShader){
			sLastBindedFullSetShader = 0;
			mPlatformShader->BindGS();
		}
	}

	void BindPS(){
		if (mPlatformShader){
			sLastBindedFullSetShader = 0;
			mPlatformShader->BindPS();
		}
	}

	void BindDS(){
		if (mPlatformShader){
			sLastBindedFullSetShader = 0;
			mPlatformShader->BindDS();
		}
	}

	void BindHS(){
		if (mPlatformShader){
			sLastBindedFullSetShader = 0;
			mPlatformShader->BindHS();
		}
	}

	bool GetCompileFailed() const{
		if (mPlatformShader)
			return mPlatformShader->GetCompileFailed();

		return true;
	}

	void* GetVSByteCode(unsigned& size) const{
		if (mPlatformShader)
			return mPlatformShader->GetVSByteCode(size);

		size = 0;
		return 0;
	}

	void SetPath(const char* path){
		if (path)
			mPath = path;
		else
			mPath.clear();
	}

	void SetBindingShaders(int bindingShaders){
		mBindingShaders = bindingShaders;
	}

	int GetBindingShaders() const{
		return mBindingShaders;
	}

	const char* GetPath() const{
		return mPath.c_str();
	}

	void SetShaderDefines(const SHADER_DEFINES& defines){
		mDefines = defines;
		std::sort(mDefines.begin(), mDefines.end());
	}
	const SHADER_DEFINES& GetShaderDefines() const { 
		return mDefines; 
	}

	void SetDebugName(const char* debugName){
		if (mPlatformShader)
			mPlatformShader->SetDebugName(debugName);
	}

	bool CheckIncludes(const char* shaderHeaderFile) const{
		return mPlatformShader ? mPlatformShader->CheckIncludes(shaderHeaderFile) : false;
	}

	void SetPlatformShader(IPlatformShaderPtr shader){
		mPlatformShader = shader;		
	}

	IPlatformShaderPtr GetPlatformShader() const{
		return mPlatformShader;
	}
};

Shader::Impl* Shader::Impl:: sLastBindedFullSetShader = 0;

//----------------------------------------------------------------------------
void Shader::ReloadShader(const char* filepath)
{
	Profiler profile("Reloading Shaders");
	std::string path = filepath;
	ToLowerCase(path);
	auto& renderer = Renderer::GetInstance();
	for (auto it = sAllShaders.begin(); it != sAllShaders.end(); /**/){
		IteratingWeakContainer(sAllShaders, it, shader);
		if (shader->mImpl->mPlatformShader){
			if (strcmp(filepath, shader->GetPath()) == 0 || shader->mImpl->mPlatformShader->CheckIncludes(path.c_str()))
			{
				renderer.ReloadShader(shader, filepath);
			}
		}
	}
}

ShaderPtr Shader::Create(){
	auto shader = ShaderPtr(FB_NEW(Shader), [](Shader* obj){ FB_DELETE(obj); });
	sAllShaders.push_back(shader);
	return shader;
}

Shader::Shader()
	: mImpl(new Impl(this))
{	
}

Shader::~Shader(){
	auto itEnd = sAllShaders.end();
	for (auto it = sAllShaders.begin(); it != itEnd; it++){
		if (it->expired()){
			sAllShaders.erase(it);
			break;
		}
	}
}

void Shader::Bind(){
	mImpl->Bind();
}

void Shader::BindVS(){
	mImpl->BindVS();
}

void Shader::BindHS(){
	mImpl->BindHS();
}

void Shader::BindDS(){
	mImpl->BindDS();
}

void Shader::BindGS(){
	mImpl->BindGS();
}

void Shader::BindPS(){
	mImpl->BindPS();
}

bool Shader::GetCompileFailed() const{
	return mImpl->GetCompileFailed();
}

void* Shader::GetVSByteCode(unsigned& size) const{
	return mImpl->GetVSByteCode(size);
}

void Shader::SetPath(const char* path){
	mImpl->SetPath(path);
}

void Shader::SetBindingShaders(int bindingShaders){
	mImpl->SetBindingShaders(bindingShaders);
}

int Shader::GetBindingShaders() const{
	return mImpl->GetBindingShaders();
}

const char* Shader::GetPath() const{
	return mImpl->GetPath();
}

void Shader::SetShaderDefines(const SHADER_DEFINES& defines){
	return mImpl->SetShaderDefines(defines);
}


const SHADER_DEFINES& Shader::GetShaderDefines() const{
	return mImpl->GetShaderDefines();
}

//void Shader::ApplyShaderDefines(){
//	mImpl->ApplyShaderDefines();
//}

void Shader::SetDebugName(const char* debugName){
	mImpl->SetDebugName(debugName);
}

bool Shader::CheckIncludes(const char* shaderHeaderFile) const{
	return mImpl->CheckIncludes(shaderHeaderFile);
}

void Shader::SetPlatformShader(IPlatformShaderPtr shader){
	mImpl->SetPlatformShader(shader);
}

IPlatformShaderPtr Shader::GetPlatformShader() const{
	return mImpl->GetPlatformShader();
}