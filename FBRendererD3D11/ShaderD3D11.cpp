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
#include "ShaderD3D11.h"
#include "IUnknownDeleter.h"
#include "D3D11Types.h"
#include "RendererD3D11.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBStringLib/StringLib.h"

using namespace fb;
class ShaderD3D11::Impl
{
public:
	ShaderD3D11* mSelf;	
	bool mCompileFailed;	
	StringVector mRelatedFiles;	

	//---------------------------------------------------------------------------
	Impl(ShaderD3D11* self)
		: mSelf(self)
		, mCompileFailed(false)			
	{
	}

	~Impl(){
	}

	void* GetVSByteCode(unsigned& size) const{
		size = 0;
		return nullptr;
	}

	bool IsRelatedFile(const char* file){
		if (ValidCString(file)){			
			for (const auto& v : mRelatedFiles) {
				if (_stricmp(v.c_str(), file) == 0)
					return true;
			}			
		}
		return false;
	}
};

//---------------------------------------------------------------------------
ShaderD3D11::ShaderD3D11()
	: mImpl(new Impl(this))
{
}

bool ShaderD3D11::GetCompileFailed() const {
	return mImpl->mCompileFailed;
}

void* ShaderD3D11::GetVSByteCode(unsigned& size) const {
	return mImpl->GetVSByteCode(size);
}

bool ShaderD3D11::RunComputeShader(void* constants, size_t size,
	int x, int y, int z, ByteArray& output, size_t outputSize) {
	Logger::Log(FB_ERROR_LOG_ARG, "Not compute shader.");
	return false;
}

bool ShaderD3D11::QueueRunComputeShader(void* constants, size_t size,
	int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&&) {
	Logger::Log(FB_ERROR_LOG_ARG, "Not compute shader.");
	return false;
}

bool ShaderD3D11::IsRelatedFile(const char* file) {
	return mImpl->IsRelatedFile(file);
}

bool ShaderD3D11::Reload(const SHADER_DEFINES& defines) {
	return RendererD3D11::GetInstance().ReloadShader(this, defines);
}

void ShaderD3D11::SetRelatedFiles(const StringVector& files) {
	mImpl->mRelatedFiles = files;
}
void ShaderD3D11::SetCompileFailed(bool failed) {
	mImpl->mCompileFailed = failed;
}

const StringVector& ShaderD3D11::GetRelatedFiles() const {
	return mImpl->mRelatedFiles;
}

//---------------------------------------------------------------------------
// VertexShader
//---------------------------------------------------------------------------
class VertexShaderD3D11::Impl {
public:
	ID3D11VertexShaderPtr mVertexShader;
	ByteArray mByteCodes;

	void SetDebugName(const char* name) {
		char buf[256];
		if (mVertexShader)
		{
			sprintf_s(buf, "%s VS", name);
			mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
	}

};

FB_IMPLEMENT_STATIC_CREATE(VertexShaderD3D11);
VertexShaderD3D11::VertexShaderD3D11()
	: mImpl(new Impl)
{

}

void VertexShaderD3D11::Bind() {
	RendererD3D11::GetInstance().SetShader(this);
}
void VertexShaderD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

void VertexShaderD3D11::SetVertexShader(ID3D11VertexShader* pVertexShader) {
	mImpl->mVertexShader = ID3D11VertexShaderPtr(pVertexShader, IUnknownDeleter());
}
void VertexShaderD3D11::SetVertexShaderBytecode(ByteArray&& bytecode) {
	mImpl->mByteCodes = bytecode;
}
void VertexShaderD3D11::SetVertexShaderBytecode(void* bytecode, size_t size) {
	if (!bytecode) {
		mImpl->mByteCodes.clear();
		return;
	}
	mImpl->mByteCodes.reserve(size);
	memcpy(&mImpl->mByteCodes[0], bytecode, size);
}

SHADER_TYPE VertexShaderD3D11::GetShaderType() const {
	return SHADER_TYPE_VS;
}

void* VertexShaderD3D11::GetVSByteCode(unsigned& size) const {
	size = mImpl->mByteCodes.size();
	if (size == 0) {
		return nullptr;
	}
	return &mImpl->mByteCodes[0];
}

ID3D11VertexShader* VertexShaderD3D11::GetVertexShader() const {
	return mImpl->mVertexShader.get();
}

//---------------------------------------------------------------------------
// GeometryShader
//---------------------------------------------------------------------------
class GeometryShaderD3D11::Impl {
public:
	ID3D11GeometryShaderPtr mGeometryShader;	

