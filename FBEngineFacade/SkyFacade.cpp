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
#include "FBSceneObjectFactory/SkyBox.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBSceneManager/Scene.h"
#include "ISkyFacadeListener.h"
using namespace fb;
class SkyFacade::Impl{
public:
	SkyFacadeWeakPtr mSelfPtr;
	SkySpherePtr mSkySphere;
	SkyBoxPtr mSkyBox;

	Impl(){
		
	}

	SkyFacadePtr CreateSkySphere(){
		mSkySphere = SceneObjectFactory::GetInstance().CreateSkySphere();
		mSkyBox = 0;
		return mSelfPtr.lock();
	}

	SkyFacadePtr CreateSkyBox(const char* materialPath){
		mSkyBox = SceneObjectFactory::GetInstance().CreateSkyBox(materialPath);
		mSkySphere = 0;
		return mSelfPtr.lock();
	}

	void AttachToScene(){
		auto mainScene = EngineFacade::GetInstance().GetMainScene();
		if (mainScene){
			if (mSkySphere)
				mainScene->AttachSky(mSkySphere);
			else if (mSkyBox)
				mainScene->AttachSky(mSkyBox);
		}
	}

	void AttachToScene(IScenePtr scene){
		if (scene){
			if (mSkySphere)
				scene->AttachSky(mSkySphere);
			else if (mSkyBox)
				scene->AttachSky(mSkyBox);
		}
	}

	void DetachFromScene(){
		if (mSkySphere){
			auto scenes = mSkySphere->GetScenes();
			for (auto scene : scenes){
				scene->DetachSky();
			}
		}
		else if (mSkyBox) {
			auto scenes = mSkyBox->GetScenes();
			for (auto scene : scenes){
				scene->DetachSky();
			}
		}
	}

	MaterialPtr GetMaterial() const{
		if (mSkySphere)
			return mSkySphere->GetMaterial(0);
		else if (mSkyBox)
			return mSkyBox->GetMaterial();

		return 0;
	}

	void AttachToBlend(){
		auto sky = EngineFacade::GetInstance().GetMainScene()->GetSky();
		auto skySpherer = std::dynamic_pointer_cast<SkySphere>(sky);
		if (skySpherer && mSkySphere)
			skySpherer->AttachBlendingSky(mSkySphere);
	}

	void SetAlpha(float alpha){
		if (mSkySphere)
			mSkySphere->SetAlpha(alpha);
	}

	void PrepareInterpolation(float time, SkyFacadePtr startFrom){
		if (mSkySphere && startFrom->mImpl->mSkySphere)
			mSkySphere->PrepareInterpolation(time, startFrom->mImpl->mSkySphere);
	}

	void AttachBlendingSky(SkyFacadePtr blending){
		if (mSkySphere && blending->mImpl->mSkySphere)
			mSkySphere->AttachBlendingSky(blending->mImpl->mSkySphere);
	}

	void SetInterpolationData(unsigned index, const Vec4& data){
		if (mSkySphere)
			mSkySphere->SetInterpolationData(index, data);
	}

	void StartInterpolation(float time){
		if (mSkySphere)
			mSkySphere->StartInterpolation(time);
	}
};

//---------------------------------------------------------------------------
std::vector<SkyFacadeWeakPtr> sSkySpheres;
SkyFacadePtr SkyFacade::Create(){
	SkyFacadePtr p(new SkyFacade, [](SkyFacade* obj){ delete obj; });
	sSkySpheres.push_back(p);
	p->mImpl->mSelfPtr = p;
	return p;
}

SkyFacadePtr SkyFacade::GetMain(){
	auto scene = EngineFacade::GetInstance().GetMainScene();
	if (!scene)
		return 0;
	auto mainSky = scene->GetSky();
	if (!mainSky)
		return 0;
	for (auto it = sSkySpheres.begin(); it != sSkySpheres.end(); /**/){
		IteratingWeakContainer(sSkySpheres, it, sky);
		if (sky->mImpl->mSkySphere == mainSky)
			return sky;
		else if (sky->mImpl->mSkyBox == mainSky)
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

SkyFacadePtr SkyFacade::CreateSkySphere(){
	return mImpl->CreateSkySphere();	
}

SkyFacadePtr SkyFacade::CreateSkyBox(const char* materialPath){
	return mImpl->CreateSkyBox(materialPath);
}

void SkyFacade::SetMaterial(const char* path, RENDER_PASS pass){
	if (mImpl->mSkySphere)
		mImpl->mSkySphere->SetMaterial(path, pass);
}

void SkyFacade::SetGeometry(const char* path) {
	if (mImpl->mSkySphere)
		mImpl->mSkySphere->SetGeometry(path);
}

void SkyFacade::AttachToScene(){
	mImpl->AttachToScene();
}

void SkyFacade::AttachToScene(IScenePtr scene){
	mImpl->AttachToScene(scene);	
}

void SkyFacade::DetachFromScene(){
	mImpl->DetachFromScene();	
}

MaterialPtr SkyFacade::GetMaterial() const{
	return mImpl->GetMaterial();
	return mImpl->mSkySphere->GetMaterial(0);
}

void SkyFacade::UpdateEnvironmentMap(const Vec3& pos){
	mImpl->mSkySphere->UpdateEnvironmentMap(pos);
}

void SkyFacade::AttachToBlend(){
	mImpl->AttachToBlend();
}

void SkyFacade::SetAlpha(float alpha){
	mImpl->SetAlpha(alpha);	
}

void SkyFacade::PrepareInterpolation(float time, SkyFacadePtr startFrom){
	mImpl->PrepareInterpolation(time, startFrom);
}

void SkyFacade::AttachBlendingSky(SkyFacadePtr blending){
	mImpl->AttachBlendingSky(blending);
}

void SkyFacade::SetInterpolationData(unsigned index, const Vec4& data){
	mImpl->SetInterpolationData(index, data);
}

void SkyFacade::StartInterpolation(float time){
	mImpl->StartInterpolation(time);
}


void SkyFacade::AddListener(ISkyFacadeListener* listener) {
	if (mImpl->mSkySphere) {
		mImpl->mSkySphere->AddListener(this);
		__super::AddListener(listener);
	}
	else {
		Logger::Log(FB_ERROR_LOG_ARG, "No skysphere ready!");
	}	
}

void SkyFacade::RemoveListener(ISkyFacadeListener* listener) {
	if (mImpl->mSkySphere) {
		mImpl->mSkySphere->RemoveListener(this);
		__super::RemoveListener(listener);
	}
	else {
		Logger::Log(FB_ERROR_LOG_ARG, "No skysphere ready!");
	}	
}

void SkyFacade::OnInterpolationFinished(SkySphere* sky) {
	if (mImpl->mSkySphere.get()!= sky) {
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid listener link.");
		return;
	}
	for (auto l : mListeners) {
		l->OnInterpolationFinished(mImpl->mSelfPtr.lock());
	}
}