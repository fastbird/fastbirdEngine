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
	ID3D11VertexShaderPtr mVertexShader;
	ID3D11GeometryShaderPtr mGeometryShader;
	ID3D11HullShaderPtr mHullShader;
	ID3D11DomainShaderPtr mDomainShader;
	ID3D11PixelShaderPtr mPixelShader;
	bool mCompileFailed;
	BinaryData mVSBytecode;
	size_t mBytecodeSize;
	INCLUDE_FILES mIncludeFiles;

	//---------------------------------------------------------------------------
	Impl(ShaderD3D11* self)
		: mSelf(self)
		, mCompileFailed(false)		
		, mBytecodeSize(0)
	{
	}

	~Impl(){

	}

	void Bind(){
		RendererD3D11::GetInstance().SetShaders(mSelf);
	}

	/** Bind vertex shader only.*/
	void BindVS(){
		RendererD3D11::GetInstance().SetVSShader(mSelf);
	}

	/** Bind hull shader only.*/
	void BindHS(){
		RendererD3D11::GetInstance().SetHSShader(mSelf);
	}

	/** Bind domain shader only.*/
	void BindDS(){
		RendererD3D11::GetInstance().SetDSShader(mSelf);
	}

	/** Bind geometry shader only.*/
	void BindGS(){
		RendererD3D11::GetInstance().SetGSShader(mSelf);
	}

	/** Bind pixel shader only.	*/
	void BindPS(){
		RendererD3D11::GetInstance().SetPSShader(mSelf);
	}	

	bool GetCompileFailed() const{
		return mCompileFailed;
	}

	void* GetVSByteCode(unsigned& size) const{
		size = mBytecodeSize;
		return mVSBytecode.get();
	}

	void SetDebugName(const char* name){
		char buf[256];
		if (mVertexShader)
		{
			sprintf_s(buf, "%s VS", name);
			mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}

		if (mGeometryShader)
		{
			sprintf_s(buf, "%s GS", name);
			mGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}

		if (mHullShader)
		{
			sprintf_s(buf, "%s HS", name);
			mHullShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mHullShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}

		if (mDomainShader)
		{
			sprintf_s(buf, "%s DS", name);
			mDomainShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mDomainShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}

		if (mPixelShader)
		{
			sprintf_s(buf, "%s PS", name);
			mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
	}

	bool CheckIncludes(const char* inc){		
		if (ValidCStringLength(inc)){
			std::string testing(inc);
			ToLowerCase(testing);
			return mIncludeFiles.find(testing) != mIncludeFiles.end();
		}
		return false;
	}

	//---------------------------------------------------------------------------
	// Own
	//---------------------------------------------------------------------------
	void SetVertexShader(ID3D11VertexShader* pVertexShader){
		mVertexShader = ID3D11VertexShaderPtr(pVertexShader, IUnknownDeleter());
	}

	void SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode){
		if (pVertexShaderBytecode)
		{
			mBytecodeSize = pVertexShaderBytecode->GetBufferSize();
			mVSBytecode = BinaryData(new char[mBytecodeSize], [](char* obj){ delete[] obj; });
			memcpy(mVSBytecode.get(), pVertexShaderBytecode->GetBufferPointer(), mBytecodeSize);
		}
		else{
			mVSBytecode.reset();
		}
	}

	void SetVertexShaderBytecode(void* bytecode, size_t size){
		if (bytecode && size > 0){
			mBytecodeSize = size;
			mVSBytecode = BinaryData(new char[mBytecodeSize], [](char* obj){ delete[] obj; });
			memcpy(mVSBytecode.get(), bytecode, mBytecodeSize);
		}
		else{
			mVSBytecode.reset();
		}
	}

	void SetGeometryShader(ID3D11GeometryShader* pGeometryShader){
		mGeometryShader = ID3D11GeometryShaderPtr(pGeometryShader, IUnknownDeleter());
	}

	void SetHullShader(ID3D11HullShader* pHullShader){
		mHullShader = ID3D11HullShaderPtr(pHullShader, IUnknownDeleter());
	}

	void SetDomainShader(ID3D11DomainShader* pDomainShader){
		mDomainShader = ID3D11DomainShaderPtr(pDomainShader, IUnknownDeleter());
	}

	void SetPixelShader(ID3D11PixelShader* pPixelShader){
		mPixelShader = ID3D11PixelShaderPtr(pPixelShader, IUnknownDeleter());
	}
	
	ID3D11VertexShader* GetVertexShader() const{
		return mVertexShader.get();
	}

	ID3D11GeometryShader* GetGeometryShader() const{
		return mGeometryShader.get();
	}

	ID3D11HullShader* GetHullShader() const{
		return mHullShader.get();
	}

	ID3D11DomainShader* GetDomainShader() const{
		return mDomainShader.get();
	}

	ID3D11PixelShader* GetPixelShader() const{
		return mPixelShader.get();
	}

	void SetIncludeFiles(const INCLUDE_FILES& ifs){
		mIncludeFiles = ifs;
	}

	void SetCompileFailed(bool failed){
		mCompileFailed = failed;
	}

};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(ShaderD3D11);

