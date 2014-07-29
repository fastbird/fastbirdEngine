#pragma once
#ifndef _IVertexBuffer_header_included__
#define _IVertexBuffer_header_included__

#include <CommonLib/SmartPtr.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class IVertexBuffer : public ReferenceCounter
	{
	public:
		virtual ~IVertexBuffer() {}
		virtual bool IsReady() const = 0;
		virtual unsigned GetStride() const = 0;
		virtual unsigned GetNumVertices() const = 0;
		virtual void Bind() = 0;
		virtual MapData Map(MAP_TYPE type, UINT subResource, MAP_FLAG flag) = 0;
		virtual void Unmap() = 0;
	};
}

#endif //_IVertexBuffer_header_included__