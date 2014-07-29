#pragma once
#ifndef _VertexBuffer_header_included_
#define _VertexBuffer_header_included_

#include <Engine/Renderer/VertexBuffer.h>
#include <D3D11.h>
namespace fastbird
{
	class VertexBufferD3D11 : public VertexBuffer
	{
	public:
		static VertexBufferD3D11* CreateInstance(unsigned stride, unsigned numVertices);
		virtual bool IsReady() const;

	protected:
		VertexBufferD3D11(unsigned int stride, unsigned int num);
		virtual ~VertexBufferD3D11();

		
	private:
		friend class RendererD3D11;
		ID3D11Buffer* GetHardwareBuffer() const;
		void SetHardwareBuffer(ID3D11Buffer* pVertexBuffer);

	private:
		ID3D11Buffer* m_pVertexBuffer;

	};
}

#endif //_VertexBuffer_header_included_