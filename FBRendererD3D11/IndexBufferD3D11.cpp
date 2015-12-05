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
#include "IndexBufferD3D11.h"
#include "D3D11Types.h"
#include "RendererD3D11.h"
#include "IUnknownDeleter.h"
#include "FBRenderer/RendererEnums.h"
using namespace fb;
FB_IMPLEMENT_STATIC_CREATE(IndexBufferD3D11);

IndexBufferD3D11::IndexBufferD3D11()
{

}

//---------------------------------------------------------------------------
// IPlatformIndexBuffer
//---------------------------------------------------------------------------
bool IndexBufferD3D11::IsReady() const{
	return mIndexBuffer != 0;
}

void IndexBufferD3D11::Bind(unsigned offset){
	RendererD3D11::GetInstance().SetIndexBuffer(this, offset);
}

MapData IndexBufferD3D11::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag){
	return RendererD3D11::GetInstance().MapBuffer(mIndexBuffer.get(), subResource, type, flag);
}

void IndexBufferD3D11::Unmap(UINT subResource){
	RendererD3D11::GetInstance().UnmapBuffer(mIndexBuffer.get(), subResource);
}

//---------------------------------------------------------------------------
void IndexBufferD3D11::SetFormatD3D11(DXGI_FORMAT format){
	mFormat = format;
}

DXGI_FORMAT IndexBufferD3D11::GetFormatD3D11() const{
	return mFormat;
}

void IndexBufferD3D11::SetHardwareBuffer(ID3D11Buffer* pIndexBuffer){
	mIndexBuffer = ID3D11BufferPtr(pIndexBuffer, IUnknownDeleter());
}

ID3D11Buffer* IndexBufferD3D11::GetHardwareBuffer() const{
	return mIndexBuffer.get();
}