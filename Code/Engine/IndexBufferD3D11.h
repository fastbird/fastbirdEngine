#pragma once
#ifndef _IndexBuffer_header_included_
#define _IndexBuffer_header_included_

#include <Engine/IIndexBuffer.h>
#include <D3D11.h>

namespace fastbird
{
	class IndexBufferD3D11 : public IIndexBuffer
	{
	public:
		virtual bool IsReady() const;
		virtual void Bind();
		virtual INDEXBUFFER_FORMAT GetFormat() const { return mFormat; }
		DXGI_FORMAT GetFormatD3D11() const { return mFormatD3D11; }
		unsigned GetSize() const { return mSize; }
		unsigned inline GetNumIndices() const { return mNumIndices; }
		
		unsigned GetOffset() const { return mOffset; }
		void SetOffset(unsigned offset) { mOffset = offset; }

	protected:
		IndexBufferD3D11(unsigned numIndices, INDEXBUFFER_FORMAT format);
		virtual ~IndexBufferD3D11();

	private:
		friend class RendererD3D11;
		static IndexBufferD3D11* CreateInstance(unsigned numIndices, INDEXBUFFER_FORMAT format);
		ID3D11Buffer* GetHardwareBuffer() const;
		void SetHardwareBuffer(ID3D11Buffer* pIndexBuffer);


	private:
		ID3D11Buffer* m_pIndexBuffer;
		unsigned mSize;
		unsigned mNumIndices;
		unsigned mOffset;
		INDEXBUFFER_FORMAT mFormat;
		DXGI_FORMAT mFormatD3D11;

	};
}

#endif //_IndexBuffer_header_included_