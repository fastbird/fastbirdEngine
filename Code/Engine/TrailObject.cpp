#include <Engine/StdAfx.h>
#include <Engine/TrailObject.h>
#include <Engine/Renderer.h>

using namespace fastbird;

TrailObject::TrailObject()
	: mMaxPoints(100)
	, mDirty(false)
	, mWidth(0.05f)
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
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
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
	if (!mPoints.empty()){
		if (mPoints.back().mPos == worldPos)
			return;
	}
	mPoints.push_back( TrailVertex(Vec4(worldPos, 1.f), Color(1, 1, 1, 1)) );
	mDirty = true;
	while (mPoints.size() >= mMaxPoints){
		mPoints.erase(mPoints.begin());
	}
}

void TrailObject::SetWidth(float width){
	mWidth = width;
}

// for manual trail
void TrailObject::AddPoint(const Vec3& worldPosA, const Vec3& worldPosB){
	mPairedPoints.push(std::make_pair(worldPosA, worldPosB));
	mDirty = true;
}

void TrailObject::SetMaxPoints(unsigned num){
	mMaxPoints = num;
	mVB = 0;
}

void TrailObject::Clear(){
	ClearWithSwap(mPoints);
	ClearWithSwap(mPairedPoints);
	
	mDirty = true;
}

void TrailObject::RefreshVertexBuffer(){
	if (mPoints.size() < 4 && mPairedPoints.size() < 4)
		return;
	if (!mVB){
		mVB = gFBEnv->pRenderer->CreateVertexBuffer(0, sizeof(TrailVertex), mMaxPoints, BUFFER_USAGE::BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		assert(mVB);
	}
	auto mapData = mVB->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
	if (mapData.pData){
		if (!mPoints.empty()){
			memcpy(mapData.pData, &mPoints[0], sizeof(TrailVertex)*mPoints.size());
		}
		mVB->Unmap();
	}


}