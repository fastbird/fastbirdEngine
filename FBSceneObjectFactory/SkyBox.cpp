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
#include "SkyBox.h"
#include "FBRenderer/VertexBuffer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Camera.h"

using namespace fb;

namespace fb{
	struct A2V
	{
		Vec3 pos;
		Vec3 uvw;
	};

	enum SKYBOX_PLANE
	{
		SKYBOX_PLANE_FRONT,
		SKYBOX_PLANE_BACK,
		SKYBOX_PLANE_LEFT,
		SKYBOX_PLANE_RIGHT,
		SKYBOX_PLANE_UP,
		SKYBOX_PLANE_DOWN
	};
}

class SkyBox::Impl{
public:
	VertexBufferPtr mVertexBuffer;
	MaterialPtr mMaterial;

	Impl(const char* materialPath) {
		std::vector<A2V> cubeVertices;
		cubeVertices.reserve(36);
		float n, f;
		auto& renderer = Renderer::GetInstance();
		auto camera = renderer.GetMainCamera();
		camera->GetNearFar(n, f);
		float distance = (f) / ROOT_3;
		// Set up the box (6 planes)
		for (int p = 0; p < 6; ++p)
		{
			Vec3 middle;
			Vec3 up, right;

			switch (p)
			{
			case SKYBOX_PLANE_FRONT:
				middle = Vec3(0, distance, 0);
				up = Vec3::UNIT_Z * distance;
				right = Vec3::UNIT_X * distance;
				break;
			case SKYBOX_PLANE_BACK:
				middle = Vec3(0, -distance, 0);
				up = Vec3::UNIT_Z * distance;
				right = -Vec3::UNIT_X * distance;
				break;
			case SKYBOX_PLANE_LEFT:
				middle = Vec3(-distance, 0, 0);
				up = Vec3::UNIT_Z * distance;
				right = Vec3::UNIT_Y * distance;
				break;
			case SKYBOX_PLANE_RIGHT:
				middle = Vec3(distance, 0, 0);
				up = Vec3::UNIT_Z * distance;
				right = -Vec3::UNIT_Y * distance;
				break;
			case SKYBOX_PLANE_UP:
				middle = Vec3(0, 0, distance);
				up = -Vec3::UNIT_Y * distance;
				right = Vec3::UNIT_X * distance;
				break;
			case SKYBOX_PLANE_DOWN:
				middle = Vec3(0, 0, -distance);
				up = Vec3::UNIT_Y * distance;
				right = Vec3::UNIT_X * distance;
				break;
			}

			// bottom left
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle - up - right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();
			// top left
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle + up - right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();
			//bottom right
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle - up + right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();

			//bottom right
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle - up + right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();
			// top left
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle + up - right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();
			// top right
			cubeVertices.push_back(A2V());
			cubeVertices.back().pos = middle + up + right;
			cubeVertices.back().uvw = cubeVertices.back().pos.NormalizeCopy();
		} // for each plane

		mVertexBuffer = renderer.CreateVertexBuffer(&cubeVertices[0], sizeof(A2V), cubeVertices.size(),
			BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
		assert(mVertexBuffer);

		// Init Material
		SetMaterial(materialPath);
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		RenderEventMarker  mark("SkyBox");
		mMaterial->Bind(true);
		mVertexBuffer->Bind();
		auto& renderer = Renderer::GetInstance();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		renderer.Draw(mVertexBuffer->GetNumVertices(), 0);
	}

	void SetMaterial(const char* path){
		if (ValidCStringLength(path))
			mMaterial = Renderer::GetInstance().CreateMaterial(path);
	}
};

//---------------------------------------------------------------------------
SkyBoxPtr SkyBox::Create(const char* materialPath){
	return SkyBoxPtr(new SkyBox(materialPath), [](SkyBox* obj){ delete obj; });
}

SkyBox::SkyBox(const char* materialPath)
	: mImpl(new Impl(materialPath)){

}
SkyBox::~SkyBox(){
}

void SkyBox::PreRender(const RenderParam& param, RenderParamOut* paramOut){

}

void SkyBox::Render(const RenderParam& param, RenderParamOut* paramOut){
	mImpl->Render(param, paramOut);
}

void SkyBox::PostRender(const RenderParam& param, RenderParamOut* paramOut){

}

void SkyBox::SetMaterial(const char* path){
	mImpl->SetMaterial(path);
}

MaterialPtr SkyBox::GetMaterial() const{
	return mImpl->mMaterial;
}