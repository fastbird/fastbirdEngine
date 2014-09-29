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
	std::string id = texturePath;
	ToLowerCase(id);
	RENDER_OBJECTS::iterator it = mRenderObjects.Find(id.c_str());
	if (it != mRenderObjects.end())
		return it->second;
	ParticleRenderObject* p = FB_NEW(ParticleRenderObject);
	p->SetTexture(texturePath);
	p->AttachToScene();
	mRenderObjects.Insert(RENDER_OBJECTS::value_type(id.c_str(), p));

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

void ParticleRenderObject::FinalizeRenderObjects()
{
	mRenderObjects.clear();
}

ParticleRenderObject::ParticleRenderObject()
	: mNextMap(0)
	, mMapped(0)
	, mDoubleSided(false)
	, mGlow(true)
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
	mMaterial->SetTexture(texturePath, BINDING_SHADER_PS, 0);
}

//---------------------------------------------------------------------------
IMaterial* ParticleRenderObject::GetMaterial(int pass) const
{
	if (pass == RENDER_PASS::PASS_NORMAL)
		return mMaterial;
	return 0;
}

void ParticleRenderObject::PreRender()
{
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
			assert(start + num <= MAX_SHARED_VERTICES);
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
		num = std::min(num, (UINT)MAX_SHARED_VERTICES);
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
}

ParticleRenderObject::Vertex* ParticleRenderObject::Map(UINT numVertices)
{
	assert(numVertices < MAX_SHARED_VERTICES/2);
	if (!mVertexBuffer)
		mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(
			0, sizeof(Vertex), MAX_SHARED_VERTICES, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

	if (mNextMap + numVertices >= MAX_SHARED_VERTICES)
	{
		if (!mBatches.empty())
		{
			if (mBatches[0].first <= numVertices)
				Log("MAX_SHARED_VERTICES exceeded!");
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

void ParticleRenderObject::SetGlow(bool glow)
{
	mGlow = glow;
}

}