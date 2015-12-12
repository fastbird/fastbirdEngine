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

#pragma once

#include "FBRenderer/IPlatformTexture.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(TextureD3D11);
	class TextureD3D11 : public IPlatformTexture
	{
		FB_DECLARE_PIMPL(TextureD3D11);
		TextureD3D11();
		~TextureD3D11();

	public:
		static TextureD3D11Ptr Create();

		//--------------------------------------------------------------------
		// IPlatformTexture
		//--------------------------------------------------------------------		
		Vec2ITuple GetSize() const;
		PIXEL_FORMAT GetPixelFormat() const;
		bool IsReady() const;
		void Bind(BINDING_SHADER shader, int slot) const;		
		MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const;
		void Unmap(UINT subResource) const;
		void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource,
			UINT dstX, UINT dstY, UINT dstZ, UINT srcSubresource, Box3D* srcBox) const;
		void SaveToFile(const char* filename) const;
		void GenerateMips();
		void SetDebugName(const char* name);
		
		//--------------------------------------------------------------------
		// Own
		//--------------------------------------------------------------------		
		ID3D11Texture2D* GetHardwareTexture() const;
		ID3D11ShaderResourceView* GetHardwareResourceView();
		// pTexture will be owned by this instance.
		// DO NOT pass a pointer you get from shared_ptr.
		void SetHardwareTexture(ID3D11Texture2D* pTexture);
		// pResourceView will be owned by this instance.
		// DO NOT pass a pointer you get from shared_ptr.
		void SetHardwareResourceView(ID3D11ShaderResourceView* pResourceView);
		// pRenderTargetView will be owned by this instance.
		// DO NOT pass a pointer you get from shared_ptr.
		void AddRenderTargetView(ID3D11RenderTargetView* pRenderTargetView);
		void ClearRenderTargetViews();
		ID3D11RenderTargetView* GetRenderTargetView(int idx) const;
		size_t NumRTViews() const;
		// pDepthStencilView will be owned by this instance.
		// DO NOT pass a pointer you get from shared_ptr.
		void AddDepthStencilView(ID3D11DepthStencilView* pDepthStencilView);
		void ClearDepthStencilViews();
		ID3D11DepthStencilView* GetDepthStencilView(int idx) const;
		size_t NumDSViews() const;
		void SetSize(const Vec2I& size);
		void SetLoadInfoTextureFormat(DXGI_FORMAT format);
		D3DX11_IMAGE_LOAD_INFO* GetLoadInfoPtr();
		ID3D11ShaderResourceView** GetSRViewSyncPtr();
		HRESULT* GetHRPtr();
		unsigned GetTextureId() const;
		void SetPath(const char* path);
		const char* GetPath() const;
		ID3D11Texture2D* GetHardwareTexture();
	};
}