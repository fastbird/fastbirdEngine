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
#include "TextureD3D11.h"
#include "RendererD3D11.h"
#include "D3D11Types.h"
#include "ConvertEnumD3D11.h"
#include "IUnknownDeleter.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBStringLib/StringLib.h"
//DEFINE_GUID(WKPDID_D3DDebugObjectName, 0x429b8c22, 0x9188, 0x4b0c, 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00);
using namespace fb;
static std::atomic<UINT64> sNextTextureId = 1ull;
class TextureD3D11::Impl {
public:
	TextureD3D11* mSelf;
	UINT64 mId;
	std::string mPath;
	ID3D11ResourcePtr mTexture;
	mutable ID3D11ShaderResourceViewPtr mSRView;
	mutable ID3D11ShaderResourceView* mSRViewSync;
	DXGI_FORMAT mFormat;
	Vec2I mSize;
	// These views are in vector because Cube render targets has 6 views;
	std::vector<ID3D11RenderTargetViewPtr> mRTViews;
	std::vector<ID3D11DepthStencilViewPtr> mDSViews;
	size_t mSizeInBytes;
	bool mMipGenerated = false;


	//---------------------------------------------------------------------------
	Impl(TextureD3D11* self)
		: mSelf(self)
		, mSRViewSync(0)
		, mId(sNextTextureId++)
		, mSize(0, 0)
		, mFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
		, mSizeInBytes(0)
	{
	}

	~Impl() {

	}

	const Vec2ITuple GetSize() const {
		return mSize;
	}

	PIXEL_FORMAT GetPixelFormat() const {
		return  ConvertEnumFB(mFormat);
	}

	void SetPixelFormat(DXGI_FORMAT format) {
		mFormat = format;
	}

	bool IsReady() const {
		if (!mSRView && mSRViewSync) {
			mSRView = ID3D11ShaderResourceViewPtr(mSRViewSync, IUnknownDeleter());
			mSRViewSync = 0;
		}
		return mSRView != 0;
	}

	void Bind(SHADER_TYPE shader, int slot) const {
		RendererD3D11::GetInstance().SetTexture(mSelf, shader, slot);
	}

	MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const {
		return RendererD3D11::GetInstance().MapBuffer(mTexture.get(), subResource, type, flag);
	}

	void Unmap(UINT subResource) const {
		RendererD3D11::GetInstance().UnmapBuffer(mTexture.get(), subResource);
	}

