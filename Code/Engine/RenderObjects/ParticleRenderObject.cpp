#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>
#include <Engine/IConsole.h>

namespace fastbird
{

ParticleRenderObject::RENDER_OBJECTS ParticleRenderObject::mRenderObjects;
size_t ParticleRenderObject::mNumDrawCalls = 0;
size_t ParticleRenderObject::mNumDrawPrimitives = 0;

const int ParticleRenderObject::MAX_SHARED_VERTICES = 5000;
ParticleRenderObject* ParticleRenderObject::GetRenderObject(const char* texturePath)
{
	assert(texturePath);
	RENDER_OBJECTS::iterator it = mRenderObjects.Find(texturePath);
	if (it != mRenderObjects.end())
		return it->second;
	ParticleRenderObject* p = FB_NEW(ParticleRenderObject);
	p->SetTexture(texturePath);
	p->AttachToScene();
	mRenderObjects.Insert(RENDER_OBJECTS::value_type(texturePath, p));

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
	, mGlow(true)
	, mMaxVertices(MAX_SHARED_VERTICES)
	, mLastFrameNumVertices(0)
{
	mMaterial = IMaterial::CreateMaterial("es/materials/particle.material");
	mBoundingVolumeWorld->SetAlwaysPass(true);
	BLEND_DESC desc;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;

	SetBlendState(desc);

	DEPTH_STENCIL_DESC ddesc;
	ddesc.DepthEnable = true;
	ddesc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	SetDepthStencilState(ddesc);
}
ParticleRenderObject::~ParticleRenderObject()
{
}

void ParticleRenderObject::SetDoubleSided(bool set)
{
	if (mDoubleSided == set)
		return;

	if (set)
	{
		mDoubleSided = set;
		RASTERIZER_DESC desc;
		desc.CullMode = CULL_MODE_NONE;
		SetRasterizerState(desc);
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
	BindRenderStates();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
	
	//draw
	UINT num = 0;
	UINT start = mBatches[0].first;
	if (mGlow)
		pRenderer->SetGlowRenderTarget();
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
	if (mGlow)
		pRenderer->UnSetGlowRenderTarget();

	if (mDoubleSided)
	{
		pRenderer->RestoreRasterizerState();
	}
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

void ParticleRenderObject::SetGlow(bool glow)
{
	mGlow = glow;
}

}