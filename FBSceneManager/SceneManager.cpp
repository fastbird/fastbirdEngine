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
#include "SceneManager.h"
#include "Scene.h"
#include "DirectionalLight.h"
#include "FBTimer/Timer.h"
using namespace fb;

//---------------------------------------------------------------------------
class SceneManager::Impl{
public:
	SceneManagerWeakPtr mSelf;
	std::map<std::string, SceneWeakPtr> mScenes;
	SceneWeakPtr mMainScene;

	//---------------------------------------------------------------------------
	Impl(){
		
	}
	~Impl(){

	}
	ScenePtr CreateScene(const char* name){
		if (!ValidCString(name)){
			Logger::Log(FB_ERROR_LOG_ARG, "invalid arg");
			return 0;
		}

		auto it = mScenes.find(name);
		if (it != mScenes.end()){
			auto scene = it->second.lock();
			if (scene)
				return scene;
		}
		auto scene = Scene::Create(name);
		mScenes[name] = scene;
		if (mMainScene.expired())
			mMainScene = scene;
		return scene;
	}

	ScenePtr GetMainScene() const{
		return mMainScene.lock();
	}

	void Update(TIME_PRECISION dt){
		for (auto it = mScenes.begin(); it != mScenes.end(); ){
			auto scene = it->second.lock();
			if (!scene){
				auto curIt = it;
				++it;
				mScenes.erase(curIt);
				continue;
			}			
			scene->Update(dt);
			++it;
		}
	}

	void CopyDirectionalLight(IScenePtr destScene, int destLightSlot, IScenePtr srcScene, int srcLightSlot){
		auto d = std::dynamic_pointer_cast<Scene>(destScene);
		auto s = std::dynamic_pointer_cast<Scene>(srcScene);
		if (d && s){
			auto dLight = d->GetDirectionalLight(destLightSlot);
			auto sLight = s->GetDirectionalLight(srcLightSlot);
			if (dLight && sLight)
				dLight->CopyLight(sLight);
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid Arg.");
		}
	}
};

Timer* fb::gpTimer = 0;
//---------------------------------------------------------------------------
SceneManagerWeakPtr sSceneManager;
SceneManagerPtr SceneManager::Create(){
	if (sSceneManager.expired()){
		auto sceneManager = SceneManagerPtr(new SceneManager, [](SceneManager* obj){ delete obj; });
		sceneManager->mImpl->mSelf = sceneManager;
		sSceneManager = sceneManager;
		gpTimer = Timer::GetMainTimer().get();
		return sceneManager;
	}
	return sSceneManager.lock();
}

SceneManager& SceneManager::GetInstance(){
	return *sSceneManager.lock();
}

SceneManager::SceneManager()
	: mImpl(new Impl){
}

SceneManager::~SceneManager(){

}

ScenePtr SceneManager::CreateScene(const char* name){
	return 	mImpl->CreateScene(name);
}

ScenePtr SceneManager::GetMainScene() const{
	return mImpl->GetMainScene();
}

void SceneManager::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}

void SceneManager::CopyDirectionalLight(IScenePtr destScene, int destLightSlot, IScenePtr srcScene, int srcLightSlot){
	mImpl->CopyDirectionalLight(destScene, destLightSlot, srcScene, srcLightSlot);
}