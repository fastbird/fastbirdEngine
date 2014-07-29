#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/DustRenderer.h>
#include <Engine/Renderer/D3DEventMarker.h>

namespace fastbird
{
IDustRenderer* IDustRenderer::CreateDustRenderer()
{
	return new DustRenderer();
}

//---------------------------------------------------------------------------
DustRenderer::DustRenderer()
	: mMin(0, 0, 0)
{
	SetMaterial("es/materials/DustRenderer.material");
}

DustRenderer::~DustRenderer()
{
}

void DustRenderer::InitDustRenderer(const Vec3& min, const Vec3& max, size_t count, 
			const Color& cmin, const Color& cmax,
			float normalizeDist)
{
	if (count==0)
		return;
	mMin = min;
	std::vector<Vec3> pos;
	pos.reserve(count);
	std::vector<DWORD> color;
	color.reserve(count);
	
	for (size_t i=0; i<count; i++)
	{
		pos.push_back(Random(min, max));
		color.push_back(Random(cmin, cmax).Get4Byte());
	}

	if (normalizeDist>0.0f)
	{
		FB_FOREACH(it, pos)
		{
			it->Normalize();
			(*it) *= normalizeDist;
		}
	}

	mVBPos = gFBEnv->pRenderer->CreateVertexBuffer(&pos[0], 
		sizeof(Vec3), count, BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	assert(mVBPos);

	mVBColor = gFBEnv->pRenderer->CreateVertexBuffer(&color[0], 
		sizeof(DWORD), count, BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);

	Vec3 len = max - min;
	mBoundingVolume->SetCenter(min + len/2.f);
	mBoundingVolume->SetRadius(len.Length());
	mBoundingVolumeWorld = mBoundingVolume;
}

//---------------------------------------------------------------------------
void DustRenderer::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
			return;
}

void DustRenderer::Render()
{
	if (mObjFlag & IObject::OF_HIDE)
			return;

	D3DEventMarker mark("DustRenderer");

	mMaterial->Bind(true);
	gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
	BindRenderStates();

	const unsigned int numBuffers = 2;
	IVertexBuffer* buffers[numBuffers] = {mVBPos, mVBColor};
	unsigned int strides[numBuffers] = {mVBPos->GetStride(), 
		mVBColor ? mVBColor->GetStride() : 0};
	unsigned int offsets[numBuffers] = {0, 0};
	gFBEnv->pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
	gFBEnv->pRenderer->Draw(mVBPos->GetNumVertices(), 0);
}

void DustRenderer::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
			return;
}

void DustRenderer::SetMaterial(const char* name)
{
	mMaterial = IMaterial::CreateMaterial(name);
	if (!mMaterial)
		Log("Failed to load material %s", name);
}


}

