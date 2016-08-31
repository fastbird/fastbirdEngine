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