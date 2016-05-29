#include "stdafx.h"
#include "GeometryBuffer.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "FBMathLib/GeomUtils.h"
using namespace fb;

GeometryBuffer GeometryBuffer::CreateSphereMesh(
	Real radius, int numRings, int numSegments)
{
	Vec3::Array pos;
	std::vector<unsigned short> index;
	GeomUtils::CreateSphere(radius, numRings, numSegments, pos, index);
	auto& renderer = Renderer::GetInstance();
	GeometryBuffer mesh;
	mesh.mVertexBuffer = renderer.CreateVertexBuffer(&pos[0], 12, pos.size(), BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE);
	mesh.mIndexBuffer = renderer.CreateIndexBuffer(&index[0], index.size(), INDEXBUFFER_FORMAT_16BIT);
	mesh.mTopology = PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	return mesh;
}

bool GeometryBuffer::Bind() {
	if (mVertexBuffer)
		mVertexBuffer->Bind();
	else
		return false;

	if (mIndexBuffer)
		mIndexBuffer->Bind(0);	

	Renderer::GetInstance().SetPrimitiveTopology(mTopology);

	return true;
}