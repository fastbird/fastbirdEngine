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
#include "IndexBuffer.h"
#include "IPlatformIndexBuffer.h"
using namespace fb;
class IndexBuffer::Impl{
public:
	INDEXBUFFER_FORMAT mFormat;
	unsigned mNumIndices;
	IPlatformIndexBufferPtr mPlatformBuffer;

	//---------------------------------------------------------------------------
	Impl(unsigned numIndices, INDEXBUFFER_FORMAT format)
		: mFormat(format)
		, mNumIndices(numIndices){

	}

	bool IsReady() const{
		return mPlatformBuffer->IsReady();
	}

	void Bind(unsigned offset) const{
		mPlatformBuffer->Bind(offset);
	}

	MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
		return mPlatformBuffer->Map(subResource, type, flag);
	}

	void Unmap(UINT subResource) const{
		mPlatformBuffer->Unmap(subResource);
	}

	unsigned GetNumIndices() const{
		return mNumIndices;
	}

	INDEXBUFFER_FORMAT GetFormat() const{
		return mFormat;
	}

	void SetPlatformBuffer(IPlatformIndexBufferPtr buffer) {
		mPlatformBuffer = buffer;
	}

	IPlatformIndexBufferPtr GetPlatformBuffer() const{
		return mPlatformBuffer;
	}
};

//---------------------------------------------------------------------------
IndexBufferPtr IndexBuffer::Create(unsigned numIndices, INDEXBUFFER_FORMAT format){
	return IndexBufferPtr(new IndexBuffer(numIndices, format), [](IndexBuffer* obj){ delete obj; });
}

IndexBuffer::IndexBuffer(unsigned numIndices, INDEXBUFFER_FORMAT format)
	: mImpl(new Impl(numIndices, format)){
}

IndexBuffer::~IndexBuffer(){}

bool IndexBuffer::IsReady() const{
	return mImpl->IsReady();
}

void IndexBuffer::Bind(unsigned offset) const{
	mImpl->Bind(offset);
}

MapData IndexBuffer::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
	return mImpl->Map(subResource, type, flag);
}

void IndexBuffer::Unmap(UINT subResource) const{
	mImpl->Unmap(subResource);
}

unsigned IndexBuffer::GetNumIndices() const{
	return mImpl->GetNumIndices();
}

INDEXBUFFER_FORMAT IndexBuffer::GetFormat() const{
	return mImpl->GetFormat();
}

void IndexBuffer::SetPlatformBuffer(IPlatformIndexBufferPtr buffer){
	mImpl->SetPlatformBuffer(buffer);
}

IPlatformIndexBufferPtr IndexBuffer::GetPlatformBuffer(){
	return mImpl->GetPlatformBuffer();
}