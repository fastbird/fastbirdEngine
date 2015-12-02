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

void BillboardQuadFacade::AttachToCurrentScene(){
	auto scene = EngineFacade::GetInstance().GetCurrentScene();
	if (scene)
		scene->AttachObjectFB(mImpl->mBillboardQuad);
}

void BillboardQuadFacade::DetachFromScenes(){
	mImpl->mBillboardQuad->DetachFromScene();
}

void BillboardQuadFacade::SetAlwaysPassCullingTest(bool passAlways){
	mImpl->mBillboardQuad->GetBoundingVolumeWorld()->SetAlwaysPass(passAlways);
}

void BillboardQuadFacade::SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color){
	mImpl->mBillboardQuad->SetBillobardData(pos, size, offset, color);
}

void BillboardQuadFacade::SetMaterial(MaterialPtr mat){
	mImpl->mBillboardQuad->SetMaterial(mat);
}

MaterialPtr BillboardQuadFacade::GetMaterial() const{
	return mImpl->mBillboardQuad->GetMaterial();
}