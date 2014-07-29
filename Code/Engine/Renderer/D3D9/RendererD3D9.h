#pragma once
#ifndef __Rendererd3d9_header_included__
#define __Rendererd3d9_header_included__

#include <Engine/IRenderer.h>
#include <d3d9.h>

namespace fastbird
{
	class RendererD3D9 : public IRenderer
	{
	public:
		virtual bool Init(int threadPool);
		virtual void Deinit();
		virtual void Clear(float r, float g, float b, float z);

		virtual IVertexBuffer* CreateVertexBuffer(void* data, unsigned stride, unsigned numVertices);
		virtual IIndexBuffer* CreateIndexBuffer(void* data, unsigned int size, INDEXBUFFER_FORMAT format);

	private:
		IDirect3D9Ex* m_pD3D;
		IDirect3DDevice9Ex* m_pDevice;
	};
}

#endif //__Rendererd3d9_header_included__