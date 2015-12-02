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
#include "DustFacade.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"
#include "FBSceneObjectFactory/DustRenderer.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBSceneManager/Scene.h"
using namespace fb;
class DustFacade::Impl{
public:
	DustRendererPtr mDustRenderer;

	//---------------------------------------------------------------------------
	Impl()
		:mDustRenderer(SceneObjectFactory::GetInstance().CreateDustRenderer())
	{
		
	}


};

std::vector<DustFacadeWeakPtr> sDustFacades;
DustFacadePtr DustFacade::Create(){
	DustFacadePtr p(new DustFacade, [](DustFacade* obj){delete obj; });
	sDustFacades.push_back(p);
	return p;
}

DustFacade::DustFacade()
	: mImpl(new Impl)
{

}
DustFacade::~DustFacade(){
	for (auto it = sDustFacades.begin(); it != sDustFacades.end(); /**/){
		IteratingWeakContainer(sDustFacades, it, dust);
	}
}

void DustFacade::SetMaterial(const char* path){
	mImpl->mDustRenderer->SetMaterial(path, PASS_NORMAL);
}

void DustFacade::InitDustRenderer(const Vec3& min, const Vec3& max, size_t count,
	const Color& cmin, const Color& cmax, float normalizeDist){
	mImpl->mDustRenderer->Initialize(min, max, count, cmin, cmax, normalizeDist);
}

void DustFacade::AttachToScene(){
	auto scene = EngineFacade::GetInstance().GetMainScene();
	if (scene)
		scene->AttachObjectFB(mImpl->mDustRenderer);
}

void DustFacade::DetachFromScene(){
	mImpl->mDustRenderer->DetachFromScene();
}

const Vec3& DustFacade::GetMin() const{
	return mImpl->mDustRenderer->GetMin();
}