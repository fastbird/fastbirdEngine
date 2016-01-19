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
#include "BillboardQuadFacade.h"
#include "EngineFacade.h"
#include "FBSceneObjectFactory/BillboardQuad.h"
#include "FBSceneManager/Scene.h"
using namespace fb;
class BillboardQuadFacade::Impl{
public:
	BillboardQuadPtr mBillboardQuad;

	void SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color){
		if (!mBillboardQuad){
			CreateBillboardQuad();
		}
		mBillboardQuad->SetBillobardData(pos, size, offset, color);
	}

	void CreateBillboardQuad(){
		mBillboardQuad = BillboardQuad::Create();
	}

	void SetMaterial(MaterialPtr mat){
		if (!mBillboardQuad){
			CreateBillboardQuad();
		}
		mBillboardQuad->SetMaterial(mat);
	}

	MaterialPtr GetMaterial() {
		if (mBillboardQuad)
			return mBillboardQuad->GetMaterial();
		return 0;
	}

	void AttachToScene(IScene* scene){
		if (scene && mBillboardQuad)
			scene->AttachObjectFB(mBillboardQuad);
	}

	void DetachFromScene(IScene* scene){
		mBillboardQuad->DetachFromScene(scene);		
	}
};

//---------------------------------------------------------------------------
std::vector<BillboardQuadFacadeWeakPtr> sBillboards;
BillboardQuadFacadePtr BillboardQuadFacade::Create(){
	BillboardQuadFacadePtr p(new BillboardQuadFacade, [](BillboardQuadFacade* obj){delete obj; });	
	sBillboards.push_back(p);
	return p;
}

BillboardQuadFacade::BillboardQuadFacade()
	: mImpl(new Impl)
{

}

BillboardQuadFacade::~BillboardQuadFacade(){
	for (auto it = sBillboards.begin(); it != sBillboards.end(); /**/){
		IteratingWeakContainer(sBillboards, it, billboard);
	}
}

void BillboardQuadFacade::AttachToMainScene(){
	auto scene = EngineFacade::GetInstance().GetMainScene();
	if (scene){
		scene->AttachObjectFB(mImpl->mBillboardQuad);
		mImpl->mBillboardQuad->mDebug = true;
	}
}

void BillboardQuadFacade::DetachFromScenes(){
	mImpl->mBillboardQuad->DetachFromScene();
	mImpl->mBillboardQuad->mDebug = false;
}

void BillboardQuadFacade::SetAlwaysPassCullingTest(bool passAlways){
	mImpl->mBillboardQuad->GetBoundingVolumeWorld()->SetAlwaysPass(passAlways);
}

void BillboardQuadFacade::SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color){
	mImpl->SetBillobardData(pos, size, offset, color);	
}

void BillboardQuadFacade::SetMaterial(MaterialPtr mat){
	mImpl->SetMaterial(mat);	
}

MaterialPtr BillboardQuadFacade::GetMaterial() const{
	return mImpl->GetMaterial();
}

void BillboardQuadFacade::AttachToScene(IScene* scene){
	mImpl->AttachToScene(scene);
}
void BillboardQuadFacade::DetachFromScene(IScene* scene){
	mImpl->DetachFromScene(scene);
}