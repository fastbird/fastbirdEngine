#include <Engine/StdAfx.h>
#include <Engine/ParticleRenderObject.h>
#include <Engine/IConsole.h>

namespace fastbird
{

ParticleRenderObject::RENDER_OBJECTS ParticleRenderObject::mRenderObjects;
size_t ParticleRenderObject::mNumDrawCalls = 0;
size_t ParticleRenderObject::mNumDrawPrimitives = 0;

const int ParticleRenderObject::MAX_SHARED_VERTICES = 5000;
ParticleRenderObject* ParticleRenderObject::GetRenderObject(Key& key, bool& created)
{
	RENDER_OBJECTS::iterator it = mRenderObjects.Find(key);
	if (it != mRenderObjects.end())
	{
		created = false;
		return it->second;
	}
	created = true;
	ParticleRenderObject* p = FB_NEW(ParticleRenderObject);
	auto material = p->GetMaterial();
	if_assert_pass(material)
	{
		material->SetBlendState(key.mBDesc);
	}
	p->SetTexture(key.mTexturePath);
	p->AttachToScene();
	mRenderObjects.Insert(RENDER_OBJECTS::value_type(key, p));
	return p;
}

size_t ParticleRenderObject::GetNumRenderObject()
{
	return mRenderObjects.size();
}

size_t ParticleRenderObject::GetNumDrawCalls()
{
	return mNumDrawCalls;
}
size_t ParticleRenderObject::GetNumPrimitives()
{
	return mNumDrawPrimitives;
}

void ParticleRenderObject::ClearParticles()
{
	FB_FOREACH(it, mRenderObjects)
	{
		(*it).second->Clear();
	}

	mNumDrawCalls = 0;
	mNumDrawPrimitives = 0;
}

void ParticleRenderObject::EndUpdateParticles()
{
	for (auto it : mRenderObjects)
	{
		it.second->EndUpdate();
	}
}

void ParticleRenderObject::FinalizeRenderObjects()
{
	mRenderObjects.clear();
}

ParticleRenderObject::ParticleRenderObject()
	: mNextMap(0)
	, mMapped(0)
	, mDoubleSided(false)
	, mMaxVertices(MAX_SHARED_VERTICES)
	, mLastFrameNumVertices(0)
{
	mMaterial = IMaterial::CreateMaterial("es/materials/particle.material");
	mMaterial->CloneRenderStates();
	mBoundingVolumeWorld->SetAlwaysPass(true);
}
ParticleRenderObject::~ParticleRenderObject()
{
}

void ParticleRenderObject::SetDoubleSided(bool set)
{
	if (mDoubleSided == set)
		return;

	if (!mMaterial)
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Doesn't have material.");
		return;
	}

	if (set)
	{
		mDoubleSided = set;
		RASTERIZER_DESC desc;
		desc.CullMode = CULL_MODE_NONE;
		mMaterial->SetRasterizerState(desc);
	}
	else
	{
		mMaterial->ClearRasterizerState();
	}
}

void ParticleRenderObject::SetTexture(const char* texturePath)
{
	mTextureName = texturePath;
	mMaterial->SetTexture(texturePath, BINDING_SHADER_PS, 0);
}

//---------------------------------------------------------------------------
IMaterial* ParticleRenderObject::GetMaterial(int pass) const
{
	if (pass == RENDER_PASS::PASS_NORMAL)
		return mMaterial;
	return 0;
}

void ParticleRenderObject::Render()
{
	D3DEventMarker mark("ParticleRenderObject");
	if (mMapped)
	{
		mVertexBuffer->Unmap();
		mMapped = 0;
	}

	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL || !mVertexBuffer || mBatches.empty() ||
		gFBEnv->pConsole->GetEngineCommand()->r_noParticleDraw)
		return;

	IRenderer* pRenderer = gFBEnv->pRenderer;
	mMaterial->Bind(true);
	mVertexBuffer->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
	
	/*for (auto batch : mBatches){
		++mNumDrawCalls;
		pRenderer->Draw(batch.second, batch.first);
		mNumDrawPrimitives += batch.second;
	}*/

	//draw
	UINT num = 0;
	UINT start = mBatches[0].first;
	FB_FOREACH(it, mBatches)
	{
		if (start > it->first)
		{
			// draw
			assert(start + num <= mMaxVertices);
			pRenderer->Draw(num, start);
			++mNumDrawCalls;
			mNumDrawPrimitives += num;
			start = it->first;
			num = 0;
		}
		num += it->second;
	}
	if (num)
	{
		assert(num < mMaxVertices);
		pRenderer->Draw(num, start);
		++mNumDrawCalls;
		mNumDrawPrimitives += num;
	}
	mMaterial->Unbind();
}

void ParticleRenderObject::PostRender()
{
}

void ParticleRenderObject::Clear()
{
	mBatches.clear();
	if (mLastFrameNumVertices * 3 >= mMaxVertices)
	{
		mMaxVertices *= 2;
		mVertexBuffer = 0;
		Log("Particle Vertex Buffer(%s) resized to : %u", mTextureName.c_str(), mMaxVertices);
	}
	mLastFrameNumVertices = 0;
}

void ParticleRenderObject::EndUpdate()
{
	if (mVertexBuffer && mMapped)
	{
		mVertexBuffer->Unmap();
		mMapped = 0;
	}
}

ParticleRenderObject::Vertex* ParticleRenderObject::Map(UINT numVertices, unsigned& canWrite)
{
	mLastFrameNumVertices += numVertices;
	assert(numVertices < mMaxVertices / 2);
	if (!mVertexBuffer)
		mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(
		0, sizeof(Vertex), mMaxVertices, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

	canWrite = numVertices;
	if (mNextMap + numVertices >= mMaxVertices)
	{
		if (!mBatches.empty())
		{
			if (mBatches[0].first < numVertices)
			{
				canWrite = mBatches[0].first;
			}				
		}
		mNextMap = 0;
	}
	if (canWrite == 0)
		return 0;
	
	if (!mMapped)
	{
		MapData m = mVertexBuffer->Map(MAP_TYPE_WRITE_NO_OVERWRITE, 0, MAP_FLAG_NONE);
		assert(m.pData);
		mMapped = (Vertex*)m.pData;
	}
	
	mBatches.push_back(BATCHES::value_type(mNextMap, canWrite));
	Vertex* p = mMapped;
	p += mNextMap;
	mNextMap += canWrite;

	return p;
	
}

}