	void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ,
		UINT srcSubresource, Box3D* srcBox) const 
	{
		RendererD3D11::GetInstance().CopyToStaging(dst, dstSubresource, dstX, dstY, dstZ,
			mSelf, srcSubresource, srcBox);
	}

	void CopyToStaging(IPlatformTexture* dst) const {
		RendererD3D11::GetInstance().CopyResource(dst, mSelf);
	}

	void SaveToFile(const char* filename) const {
		RendererD3D11::GetInstance().SaveTextureToFile(mSelf, filename);
	}

	void GenerateMips() {
		RendererD3D11::GetInstance().GenerateMips(mSelf);
		mMipGenerated = true;
	}

	void SetDebugName(const char* name) {
		if (mTexture)
			mTexture->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
		if (mSRView)
		{
			char buff[255];
			sprintf_s(buff, "%s SRV", name);
			mSRView->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			mSRView->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
		}
		int i = 0;
		for (auto it : mRTViews)
		{
			char buff[255];
			sprintf_s(buff, "%s RTV %d", name, i++);
			it->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			it->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
		}

		i = 0;
		for (auto it : mDSViews)
		{
			char buff[255];
			sprintf_s(buff, "%s DSV %d", name, i++);
			it->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
			it->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
		}
	}

	size_t GetSizeInBytes() const {
		return mSizeInBytes;
	}

	//--------------------------------------------------------------------
	// Own
	//--------------------------------------------------------------------
	ID3D11Resource* GetHardwareTexture() {
		if (!mTexture && mSRView){
			ID3D11Resource* resource = 0;
			mSRView->GetResource(&resource);
			mTexture = ID3D11ResourcePtr(resource, IUnknownDeleter());
		}
		return mTexture.get();
	}

	ID3D11ShaderResourceView* GetHardwareResourceView(){
		if (!mSRView && mSRViewSync) {
			mSRView = ID3D11ShaderResourceViewPtr(mSRViewSync, IUnknownDeleter());
			mSRViewSync = 0;
		}
		return mSRView.get();
	}

	// pTexture will be owned by this instance.
	// DO NOT pass a pointer you get from shared_ptr.
	void SetHardwareTexture(ID3D11Resource* pTexture){
		mTexture = ID3D11ResourcePtr(pTexture, IUnknownDeleter());
	}

	// pResourceView will be owned by this instance.
	// DO NOT pass a pointer you get from shared_ptr.
	void SetHardwareResourceView(ID3D11ShaderResourceView* pResourceView){
		mSRView = ID3D11ShaderResourceViewPtr(pResourceView, IUnknownDeleter());		
		if (pResourceView && !mTexture) {
			ID3D11Resource* pResource = 0;
			pResourceView->GetResource(&pResource);
			SetHardwareTexture(pResource);
		}
	}

	// pRenderTargetView will be owned by this instance.
	// DO NOT pass a pointer you get from shared_ptr.
	void AddRenderTargetView(ID3D11RenderTargetView* pRenderTargetView){
		mRTViews.push_back(ID3D11RenderTargetViewPtr(pRenderTargetView, IUnknownDeleter()));
	}

	void ClearRenderTargetViews(){
		mRTViews.clear();
	}

	ID3D11RenderTargetView* GetRenderTargetView(int idx) const{
		if (idx < (int)mRTViews.size()){
			return mRTViews[idx].get();
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the render target view with idx(%d)", idx).c_str());
		return 0;
	}

	size_t NumRTViews() const{
		return mRTViews.size();
	}

	// pDepthStencilView will be owned by this instance.
	// DO NOT pass a pointer you get from shared_ptr.
	void AddDepthStencilView(ID3D11DepthStencilView* pDepthStencilView){
		mDSViews.push_back(ID3D11DepthStencilViewPtr(pDepthStencilView, IUnknownDeleter()));
	}

	void ClearDepthStencilViews(){
		mDSViews.clear();
	}

	ID3D11DepthStencilView* GetDepthStencilView(int idx) const{
		if (idx < (int)mDSViews.size()){
			return mDSViews[idx].get();
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the depth stencil view with idx(%d)", idx).c_str());
		return 0;
	}

	size_t NumDSViews() const{
		return mDSViews.size();
	}

	void SetSize(const Vec2I& size){
		mSize = size;
	}

	ID3D11ShaderResourceView** GetSRViewSyncPtr(){
		return &mSRViewSync;
	}

	UINT64 GetTextureId() const{
		return mId;
	}

	void SetPath(const char* path){
		if (ValidCString(path))
			mPath = path;
	}

	const char* GetPath() const{
		return mPath.c_str();
	}
};

//----------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(TextureD3D11);
TextureD3D11::TextureD3D11()
	: mImpl(new Impl(this))
{
}

TextureD3D11::~TextureD3D11(){

}

//--------------------------------------------------------------------
// IPlatformTexture
//--------------------------------------------------------------------		
Vec2ITuple TextureD3D11::GetSize() const {
	return mImpl->GetSize();
}

PIXEL_FORMAT TextureD3D11::GetPixelFormat() const {
	return mImpl->GetPixelFormat();
}

void TextureD3D11::SetPixelFormat(DXGI_FORMAT format){
	mImpl->SetPixelFormat(format);
}

bool TextureD3D11::IsReady() const {
	return mImpl->IsReady();
}

void TextureD3D11::Bind(SHADER_TYPE shader, int slot) const {
	mImpl->Bind(shader, slot);
}

MapData TextureD3D11::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const {
	return mImpl->Map(subResource, type, flag);
}

void TextureD3D11::Unmap(UINT subResource) const {
	mImpl->Unmap(subResource);
}

void TextureD3D11::CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ, UINT srcSubresource, Box3D* srcBox) const {
	mImpl->CopyToStaging(dst, dstSubresource, dstX, dstY, dstZ, srcSubresource, srcBox);
}

