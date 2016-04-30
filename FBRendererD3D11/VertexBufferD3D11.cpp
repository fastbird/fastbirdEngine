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
#include "VertexBufferD3D11.h"
#include "IUnknownDeleter.h"
#include "RendererD3D11.h"

namespace fb
{
	VertexBufferD3D11Ptr VertexBufferD3D11::Create(ID3D11Buffer* vertexBuffer, unsigned stride){
		return VertexBufferD3D11Ptr(new VertexBufferD3D11(vertexBuffer, stride), [](VertexBufferD3D11* obj){ delete obj; });
	}

	VertexBufferD3D11::VertexBufferD3D11(ID3D11Buffer* vertexBuffer, unsigned stride)
		: mVertexBuffer(vertexBuffer, IUnknownDeleter())
		, mStride(stride)
	{
	}
	
	void VertexBufferD3D11::Bind() const{
		unsigned int offset = 0;
		IPlatformVertexBuffer const * buffers[] = { this };
		RendererD3D11::GetInstance().SetVertexBuffers(0, 1, buffers, &mStride, &offset);
	}

	bool VertexBufferD3D11::IsReady() const
	{
		return mVertexBuffer != 0;
	}

	MapData VertexBufferD3D11::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag){
		return RendererD3D11::GetInstance().MapBuffer(mVertexBuffer.get(), subResource, type, flag);
	}

	void VertexBufferD3D11::Unmap(UINT subResource){
		RendererD3D11::GetInstance().UnmapBuffer(mVertexBuffer.get(), subResource);
	}

	bool VertexBufferD3D11::UpdateBuffer(void* data, unsigned bytes) {
		return RendererD3D11::GetInstance().UpdateBuffer(mVertexBuffer.get(), data, bytes);
	}

	ID3D11Buffer* VertexBufferD3D11::GetHardwareBuffer() const{
		return mVertexBuffer.get();
	}
}