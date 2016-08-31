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
#include "ParticleRenderObject.h"
#include "ParticleRenderKey.h"
#include "FBRenderer/VertexBuffer.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Material.h"
#include "FBSceneManager/SceneManager.h"
#include "FBSceneManager/Scene.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

namespace fb{
	typedef std::unordered_map< ParticleRenderKey, ParticleRenderObjectPtr > RENDER_OBJECTS;
	static RENDER_OBJECTS sRenderObjects;
	static size_t sNumDrawCalls;
	static size_t sNumDrawPrimitives;
	const int ParticleRenderObject::MAX_SHARED_VERTICES = 5000;

	void ClearParticleRenderObjects(){
		sRenderObjects.clear();
	}
}

class ParticleRenderObject::Impl{
public:	
	ParticleRenderObject* mSelf;
	MaterialPtr mMaterial;	
	unsigned mMaxVertices;
	unsigned mLastFrameNumVertices;
	std::string mTextureName;
	VertexBufferPtr mVertexBuffer;
	UINT mNextMap;
	typedef std::vector< std::pair<UINT, UINT> > BATCHES;
	BATCHES mBatches;
	Vertex* mMapped;
	bool mDoubleSided;
	int mScreenspace;

	//---------------------------------------------------------------------------
	Impl(ParticleRenderObject* self)
		: mSelf(self)
		, mNextMap(0)
		, mMapped(0)
		, mDoubleSided(false)
		, mMaxVertices(MAX_SHARED_VERTICES)
		, mLastFrameNumVertices(0)
		, mScreenspace(0)
	{
		mMaterial = Renderer::GetInstance().CreateMaterial("EssentialEngineData/materials/particle.material");
		mSelf->GetBoundingVolumeWorld()->SetAlwaysPass(true);
		mSelf->ModifyObjFlag(SceneObjectFlag::Transparent, true);
	}

	//---------------------------------------------------------------------------
	// IRenderable Interfaces
	//---------------------------------------------------------------------------
	void PreRender(const RenderParam& param, RenderParamOut* paramOut){

	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){		
		RenderEventMarker mark("ParticleRenderObject");
		if (mMapped)
		{
			mVertexBuffer->Unmap(0);
			mMapped = 0;
		}

		if (param.mRenderPass != RENDER_PASS::PASS_NORMAL || !mVertexBuffer || mBatches.empty())
			return;

		auto& renderer = Renderer::GetInstance();
		mMaterial->Bind(true);
		mVertexBuffer->Bind();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
		if (mScreenspace) {
			static unsigned lastFrame = -1;
			if (lastFrame != gpTimer->GetFrame()) {
				lastFrame = gpTimer->GetFrame();
				auto& mat = renderer.GetScreenToNDCMatric();
				OBJECT_CONSTANTS cont;
				cont.gWorldViewProj = mat;
				renderer.UpdateObjectConstantsBuffer(&cont);
			}
		}

		/*for (auto batch : mBatches){
		++sNumDrawCalls;
		pRenderer->Draw(batch.second, batch.first);
		sNumDrawPrimitives += batch.second;
		}*/

		//draw
		UINT num = 0;
		UINT start = mBatches[0].first;
		for(const auto& it : mBatches)
		{
			if (start > it.first)
			{
				// draw
				assert(start + num <= mMaxVertices);
				renderer.Draw(num, start);
				++sNumDrawCalls;
				sNumDrawPrimitives += num;
				start = it.first;
				num = 0;
			}
			num += it.second;
		}
		if (num)
		{
			//assert(num < mMaxVertices);
			renderer.Draw(num, start);
			++sNumDrawCalls;
			sNumDrawPrimitives += num;
		}
		mMaterial->Unbind();
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){

	}

	//---------------------------------------------------------------------------
	// OWN
	//---------------------------------------------------------------------------
	MaterialPtr GetMaterial() const{
		return mMaterial;
	}

	void Clear(){
		mBatches.clear();
		if (mLastFrameNumVertices * 3 >= mMaxVertices)
		{
			mMaxVertices *= 2;
			mVertexBuffer = 0;
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Particle Vertex Buffer(%s) resized to : %u", mTextureName.c_str(), mMaxVertices).c_str());
		}
		mLastFrameNumVertices = 0;
	}

	void EndUpdate(){
		if (mVertexBuffer && mMapped)
		{
			mVertexBuffer->Unmap(0);
			mMapped = 0;
		}
	}

	void SetDoubleSided(bool set){
		if (mDoubleSided == set)
			return;

		if (!mMaterial)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Doesn't have material.");			
			return;
		}

