#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>

namespace fastbird
{

ParticleRenderObject::RENDER_OBJECTS ParticleRenderObject::mRenderObjects;
SmartPtr<IMaterial> ParticleRenderObject::mMaterial;
const int ParticleRenderObject::MAX_SHARED_VERTICES = 5000;
ParticleRenderObject* ParticleRenderObject::GetRenderObject(const char* texturePath)
{
	assert(texturePath);
	std::string id = texturePath;
	ToLowerCase(id);
	RENDER_OBJECTS::iterator it = mRenderObjects.Find(id.c_str());
	if (it != mRenderObjects.end())
		return it->second;

	ParticleRenderObject* p = new ParticleRenderObject;
	p->AttachToScene();
	mRenderObjects.Insert(RENDER_OBJECTS::value_type(id.c_str(), p));
	return p;
}

void ParticleRenderObject::ClearParticles()
{
	FB_FOREACH(it, mRenderObjects)
	{
		(*it).second->Clear();
	}
}

void ParticleRenderObject::FinalizeRenderObjects()
{
	mRenderObjects.clear();
}

ParticleRenderObject::ParticleRenderObject()
	: mNextMap(0)
	, mMapped(0)
{
	if (!mMaterial)
	{
		mMaterial = IMaterial::CreateMaterial("es/materials/particle.material");
	}

	mBoundingVolumeWorld->SetAlwaysPass(true);
}
ParticleRenderObject::~ParticleRenderObject()
{
}

void ParticleRenderObject::PreRender()
{
}

void ParticleRenderObject::Render()
{
	if (!mVertexBuffer || mBatches.empty())
		return;

	if (mMapped)
	{
		mVertexBuffer->Unmap();
		mMapped = 0;
	}

	mMaterial->Bind(true);
	mVertexBuffer->Bind();
	gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT num=0;
	UINT start= mBatches[0].first;
	for each(auto it in mBatches)
	{
		if (start > it.first)
		{
			// draw
			assert(start+num <= MAX_SHARED_VERTICES);
			gFBEnv->pRenderer->Draw(num, start);
			start = it.first;
			num = 0;
		}
		num += it.second;
	}
	if (num)
	{
		gFBEnv->pRenderer->Draw(num, start);
	}
}

void ParticleRenderObject::PostRender()
{
}

void ParticleRenderObject::Clear()
{
	mBatches.clear();
}

ParticleRenderObject::Vertex* ParticleRenderObject::Map(UINT numVertices)
{
	assert(numVertices < MAX_SHARED_VERTICES);
	if (!mVertexBuffer)
		mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(
			0, sizeof(Vertex), MAX_SHARED_VERTICES, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

	if (mNextMap + numVertices >= MAX_SHARED_VERTICES)
	{
		if (!mBatches.empty())
		{
			assert(mBatches[0].first > numVertices);
		}
		mNextMap = 0;
	}

	if (!mMapped)
	{
		MapData m = mVertexBuffer->Map(MAP_TYPE_WRITE_NO_OVERWRITE, 0, MAP_FLAG_NONE);
		assert(m.pData);
		mMapped = (Vertex*)m.pData;
	}
	
	mBatches.push_back( BATCHES::value_type(mNextMap, numVertices) );
	Vertex* p = mMapped;
	p += mNextMap;
	mNextMap+=numVertices;

	return p;
	
}

void ParticleRenderObject::Unmap()
{
	/*mSharedVertexBuffer->Unmap();
	mMapped = 0;*/
}

}