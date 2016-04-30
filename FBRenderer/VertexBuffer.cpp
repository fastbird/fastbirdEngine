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
	BUFFER_USAGE mUsage;
	BUFFER_CPU_ACCESS_FLAG mAccessFlag;
	IPlatformVertexBufferPtr mPlatformBuffer;
	//---------------------------------------------------------------------------
	Impl(unsigned stride, unsigned numVertices,
		BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag)
		: mStride(stride)
		, mNumVertices(numVertices)
		, mUsage(usage)
		, mAccessFlag(accessFlag)
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

	bool IsSame(unsigned stride, unsigned numVertices, BUFFER_USAGE usage,
		BUFFER_CPU_ACCESS_FLAG accessFlag) const {
		return mStride == stride && mNumVertices == numVertices &&
			mUsage == usage && mAccessFlag == accessFlag;
	}

	bool UpdateData(void* data) {		
		if (mUsage == BUFFER_USAGE::BUFFER_USAGE_DYNAMIC) {
			auto map = Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
			if (!map.pData)
				return false;
			memcpy(map.pData, data, mStride * mNumVertices);
			Unmap(0);
			return true;
		}
		else if (mUsage != BUFFER_USAGE::BUFFER_USAGE_IMMUTABLE) {
			return mPlatformBuffer->UpdateBuffer(data, mStride * mNumVertices);
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"Cannot update vertex buffer(usage : %d)", mUsage).c_str());
			return false;
		}
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
VertexBufferPtr VertexBuffer::Create(unsigned stride, unsigned numVertices, 
	BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag)
{
	return VertexBufferPtr(new VertexBuffer(stride, numVertices, usage, accessFlag), 
		[](VertexBuffer* obj){ delete obj; });
}

VertexBuffer::VertexBuffer(unsigned stride, unsigned numVertices,
	BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag)
	: mImpl(new Impl(stride, numVertices, usage, accessFlag))
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

BUFFER_USAGE VertexBuffer::GetBufferUsage() const {
	return mImpl->mUsage;
}

void VertexBuffer::SetPlatformBuffer(IPlatformVertexBufferPtr buffer){
	mImpl->SetPlatformBuffer(buffer);
}

IPlatformVertexBufferPtr VertexBuffer::GetPlatformBuffer() const{
	return mImpl->GetPlatformBuffer();
}

bool VertexBuffer::IsSame(unsigned stride, unsigned numVertices, BUFFER_USAGE usage,
	BUFFER_CPU_ACCESS_FLAG accessFlag) const {
	return mImpl->IsSame(stride, numVertices, usage, accessFlag);
}

bool VertexBuffer::UpdateData(void* data) {
	return mImpl->UpdateData(data);
}