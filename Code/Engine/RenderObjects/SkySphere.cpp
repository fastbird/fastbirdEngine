#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/SkySphere.h>
#include <Engine/ICamera.h>
#include <CommonLib/Math/GeomUtils.h>

namespace fastbird
{

ISkySphere* ISkySphere::CreateSkySphere()
{
	return new SkySphere;
}

SkySphere::SkySphere()
{
	//GenerateSphereMesh();
	SetMaterial("data/materials/skysphere.material");
	SetRasterizerState(RASTERIZER_DESC());
	DEPTH_STENCIL_DESC desc;
	desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	SetDepthStencilState(desc);
}

SkySphere::~SkySphere()
{
}

void SkySphere::SetMaterial(const char* name)
{
	IMaterial* pMat = IMaterial::CreateMaterial(name);
	if (pMat)
		mMaterial = pMat;
}

void SkySphere::SetMaterial(IMaterial* pMat)
{
	mMaterial = pMat;
}

IMaterial* SkySphere::GetMaterial() const
{
	return mMaterial;
}

void SkySphere::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
}

void SkySphere::Render()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	if (!mMaterial)
	{
		assert(0);
		return;
	}

	mMaterial->Bind(true);
	BindRenderStates();
	gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	int num = gFBEnv->pRenderer->BindFullscreenQuadUV_VB(true);
	gFBEnv->pRenderer->Draw(num, 0);
}

void SkySphere::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
}

void SkySphere::GenerateSphereMesh()
{
	float n, f;
	gFBEnv->pRenderer->GetCamera()->GetNearFar(n, f);
	float radius =  f * 0.95f;
	std::vector<Vec3> pos;
	std::vector<UINT16> index;
	CreateSphereMesh(radius, 16, 16, pos, index, 0, 0);
	assert(!pos.empty() && !index.empty());

	mVB = gFBEnv->pRenderer->CreateVertexBuffer(&pos[0], sizeof(Vec3), pos.size(), 
		BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	mIB = gFBEnv->pRenderer->CreateIndexBuffer(&index[0], index.size(), INDEXBUFFER_FORMAT_16BIT);
	assert(mVB && mIB);
}

}