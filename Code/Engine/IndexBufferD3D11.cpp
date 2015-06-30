#include <Engine/StdAfx.h>
#include <Engine/IndexBufferD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>

namespace fastbird
{
	void IndexBufferD3D11::FinishSmartPtr(){
		FB_DELETE(this);
	}

	IndexBufferD3D11* IndexBufferD3D11::CreateInstance(unsigned int numIndices, INDEXBUFFER_FORMAT format)
	{
		IndexBufferD3D11* pIndexBuffer = FB_NEW(IndexBufferD3D11)(numIndices, format);
		return pIndexBuffer;
	}

	IndexBufferD3D11::IndexBufferD3D11(unsigned numIndices, INDEXBUFFER_FORMAT format)
		: m_pIndexBuffer(0)
		, mFormat(format), mOffset(0), mNumIndices(numIndices)
	{
		switch(format)
		{
		case INDEXBUFFER_FORMAT_16BIT:
			mSize = numIndices * 2;
			mFormatD3D11 = DXGI_FORMAT_R16_UINT;
			break;
		case INDEXBUFFER_FORMAT_32BIT:
			mSize = numIndices * 4;
			mFormatD3D11 = DXGI_FORMAT_R32_UINT;
			break;

		}
		
	}

	IndexBufferD3D11::~IndexBufferD3D11()
	{
		SAFE_RELEASE(m_pIndexBuffer);
	}

	bool IndexBufferD3D11::IsReady() const
	{
		return m_pIndexBuffer != 0;
	}

	void IndexBufferD3D11::Bind()
	{
		assert(m_pIndexBuffer);
		if (!m_pIndexBuffer)
			return;

		gFBEnv->pEngine->GetRenderer()->SetIndexBuffer(this);

	}

	ID3D11Buffer* IndexBufferD3D11::GetHardwareBuffer() const
	{
		return m_pIndexBuffer;
	}

	void IndexBufferD3D11::SetHardwareBuffer(ID3D11Buffer* pIndexBuffer)
	{
		SAFE_RELEASE(m_pIndexBuffer);
		m_pIndexBuffer = pIndexBuffer;
	}





}