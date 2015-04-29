#pragma once
#include <Engine/IVertexBuffer.h>

namespace fastbird
{

class VertexBuffer : public IVertexBuffer
{
public:
	virtual ~VertexBuffer(){}
	virtual void Delete();

	virtual void Bind();
	virtual MapData Map(MAP_TYPE type, UINT subResource, MAP_FLAG flag);
	virtual void Unmap();
	virtual unsigned GetStride() const { return mStride; }
	virtual unsigned GetNumVertices() const {return mNumVertices; }
	
protected:
	unsigned mStride;
	unsigned mNumVertices;

};


}