void TextureD3D11::CopyToStaging(IPlatformTexture* dst) const {
	mImpl->CopyToStaging(dst);
}

void TextureD3D11::SaveToFile(const char* filename) const {
	mImpl->SaveToFile(filename);
}

void TextureD3D11::GenerateMips() {
	mImpl->GenerateMips();
}

void TextureD3D11::SetDebugName(const char* name) {
	mImpl->SetDebugName(name);
}

bool TextureD3D11::GetMipGenerated() const {
	return mImpl->mMipGenerated;
}

size_t TextureD3D11::GetSizeInBytes() const {
	return mImpl->GetSizeInBytes();
}

//--------------------------------------------------------------------
// Own
//--------------------------------------------------------------------		
ID3D11Resource* TextureD3D11::GetHardwareTexture() const {
	return mImpl->GetHardwareTexture();
}

ID3D11ShaderResourceView* TextureD3D11::GetHardwareResourceView() {
	return mImpl->GetHardwareResourceView();
}

// pTexture will be owned by this instance.
// DO NOT pass a pointer you get from shared_ptr.
void TextureD3D11::SetHardwareTexture(ID3D11Resource* pTexture) {
	mImpl->SetHardwareTexture(pTexture);
}

// pResourceView will be owned by this instance.
// DO NOT pass a pointer you get from shared_ptr.
void TextureD3D11::SetHardwareResourceView(ID3D11ShaderResourceView* pResourceView) {
	mImpl->SetHardwareResourceView(pResourceView);
}

// pRenderTargetView will be owned by this instance.
// DO NOT pass a pointer you get from shared_ptr.
void TextureD3D11::AddRenderTargetView(ID3D11RenderTargetView* pRenderTargetView) {
	mImpl->AddRenderTargetView(pRenderTargetView);
}

void TextureD3D11::ClearRenderTargetViews() {
	mImpl->ClearRenderTargetViews();
}

ID3D11RenderTargetView* TextureD3D11::GetRenderTargetView(int idx) const {
	return mImpl->GetRenderTargetView(idx);
}

size_t TextureD3D11::NumRTViews() const {
	return mImpl->NumRTViews();
}

// pDepthStencilView will be owned by this instance.
// DO NOT pass a pointer you get from shared_ptr.
void TextureD3D11::AddDepthStencilView(ID3D11DepthStencilView* pDepthStencilView) {
	mImpl->AddDepthStencilView(pDepthStencilView);
}

void TextureD3D11::ClearDepthStencilViews() {
	mImpl->ClearDepthStencilViews();
}

ID3D11DepthStencilView* TextureD3D11::GetDepthStencilView(int idx) const {
	return mImpl->GetDepthStencilView(idx);
}

size_t TextureD3D11::NumDSViews() const {
	return mImpl->NumDSViews();
}

void TextureD3D11::SetSize(const Vec2I& size) {
	mImpl->SetSize(size);
}

ID3D11ShaderResourceView** TextureD3D11::GetSRViewSyncPtr() {
	return mImpl->GetSRViewSyncPtr();
}

UINT64 TextureD3D11::GetTextureId() const {
	return mImpl->GetTextureId();
}

void TextureD3D11::SetPath(const char* path) {
	mImpl->SetPath(path);
}

const char* TextureD3D11::GetPath() const {
	return mImpl->GetPath();
}

void TextureD3D11::SetMipGenerated(bool requested) {
	mImpl->mMipGenerated = requested;
}

size_t TextureD3D11::Hash() const {
	return (size_t)mImpl->mId;
}

void TextureD3D11::SetSizeInBytes(size_t bytes) {
	mImpl->mSizeInBytes = bytes;
}