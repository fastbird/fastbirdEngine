#include <Engine/StdAfx.h>
#include <Engine/Renderer/VertexBuffer.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>

namespace fastbird
{

void VertexBuffer::Bind()
{
	unsigned int offset = 0;
	IVertexBuffer* buffers[] = {this};
	gFBEnv->pRenderer->SetVertexBuffer(0, 1, buffers, &mStride, &offset);
}

MapData VertexBuffer::Map(MAP_TYPE type, UINT subResource, MAP_FLAG flag)
{
	return gFBEnv->pRenderer->MapVertexBuffer(this, subResource, type, flag);
}

void VertexBuffer::Unmap()
{
	gFBEnv->pRenderer->UnmapVertexBuffer(this, 0);
}


}