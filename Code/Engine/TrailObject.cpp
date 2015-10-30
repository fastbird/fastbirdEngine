#include <Engine/StdAfx.h>
#include <Engine/TrailObject.h>
#include <Engine/Renderer.h>
#include <Engine/ICamera.h>

using namespace fastbird;

TrailObject::TrailObject()
	: mMaxPoints(100)
	, mDirty(false)
	, mWidth(0.025f)
	, mDeleteTime(2.f)
	, mLastPoint(0, 0, 0)
{
	SetMaterial("es/materials/Trail.material");
}


void TrailObject::PreRender(){
	if (mDirty){
		mDirty = false;
		RefreshVertexBuffer();
	}
}
void TrailObject::Render(){
	if (mObjFlag & IObject::OF_HIDE || !mVB)
		return;
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL || mPoints.empty())
		return;
	D3DEventMarker mark("TrailObject");
	auto const renderer = gFBEnv->_pInternalRenderer;
	renderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
	mMaterial->Bind(true);
	mVB->Bind();

	renderer->Draw(mPoints.size(), 0);
	mMaterial->Unbind();
}

void TrailObject::PostRender(){

}

void TrailObject::SetMaterial(const char* name, int pass){
	mMaterial = IMaterial::CreateMaterial(name);
}
void TrailObject::SetMaterial(IMaterial* pMat, int pass){
	mMaterial = pMat;
}
IMaterial* TrailObject::GetMaterial(int pass) const{
	return mMaterial;
}

//for billboard trail - automatically face to the camera
void TrailObject::AddPoint(const Vec3& worldPos){
	mLastPoint = worldPos;
	if (mMaxPoints == 0)
		return;

	if (GetDistToCam() > 100.0f)
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
	if (mPoints.size()>=3){ 
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

void TrailObject::SetWidth(float width){
	mWidth = width;
	if (mMaterial){
		mMaterial->SetMaterialParameters(0, Vec4(mWidth, 0, 0, 0));
	}
}

// for manual trail
void TrailObject::AddPoint(const Vec3& worldPosA, const Vec3& worldPosB){
	mLastPoint = worldPosB;
	if (GetDistToCam() > 100.0f)
		return;
	mPairedPoints.push(std::make_pair(worldPosA, worldPosB));
	mDirty = true;
}

void TrailObject::SetMaxPoints(unsigned num){
	mMaxPoints = num;
	mVB = 0;
}

void TrailObject::Clear(){
	mPoints.clear();
	mTimes.clear();
	ClearWithSwap(mPairedPoints);
	
	mDirty = true;
}

void TrailObject::RefreshVertexBuffer(){
	unsigned size = mPoints.size();
	if (size < 3 && mPairedPoints.size() < 3)
		return;
	if (!mVB){
		mVB = gFBEnv->pRenderer->CreateVertexBuffer(0, sizeof(TrailVertex), mMaxPoints+1, BUFFER_USAGE::BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		assert(mVB);
	}	

	size = mPoints.size();
	auto mapData = mVB->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
	if (mapData.pData){		
		for (unsigned i = 0; i < size; ++i){
			float alpha = (size-i) / (float)size;			
			mPoints[i].mPos.w = alpha;
		}
			
		memcpy(mapData.pData, &mPoints[0], sizeof(TrailVertex)*size);
		mVB->Unmap();
	}
}

void TrailObject::Update(float dt){
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

float TrailObject::GetDistToCam() const{
	auto cam =  gFBEnv->pRenderer->GetMainCamera();
	return cam->GetPos().DistanceTo(mLastPoint);
}