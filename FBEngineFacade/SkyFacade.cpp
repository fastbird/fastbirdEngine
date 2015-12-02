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
#include "SkyFacade.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"
#include "FBSceneObjectFactory/SkySphere.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBSceneManager/Scene.h"
using namespace fb;
class SkyFacade::Impl{
public:
	SkySpherePtr mSkySphere;

	Impl(){
		mSkySphere = SceneObjectFactory::GetInstance().CreateSkySphere();
	}


};

//---------------------------------------------------------------------------
std::vector<SkyFacadeWeakPtr> sSkySpheres;
SkyFacadePtr SkyFacade::Create(){
	SkyFacadePtr p(new SkyFacade, [](SkyFacade* obj){ delete obj; });
	sSkySpheres.push_back(p);
	return p;
}

SkyFacadePtr SkyFacade::GetMain(){
	auto scene = EngineFacade::GetInstance().GetMainScene();
	if (!scene)
		return 0;
	auto mainSky = scene->GetSkySphere();
	if (!mainSky)
		return 0;
	for (auto it = sSkySpheres.begin(); it != sSkySpheres.end(); /**/){
		IteratingWeakContainer(sSkySpheres, it, sky);
		if (sky->mImpl->mSkySphere == mainSky)
			return sky;
	}
	return 0;
}

SkyFacade::SkyFacade()
	: mImpl(new Impl)
{

}

SkyFacade::~SkyFacade(){

}

void SkyFacade::SetMaterial(const char* path, RENDER_PASS pass){
	mImpl->mSkySphere->SetMaterial(path, pass);
}

void SkyFacade::AttachToScene(){
	EngineFacade::GetInstance().GetMainScene()->AttachSkySphere(mImpl->mSkySphere);
}

void SkyFacade::AttachToScene(IScenePtr scene){
	if (scene)
		scene->AttachSkySphere(mImpl->mSkySphere);
}

void SkyFacade::DetachFromScene(){
	auto scenes = mImpl->mSkySphere->GetScenes();
	for (auto scene : scenes){
		scene->DetachSkySphere();
	}
}

MaterialPtr SkyFacade::GetMaterial() const{
	return mImpl->mSkySphere->GetMaterial(0);
}

void SkyFacade::UpdateEnvironmentMap(const Vec3& pos){
	mImpl->mSkySphere->UpdateEnvironmentMap(pos);
}

void SkyFacade::AttachToBlend(){
	EngineFacade::GetInstance().GetMainScene()->GetSkySphere()->AttachBlendingSky(mImpl->mSkySphere);
}

void SkyFacade::SetAlpha(float alpha){
	mImpl->mSkySphere->SetAlpha(alpha);
}

void SkyFacade::PrepareInterpolation(float time, SkyFacadePtr startFrom){
	mImpl->mSkySphere->PrepareInterpolation(time, startFrom->mImpl->mSkySphere);
}

void SkyFacade::AttachBlendingSky(SkyFacadePtr blending){
	mImpl->mSkySphere->AttachBlendingSky(blending->mImpl->mSkySphere);
}

void SkyFacade::SetInterpolationData(unsigned index, const Vec4& data){
	mImpl->mSkySphere->SetInterpolationData(index, data);
}