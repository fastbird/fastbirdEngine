#include <Engine/StdAfx.h>
#include <Engine/VertexBufferD3D11.h>

#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/RendererD3D11.h>

namespace fastbird
{

	VertexBufferD3D11* VertexBufferD3D11::CreateInstance(unsigned int stride, unsigned int numVertices)
	{
		VertexBufferD3D11* pVertexBuffer = FB_NEW(VertexBufferD3D11)(stride, numVertices);
		return pVertexBuffer;
	}

	VertexBufferD3D11::VertexBufferD3D11(unsigned int stride, unsigned int num)
		: m_pVertexBuffer(0)
	{
		mStride = stride;
		mNumVertices = num;
	}

	VertexBufferD3D11::~VertexBufferD3D11()
	{
		SAFE_RELEASE(m_pVertexBuffer);
	}

	bool VertexBufferD3D11::IsReady() const
	{
		return m_pVertexBuffer != 0;
	}

	ID3D11Buffer* VertexBufferD3D11::GetHardwareBuffer() const
	{
		return m_pVertexBuffer;
	}

	void VertexBufferD3D11::SetHardwareBuffer(ID3D11Buffer* pVertexBuffer)
	{
		SAFE_RELEASE(m_pVertexBuffer);
		m_pVertexBuffer = pVertexBuffer;
	}
}