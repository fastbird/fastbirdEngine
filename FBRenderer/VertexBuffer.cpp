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
#include "VertexBuffer.h"
#include "IPlatformVertexBuffer.h"
using namespace fb;

class VertexBuffer::Impl{
public:
	unsigned mStride;
	unsigned mNumVertices;
	IPlatformVertexBufferPtr mPlatformBuffer;
	//---------------------------------------------------------------------------
	Impl(unsigned stride, unsigned numVertices)
		: mStride(stride)
		, mNumVertices(numVertices)
	{
	}

	unsigned GetStride() const { 
		return mStride; 
	}

	unsigned GetNumVertices() const { 
		return mNumVertices; 
	}

	void SetPlatformBuffer(IPlatformVertexBufferPtr buffer){
		mPlatformBuffer = buffer;
	}

	IPlatformVertexBufferPtr GetPlatformBuffer() const{
		return mPlatformBuffer;
	}

	bool IsReady() const{
		return mPlatformBuffer->IsReady();
	}

	void Bind(){
		mPlatformBuffer->Bind();
	}

	MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
		return mPlatformBuffer->Map(subResource, type, flag);
	}

	void Unmap(UINT subResource) const{
		mPlatformBuffer->Unmap(subResource);
	}
};

//---------------------------------------------------------------------------
VertexBufferPtr VertexBuffer::Create(unsigned stride, unsigned numVertices){
	return VertexBufferPtr(new VertexBuffer(stride, numVertices), [](VertexBuffer* obj){ delete obj; });
}

VertexBuffer::VertexBuffer(unsigned stride, unsigned numVertices)
	: mImpl(new Impl(stride, numVertices))
{
}

VertexBuffer::~VertexBuffer(){
}

void VertexBuffer::Bind(){
	mImpl->Bind();
}

MapData VertexBuffer::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
	return mImpl->Map(subResource, type, flag);
}

void VertexBuffer::Unmap(UINT subResource) const{
	mImpl->Unmap(subResource);
}

unsigned VertexBuffer::GetStride() const{
	return mImpl->GetStride();
}

unsigned VertexBuffer::GetNumVertices() const{
	return mImpl->GetNumVertices();
}

void VertexBuffer::SetPlatformBuffer(IPlatformVertexBufferPtr buffer){
	mImpl->SetPlatformBuffer(buffer);
}

IPlatformVertexBufferPtr VertexBuffer::GetPlatformBuffer() const{
	return mImpl->GetPlatformBuffer();
}