	void SetDebugName(const char* name) {
		char buf[256];
		if (mGeometryShader)
		{
			sprintf_s(buf, "%s GS", name);
			mGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
	}

};

FB_IMPLEMENT_STATIC_CREATE(GeometryShaderD3D11);
GeometryShaderD3D11::GeometryShaderD3D11()
	: mImpl(new Impl)
{

}

void GeometryShaderD3D11::Bind() {
	RendererD3D11::GetInstance().SetShader(this);
}
void GeometryShaderD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

SHADER_TYPE GeometryShaderD3D11::GetShaderType() const {
	return SHADER_TYPE_GS;
}

void GeometryShaderD3D11::SetGeometryShader(ID3D11GeometryShader* pGeometryShader) {
	mImpl->mGeometryShader = ID3D11GeometryShaderPtr(pGeometryShader, IUnknownDeleter());
}

ID3D11GeometryShader* GeometryShaderD3D11::GetGeometryShader() const {
	return mImpl->mGeometryShader.get();
}

//---------------------------------------------------------------------------
// PixelShader
//---------------------------------------------------------------------------
class PixelShaderD3D11::Impl {
public:
	ID3D11PixelShaderPtr mPixelShader;

	void SetDebugName(const char* name) {
		char buf[256];
		if (mPixelShader)
		{
			sprintf_s(buf, "%s PS", name);
			mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
	}

};

FB_IMPLEMENT_STATIC_CREATE(PixelShaderD3D11);
PixelShaderD3D11::PixelShaderD3D11()
	: mImpl(new Impl)
{

}

void PixelShaderD3D11::Bind() {
	RendererD3D11::GetInstance().SetShader(this);
}
void PixelShaderD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

SHADER_TYPE PixelShaderD3D11::GetShaderType() const {
	return SHADER_TYPE_PS;
}

void PixelShaderD3D11::SetPixelShader(ID3D11PixelShader* pPixelShader) {
	mImpl->mPixelShader = ID3D11PixelShaderPtr(pPixelShader, IUnknownDeleter());
}

ID3D11PixelShader* PixelShaderD3D11::GetPixelShader() const {
	return mImpl->mPixelShader.get();
}

//---------------------------------------------------------------------------
// ComputeShader
//---------------------------------------------------------------------------
class ComputeShaderD3D11::Impl {
public:
	ID3D11ComputeShaderPtr mComputeShader;

	void SetDebugName(const char* name) {
		char buf[256];
		if (mComputeShader)
		{
			sprintf_s(buf, "%s PS", name);
			mComputeShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mComputeShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
	}
};

FB_IMPLEMENT_STATIC_CREATE(ComputeShaderD3D11);
ComputeShaderD3D11::ComputeShaderD3D11()
	: mImpl(new Impl)
{

}

void ComputeShaderD3D11::Bind() {
	RendererD3D11::GetInstance().SetShader(this);
}
void ComputeShaderD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

SHADER_TYPE ComputeShaderD3D11::GetShaderType() const {
	return SHADER_TYPE_CS;
}

bool ComputeShaderD3D11::RunComputeShader(void* constants, size_t size,
	int x, int y, int z, ByteArray& output, size_t outputSize) 
{
	return RendererD3D11::GetInstance().RunComputeShader(this, constants, size,
		x, y, z, output, outputSize);
}

bool ComputeShaderD3D11::QueueRunComputeShader(void* constants, size_t size,
	int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback) 
{
	return RendererD3D11::GetInstance().QueueRunComputeShader(this, constants, size,
		x, y, z, output, outputSize, std::move(callback));
}

void ComputeShaderD3D11::SetComputeShader(ID3D11ComputeShader* pComputeShader) {
	mImpl->mComputeShader = ID3D11ComputeShaderPtr(pComputeShader, IUnknownDeleter());
}

ID3D11ComputeShader* ComputeShaderD3D11::GetComputeShader() const {
	return mImpl->mComputeShader.get();
}