		if (set)
		{
			mDoubleSided = set;
			RASTERIZER_DESC desc;
			desc.SetCullMode(CULL_MODE_NONE);
			mMaterial->SetRasterizerState(desc);
		}
		else
		{
			mMaterial->ClearRasterizerState();
		}
	}

	void SetTexture(const char* texturePath){
		mTextureName = texturePath;
		if (mMaterial)
			mMaterial->SetTexture(texturePath, SHADER_TYPE_PS, 0, TEXTURE_TYPE_DEFAULT);
	}

	void SetScreenspace(int set) {
		mScreenspace = set;
		switch (set) {
		case 1: // normal
		{
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterRenderObjects, false);
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterUI, false);
			break;
		}
		case 2: // after objects
		{
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterRenderObjects, true);
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterUI, false);
			break;
		}
		case 3: // after ui
		{
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterRenderObjects, false);
			mSelf->ModifyObjFlag(SceneObjectFlag::AfterUI, true);
			break;
		}
		}
	}

	Vertex* Map(UINT numVertices, unsigned& canWrite){
		mLastFrameNumVertices += numVertices;
		assert(numVertices < mMaxVertices / 2);
		if (!mVertexBuffer)
			mVertexBuffer = Renderer::GetInstance().CreateVertexBuffer(
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
			MapData m = mVertexBuffer->Map(0, MAP_TYPE_WRITE_NO_OVERWRITE, MAP_FLAG_NONE);
			assert(m.pData);
			mMapped = (Vertex*)m.pData;
		}

		mBatches.push_back(BATCHES::value_type(mNextMap, canWrite));
		Vertex* p = mMapped;
		p += mNextMap;
		mNextMap += canWrite;

		return p;
	}

};

//---------------------------------------------------------------------------
ParticleRenderObjectPtr ParticleRenderObject::GetRenderObject(IScenePtr scene, 
	const ParticleRenderKey& key, bool& created)
{
	if (!scene){
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return 0;
	}

	RENDER_OBJECTS::iterator it = sRenderObjects.find(key);
	if (it != sRenderObjects.end())
	{
		created = false;		
		return it->second;
	}

	created = true;	
	ParticleRenderObjectPtr p(new ParticleRenderObject, [](ParticleRenderObject* obj){ delete obj; });
	auto material = p->GetMaterial();
	if_assert_pass(material)
	{
		material->SetBlendState(key.mBDesc);
	}
	p->SetTexture(key.mTexturePath);

	if (scene){
		scene->AttachObjectFB(p);
	}
	
	sRenderObjects[key] = p;
	return p;
}

size_t ParticleRenderObject::GetNumRenderObject()
{
	return sRenderObjects.size();
}

size_t ParticleRenderObject::GetNumDrawCalls()
{
	return sNumDrawCalls;
}
size_t ParticleRenderObject::GetNumPrimitives()
{
	return sNumDrawPrimitives;
}

void ParticleRenderObject::ClearParticles()
{
	for (auto& it : sRenderObjects){
		it.second->Clear();
	}

	sNumDrawCalls = 0;
	sNumDrawPrimitives = 0;
}

void ParticleRenderObject::EndUpdateParticles()
{
	for (auto it : sRenderObjects)
	{
		it.second->EndUpdate();
	}
}

void ParticleRenderObject::FinalizeRenderObjects()
{
	sRenderObjects.clear();
}

ParticleRenderObject::ParticleRenderObject()
	: mImpl(new Impl(this))
{

}

ParticleRenderObject::~ParticleRenderObject(){	
}

void ParticleRenderObject::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void ParticleRenderObject::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void ParticleRenderObject::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}

MaterialPtr ParticleRenderObject::GetMaterial() const {
	return mImpl->GetMaterial();
}

void ParticleRenderObject::Clear() {
	mImpl->Clear();
}

void ParticleRenderObject::EndUpdate() {
	mImpl->EndUpdate();
}

void ParticleRenderObject::SetDoubleSided(bool set) {
	mImpl->SetDoubleSided(set);
}

void ParticleRenderObject::SetTexture(const char* texturePath) {
	mImpl->SetTexture(texturePath);
}

void ParticleRenderObject::SetScreenspace(int set) {
	mImpl->SetScreenspace(set);
}

ParticleRenderObject::Vertex* ParticleRenderObject::Map(UINT numVertices, unsigned& canWrite) {
	return mImpl->Map(numVertices, canWrite);
}

