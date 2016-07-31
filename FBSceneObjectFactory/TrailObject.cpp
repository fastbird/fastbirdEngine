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
#include "TrailObject.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/VertexBuffer.h"
#include "FBRenderer/Camera.h"
using namespace fb;
class TrailObject::Impl
{
public:
	struct TrailVertex{
		TrailVertex(const Vec4f& pos)
			: mPos(pos)
		{
		}
		Vec4f mPos;
	};

	typedef std::vector<TrailVertex> Points;
	typedef std::vector<float> Times;

	TrailObject* mSelf;
	Points mPoints;
	Times mTimes;
	std::queue<std::pair<Vec3, Vec3>> mPairedPoints;

	bool mDirty;
	float mWidth;
	unsigned mMaxPoints;
	float mDeleteTime;

	MaterialPtr mMaterial;
	VertexBufferPtr mVB;
	Vec3 mLastPoint;

	//---------------------------------------------------------------------------
	Impl(TrailObject* self)
		: mSelf(self)
		, mMaxPoints(100)
		, mDirty(false)
		, mWidth(0.025f)
		, mDeleteTime(2.f)
		, mLastPoint(0, 0, 0)
	{
		mMaterial = Renderer::GetInstance().CreateMaterial("EssentialEngineData/materials/Trail.material");
	}

	// IRenderable
	void PreRender(const RenderParam& param, RenderParamOut* paramOut){
		if (mDirty){
			mDirty = false;
			RefreshVertexBuffer();
		}
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide) || !mVB)
			return;
		if (param.mRenderPass != RENDER_PASS::PASS_NORMAL || mPoints.empty())
			return;
		RenderEventMarker mark("Trail Object");
		auto& renderer = Renderer::GetInstance();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
		mMaterial->Bind(true);
		mVB->Bind();
		renderer.Draw(mPoints.size(), 0);
		mMaterial->Unbind();
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}

	//------------------------------------------------------------------------
	// Own
	//------------------------------------------------------------------------
	void SetMaterial(const char* filepath){
		mMaterial = Renderer::GetInstance().CreateMaterial(filepath);
	}

	void SetMaterial(MaterialPtr pMat){
		mMaterial = pMat;
	}

	MaterialPtr GetMaterial() const{
		return mMaterial;
	}
	//for billboard trail - automatically face to the camera
	void AddPoint(const Vec3& worldPos){
		mLastPoint = worldPos;
		if (mMaxPoints == 0)
			return;

		if (GetDistToCam() > 300.0f)
			return;

		if (!mPoints.empty()){
			if (mPoints.size() >= 2)
			{
				if (IsEqual(mPoints[1].mPos.ToVec3(), worldPos, 0.001f))
					return;
			}
			else if (IsEqual(mPoints[0].mPos.ToVec3(), worldPos, 0.001f))
				return;


		}
		if (mPoints.size() >= 3){
			mPoints[0] = TrailVertex(Vec4(worldPos, 1.f));
			mTimes[0] = gpTimer->GetTime();
			Vec3 dir = mPoints[0].mPos.ToVec3() - mPoints[1].mPos.ToVec3();
			mPoints.insert(mPoints.begin(), TrailVertex(Vec4(mPoints[0].mPos.ToVec3() + dir, 1.0)));
			mTimes.insert(mTimes.begin(), gpTimer->GetTime());
		}
		else{
			mPoints.insert(mPoints.begin(), TrailVertex(Vec4(worldPos, 1.f)));
			mTimes.insert(mTimes.begin(), gpTimer->GetTime());
		}
		mDirty = true;
		while (mPoints.size() > mMaxPoints){
			mPoints.pop_back();
			mTimes.pop_back();
		}
	}

	void SetWidth(float width){
		mWidth = width;
		if (mMaterial){
			mMaterial->SetShaderParameter(0, Vec4(mWidth, 0, 0, 0));
		}
	}

	// for manual trail
	void AddPoint(const Vec3& worldPosA, const Vec3& worldPosB){
		mLastPoint = worldPosB;
		if (GetDistToCam() > 100.0f)
			return;
		mPairedPoints.push(std::make_pair(worldPosA, worldPosB));
		mDirty = true;
	}

	void SetMaxPoints(unsigned num){
		mMaxPoints = num;
		mVB = 0;
	}

	void Clear(){
		mPoints.clear();
		mTimes.clear();
		ClearWithSwap(mPairedPoints);

		mDirty = true;
	}

	void RefreshVertexBuffer(){
		unsigned size = mPoints.size();
		if (size < 3 && mPairedPoints.size() < 3)
			return;
		if (!mVB){
			mVB = Renderer::GetInstance().CreateVertexBuffer(0, sizeof(TrailVertex), mMaxPoints + 1, BUFFER_USAGE::BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
			assert(mVB);
		}

		size = mPoints.size();
		auto mapData = mVB->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (mapData.pData){
			for (unsigned i = 0; i < size; ++i){
				float alpha = (size - i) / (float)size;
				mPoints[i].mPos.w = alpha;
			}

			memcpy(mapData.pData, &mPoints[0], sizeof(TrailVertex)*size);
			mVB->Unmap(0);
		}
	}

	void Update(float dt){
		if (mPoints.empty())
			return;

		int size = (int)mTimes.size();
		auto curTime = gpTimer->GetTime();
		for (int i = size - 1; i >= 0; --i){
			if (curTime - mTimes[i] >= mDeleteTime){
				mTimes.erase(mTimes.begin() + i);
				mPoints.erase(mPoints.begin() + i);
				mDirty = true;
			}
			else{
				break;
			}
		}
	}

	float GetDistToCam() const{
		auto cam = Renderer::GetInstance().GetMainCamera();
		return cam->GetPosition().DistanceTo(mLastPoint);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(TrailObject)

TrailObject::TrailObject()
: mImpl(new Impl(this))
{
}

TrailObject::~TrailObject(){

}

void TrailObject::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void TrailObject::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void TrailObject::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}


void TrailObject::SetMaterial(const char* filepath) {
	mImpl->SetMaterial(filepath);
}

void TrailObject::SetMaterial(MaterialPtr pMat) {
	mImpl->SetMaterial(pMat);
}

MaterialPtr TrailObject::GetMaterial() const {
	return mImpl->GetMaterial();
}

void TrailObject::AddPoint(const Vec3& worldPos) {
	mImpl->AddPoint(worldPos);
}

void TrailObject::SetWidth(float width) {
	mImpl->SetWidth(width);
}

void TrailObject::AddPoint(const Vec3& worldPosA, const Vec3& worldPosB) {
	mImpl->AddPoint(worldPosA, worldPosB);
}

void TrailObject::SetMaxPoints(unsigned num) {
	mImpl->SetMaxPoints(num);
}

void TrailObject::Clear() {
	mImpl->Clear();
}

void TrailObject::Update(float dt) {
	mImpl->Update(dt);
}

float TrailObject::GetDistToCam() const {
	return mImpl->GetDistToCam();
}

