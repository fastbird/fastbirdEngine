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

class Shader::Impl{
public:
	// Do not acess member by this pointer	
	Shader* mSelf;
	IPlatformShaderPtr mPlatformShaders[SHADER_TYPE_COUNT];
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

	// binding full set shader.
	bool Bind(bool unbindEmptySlot){
		int numShaders = 0;
		int lastIndex = 0;
		bool allSuccess = true;
		for (int i = 0; i < SHADER_TYPE_COUNT; ++i) {
			if (mPlatformShaders[i]) {
				if (!mPlatformShaders[i]->GetCompileFailed()) {
					mPlatformShaders[i]->Bind();
				}
				else {
					allSuccess = false;					
					Renderer::GetInstance().UnbindShader(ShaderType(i));
				}
			}
			else {
				if (unbindEmptySlot)
					Renderer::GetInstance().UnbindShader(ShaderType(i));
			}
		}

		if (allSuccess) {			
			return true;
		}
		else {
			return false;
		}
	}

	bool GetCompileFailed() const{
		bool allEmpty = true;
		for (int i = 0; i < SHADER_TYPE_COUNT; ++i) {
			if (mPlatformShaders[i]) {
				allEmpty = false;
				if (mPlatformShaders[i]->GetCompileFailed()) {
					return true;
				}
			}
		}
		return allEmpty;
	}

	void* GetVSByteCode(unsigned& size) const{
		if (mPlatformShaders[ShaderIndex(SHADER_TYPE_VS)])
			return mPlatformShaders[ShaderIndex(SHADER_TYPE_VS)]->GetVSByteCode(size);
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

	void SetPlatformShader(IPlatformShaderPtr shader, SHADER_TYPE shaderType){
		if (shader && shader->GetShaderType() != shaderType) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
			return;
		}		

		mPlatformShaders[ShaderIndex(shaderType)] = shader;		
		if (shader) {
			mBindingShaders |= (int)shaderType;
		}
		else {
			mBindingShaders &= ~shaderType;
		}
	}

	IPlatformShaderPtr GetPlatformShader(SHADER_TYPE shaderType) const{
		return mPlatformShaders[ShaderIndex(shaderType)];
	}

	bool IsRelatedFile(const char* file) const {
		for (auto p : mPlatformShaders) {
			if (p && p->IsRelatedFile(file))
				return true;
		}
		return false;
	}

	bool RunComputeShader(void* constants, size_t size,
		int x, int y, int z, ByteArray& output, size_t outputSize) 
	{
		if (!mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]) {
			Logger::Log(FB_ERROR_LOG_ARG, "Not vaild cs.");
			return false;
		}
		if (mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]->GetCompileFailed()) {
			Logger::Log(FB_ERROR_LOG_ARG, "Compile failed.");
			return false;
		}
		return mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]->RunComputeShader(constants, size,
			x, y, z, output, outputSize);
	}

	bool QueueRunComputeShader(void* constants, size_t size,
		int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback)
	{
		if (!mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]) {
			Logger::Log(FB_ERROR_LOG_ARG, "Not vaild cs.");
			return false;
		}
		if (mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]->GetCompileFailed()) {
			Logger::Log(FB_ERROR_LOG_ARG, "Compile failed.");
			return false;
		}
		return mPlatformShaders[ShaderIndex(SHADER_TYPE_CS)]->QueueRunComputeShader(constants, size,
			x, y, z, output, outputSize, std::move(callback));
	}
};

//----------------------------------------------------------------------------
ShaderPtr Shader::Create(){
	auto shader = ShaderPtr(FB_NEW(Shader), [](Shader* obj){ FB_DELETE(obj); });
	return shader;
}

Shader::Shader()
	: mImpl(new Impl(this))
{	
}

Shader::~Shader(){
}

bool Shader::Bind(bool unbindEmptySlot){
	return mImpl->Bind(unbindEmptySlot);
}

bool Shader::GetCompileFailed() const{
	return mImpl->GetCompileFailed();
}

void* Shader::GetVSByteCode(unsigned& size) const{
	return mImpl->GetVSByteCode(size);
}

//void Shader::SetPath(const char* path){
//	mImpl->SetPath(path);
//}

//void Shader::SetBindingShaders(int bindingShaders){
//	mImpl->SetBindingShaders(bindingShaders);
//}

int Shader::GetBindingShaders() const{
	return mImpl->GetBindingShaders();
}

//const char* Shader::GetPath() const{
//	return mImpl->GetPath();
//}
void Shader::SetPlatformShader(IPlatformShaderPtr shader, SHADER_TYPE shaderType){
	mImpl->SetPlatformShader(shader, shaderType);
}

IPlatformShaderPtr Shader::GetPlatformShader(SHADER_TYPE shaderType) const{
	return mImpl->GetPlatformShader(shaderType);
}

bool Shader::IsRelatedFile(const char* file) const {
	return mImpl->IsRelatedFile(file);
}

bool Shader::RunComputeShader(void* constants, size_t size,
	int x, int y, int z, ByteArray& output, size_t outputSize) 
{
	return mImpl->RunComputeShader(constants, size, x, y, z, output, outputSize);
}
bool Shader::QueueRunComputeShader(void* constants, size_t size,
	int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback)
{
	return mImpl->QueueRunComputeShader(constants, size, x, y, z, output, outputSize, std::move(callback));
}