ShaderD3D11::ShaderD3D11()
	: mImpl(new Impl(this))
{
}

//---------------------------------------------------------------------------
// IPlatformShader
//---------------------------------------------------------------------------
void ShaderD3D11::Bind() {
	mImpl->Bind();
}

void ShaderD3D11::BindVS() {
	mImpl->BindVS();
}

void ShaderD3D11::BindHS() {
	mImpl->BindHS();
}

void ShaderD3D11::BindDS() {
	mImpl->BindDS();
}

void ShaderD3D11::BindGS() {
	mImpl->BindGS();
}

void ShaderD3D11::BindPS() {
	mImpl->BindPS();
}

bool ShaderD3D11::GetCompileFailed() const {
	return mImpl->GetCompileFailed();
}

void* ShaderD3D11::GetVSByteCode(unsigned& size) const {
	return mImpl->GetVSByteCode(size);
}

void ShaderD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

bool ShaderD3D11::CheckIncludes(const char* inc) {
	return mImpl->CheckIncludes(inc);
}

//---------------------------------------------------------------------------
// Own
//---------------------------------------------------------------------------
void ShaderD3D11::SetVertexShader(ID3D11VertexShader* pVertexShader) {
	mImpl->SetVertexShader(pVertexShader);
}

void ShaderD3D11::SetVertexShaderBytecode(ID3DBlob* pVertexShaderBytecode) {
	mImpl->SetVertexShaderBytecode(pVertexShaderBytecode);
}

void ShaderD3D11::SetVertexShaderBytecode(void* bytecode, size_t size) {
	mImpl->SetVertexShaderBytecode(bytecode, size);
}

void ShaderD3D11::SetGeometryShader(ID3D11GeometryShader* pGeometryShader) {
	mImpl->SetGeometryShader(pGeometryShader);
}

void ShaderD3D11::SetHullShader(ID3D11HullShader* pHullShader) {
	mImpl->SetHullShader(pHullShader);
}

void ShaderD3D11::SetDomainShader(ID3D11DomainShader* pDomainShader) {
	mImpl->SetDomainShader(pDomainShader);
}

void ShaderD3D11::SetPixelShader(ID3D11PixelShader* pPixelShader) {
	mImpl->SetPixelShader(pPixelShader);
}

ID3D11VertexShader* ShaderD3D11::GetVertexShader() const {
	return mImpl->GetVertexShader();
}

ID3D11GeometryShader* ShaderD3D11::GetGeometryShader() const {
	return mImpl->GetGeometryShader();
}

ID3D11HullShader* ShaderD3D11::GetHullShader() const {
	return mImpl->GetHullShader();
}

ID3D11DomainShader* ShaderD3D11::GetDomainShader() const {
	return mImpl->GetDomainShader();
}

ID3D11PixelShader* ShaderD3D11::GetPixelShader() const {
	return mImpl->GetPixelShader();
}

void ShaderD3D11::SetIncludeFiles(const INCLUDE_FILES& ifs) {
	mImpl->SetIncludeFiles(ifs);
}

void ShaderD3D11::SetCompileFailed(bool failed) {
	mImpl->SetCompileFailed(failed);
}

