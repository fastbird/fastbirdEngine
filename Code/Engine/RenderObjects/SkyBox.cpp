#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/SkyBox.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/IInputLayout.h>
#include <Engine/ICamera.h>
#include <CommonLib/Math/fbMath.h>

using namespace fastbird;

//----------------------------------------------------------------------------
ISkyBox* ISkyBox::CreateSkyBoxInstance()
{
	return FB_NEW(SkyBox);
}


//----------------------------------------------------------------------------
struct A2V
{
	Vec3 pos;
	Vec3 uvw;
};

//----------------------------------------------------------------------------
SkyBox::SkyBox()
{
	DEPTH_STENCIL_DESC desc;
	desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	SetDepthStencilState(desc);
	
}

//----------------------------------------------------------------------------
SkyBox::~SkyBox()
{
}

//----------------------------------------------------------------------------
void SkyBox::Init()
{
	std::vector<A2V> cubeVertices;
	cubeVertices.reserve(36);
	float n, f;
	gFBEnv->pRenderer->GetCamera()->GetNearFar(n, f);
	float distance =  (f) / ROOT_3;
	// Set up the box (6 planes)
    for (int p = 0; p < 6; ++p)
    {
		Vec3 middle;
		Vec3 up, right;

		switch(p)
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

	mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(&cubeVertices[0], sizeof(A2V), cubeVertices.size(), 
		BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	assert(mVertexBuffer);
	mBoundingVolume->SetCenter(Vec3(0, 0, 0));
	mBoundingVolume->SetRadius(10000000);

	// Init Material
	mMaterial = fastbird::IMaterial::CreateMaterial("data/materials/skybox.material");
	assert(mMaterial);
}

void SkyBox::Render()
{
	D3DEventMarker devent("Rendering Sky");
	mMaterial->Bind(true);
	BindRenderStates();
	mVertexBuffer->Bind();
	gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gFBEnv->pRenderer->Draw(mVertexBuffer->GetNumVertices(), 0);
}