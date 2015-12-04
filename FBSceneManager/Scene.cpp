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
#include "Scene.h"
#include "DirectionalLight.h"
#include "SpatialSceneObject.h"
#include "PointLightManager.h"
#include "FBRenderer/ICamera.h"
#include "FBRenderer/RenderPass.h"
#include "FBMathLib/Color.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBTimer/Timer.h"
#include "FBStringLib/StringLib.h"
#include "FBSceneObjectFactory/SkySphere.h"// this doesn't make denpendency
using namespace fb;
#undef AttachObjectFB
#undef DetachObjectFB

class Scene::Impl{
public:
	Scene* mSelf;
	SceneWeakPtr mSelfPtr;
	std::string mName;
	RENDER_PASS mRenderPass; // processing render pass
	typedef std::vector<SpatialSceneObjectPtr> SPATIAL_OBJECTS;
	typedef std::vector<SpatialSceneObjectWeakPtr> SPATIAL_OBJECTS_WEAK;
	typedef std::vector<SpatialSceneObject*> SPATIAL_OBJECTS_RAW;
	typedef std::vector<SceneObjectPtr> OBJECTS;
	typedef std::vector<SceneObjectWeakPtr> OBJECTS_WEAK;
	typedef std::vector<SceneObject*> OBJECTS_RAW;
	OBJECTS_WEAK mObjects;
	SPATIAL_OBJECTS_WEAK mSpatialObjects;
	VectorMap<ICamera*, SPATIAL_OBJECTS_RAW> mVisibleObjectsMain;
	VectorMap<ICamera*, SPATIAL_OBJECTS_RAW> mVisibleObjectsLight;
	VectorMap<ICamera*, SPATIAL_OBJECTS_RAW> mPreRenderList;
	VectorMap<ICamera*, SPATIAL_OBJECTS_RAW> mVisibleTransparentObjects;
	
	/*std::vector<SceneObjectWeakPtr> mMarkObjects;
	std::vector<SceneObjectWeakPtr> mHPBarObjects;*/

	SceneObjectPtr mSky;
	DirectionalLightPtr mDirectionalLight[2];
	bool mSkipSpatialObjects;
	bool mSkyRendering;
	std::mutex mSpatialObjectsMutex;
	std::vector< SpatialSceneObjectPtr > mCloudVolumes;
	Vec3 mWindDir;
	float mWindVelocity;
	Vec3 mWindVector;
	Color mFogColor;
	VectorMap<ICamera*, unsigned> mLastPreRenderFramePerCam;
	bool mDrawClouds;
	bool mRttScene;
	bool mRefreshPointLight;
	PointLightManagerPtr mPointLightMan;

	Impl(Scene* self, const char* name)
		: mSelf(self)
		, mName(name)
		, mRenderPass(PASS_NORMAL)
		, mSkyRendering(true)
		, mSkipSpatialObjects(false)
		, mDrawClouds(true)
		, mRttScene(false)
		, mFogColor(0, 0, 0)
		, mWindDir(1, 0, 0)
		, mWindVelocity(0.f)
		, mRefreshPointLight(false)
		, mPointLightMan(PointLightManager::Create())
	{
		mWindVector = mWindDir * mWindVelocity;

		// Light
		for (int i = 0; i < 2; ++i)
		{
			mDirectionalLight[i] = DirectionalLight::Create();
			mDirectionalLight[i]->SetIntensity(1.0f);
		}

		mDirectionalLight[0]->SetDirection(Vec3(-3, 1, 1));
		mDirectionalLight[0]->SetDiffuse(Vec3(1, 1, 1));
		mDirectionalLight[0]->SetSpecular(Vec3(1, 1, 1));

		mDirectionalLight[1]->SetDirection(Vec3(3, 1, -1));
		mDirectionalLight[1]->SetDiffuse(Vec3(0.8f, 0.4f, 0.1f));
		mDirectionalLight[1]->SetSpecular(Vec3(0, 0, 0));
	}
	void Init(){
		mPointLightMan->SetScene(mSelfPtr.lock());

	}
	const char* GetName() const{
		return mName.c_str();
	}

	void Update(TIME_PRECISION dt){
		for (int i = 0; i < 2; i++)
			mDirectionalLight[i]->Update(dt);

		mPointLightMan->Update(dt);
		// good point to reset.
		mRefreshPointLight = false;

		// tick to spatial objects
		for (auto it = mSpatialObjects.begin(); it != mSpatialObjects.end(); /**/){
			IteratingWeakContainer(mSpatialObjects, it, spatialObj);
			spatialObj->Update(dt);
		}
	}

	void AddSceneObserver(int ISceneObserverEnum, ISceneObserverPtr observer){
		mSelf->AddObserver(ISceneObserverEnum, observer);
	}

	const Vec3& GetMainLightDirection(){
		return mDirectionalLight[0]->GetDirection();
	}

	void SetLightDirection(DirectionalLightIndex::Enum idx, const Vec3& dir){
		mDirectionalLight[idx]->SetDirection(dir);
	}

	void GetDirectionalLightInfo(DirectionalLightIndex::Enum idx, DirectionalLightInfo& data){
		auto& light = mDirectionalLight[idx];
		data.mDirection_Intensiy = Vec4(light->GetDirection(), light->GetIntensity());
		data.mDiffuse = Vec4(light->GetDiffuse(), 1.f);
		data.mSpecular = Vec4(light->GetSpecular(), 1.f);
	}

	void MakeVisibleSet(const RenderParam& renderParam, RenderParamOut* renderParamOut)
	{
		if (mSkipSpatialObjects)
			return;

		auto mainCam = renderParam.mCamera;
		auto lightCam = renderParam.mLightCamera;
		mVisibleObjectsMain[mainCam].clear();
		mVisibleObjectsLight[lightCam].clear();
		mVisibleTransparentObjects[mainCam].clear();
		mPreRenderList[mainCam].clear();

		if (!mainCam)
		{
			Logger::Log(FB_FRAME_TIME, FB_ERROR_LOG_ARG, "Invalid main camera");
			return;
		}
		if (!lightCam){
			Logger::Log(FB_FRAME_TIME, FB_ERROR_LOG_ARG, "Invalid light camera");
			return;
		}

		{
			MutexLock lock(mSpatialObjectsMutex);			
			for (auto it = mSpatialObjects.begin(); it != mSpatialObjects.end(); /**/)
			{
				auto obj = it->lock();
				if (!obj){
					it = mSpatialObjects.erase(it);
					continue;
				}

				if (obj->HasObjFlag(SceneObjectFlag::Ignore)){
					++it;
					continue;
				}
				++it;

				bool inserted = false;
				if (!mainCam->IsCulled(obj->GetBoundingVolumeWorld().get())){				
					if (obj->HasObjFlag(SceneObjectFlag::Transparent))
					{
						mVisibleTransparentObjects[mainCam].push_back(obj.get());
					}
					else
					{
						mVisibleObjectsMain[mainCam].push_back(obj.get());
					}
					inserted = true;
				}

				if (lightCam && !lightCam->IsCulled(obj->GetBoundingVolumeWorld().get()))
				{
					mVisibleObjectsLight[lightCam].push_back((obj.get()));
					inserted = true;
				}
				if (inserted)
					mPreRenderList[mainCam].push_back((obj.get()));
			}
		}

		const fb::Vec3& camPos = mainCam->GetPosition();
		for (const auto obj : mPreRenderList[mainCam])
		{
			assert(obj);
			const Vec3& objPos = obj->GetPosition();
			float dist = (camPos - objPos).Length();
			obj->SetDistToCam(dist);
		}

		std::sort(mVisibleObjectsMain[mainCam].begin(), mVisibleObjectsMain[mainCam].end(),
			[](SpatialObject* a, SpatialObject* b) -> bool
		{
			return a->GetDistToCam() < b->GetDistToCam();
		}
		);

		std::sort(mVisibleObjectsLight[lightCam].begin(), mVisibleObjectsLight[lightCam].end(),
			[](SpatialObject* a, SpatialObject* b) -> bool
		{
			return a->GetDistToCam() < b->GetDistToCam();
		}
		);

		std::sort(mVisibleTransparentObjects[mainCam].begin(), mVisibleTransparentObjects[mainCam].end(),
			[](SpatialObject* a, SpatialObject* b) -> bool
		{
			return a->GetDistToCam() > b->GetDistToCam();
		}
		);

		auto& observers = mSelf->mObservers_[ISceneObserver::Timing];
		for (auto it = observers.begin(); it != observers.end(); /**/){
			auto observer = it->lock();
			if (!observer){
				it = observers.erase(it);
				continue;
			}
			++it;

			observer->OnAfterMakeVisibleSet(mSelf);
		}		
	}

	void PreRender(const RenderParam& renderParam, RenderParamOut* renderParamOut){
		mRenderPass = (RENDER_PASS)renderParam.mRenderPass;
		renderParam.mScene = mSelf;
		if (!mSkipSpatialObjects)
		{
			auto cam = renderParam.mCamera;
			assert(cam);

			auto it = mLastPreRenderFramePerCam.Find(cam);
			if (it != mLastPreRenderFramePerCam.end() && it->second == gpTimer->GetFrame())
				return;
			mLastPreRenderFramePerCam[cam] = gpTimer->GetFrame();

			MakeVisibleSet(renderParam, renderParamOut);

			auto objIt = mPreRenderList[cam].begin(), objItEnd = mPreRenderList[cam].end();
			for (; objIt != objItEnd; objIt++)
			{
				// Only objects that have a valid renderable is in the render lists.
				(*objIt)->PreRender(renderParam, renderParamOut);				
			}
		}

		if (mSkyRendering)
		{
			if (mSky)
			{
				mSky->PreRender(renderParam, renderParamOut);
			}
		}

		for (auto it = mObjects.begin(); it != mObjects.end(); /**/){
			auto obj = it->lock();
			if (!obj){
				it = mObjects.erase(it);
				continue;
			}
			++it;
			obj->PreRender(renderParam, renderParamOut);			
		}
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		mRenderPass = (RENDER_PASS)param.mRenderPass;
		param.mScene = mSelf;
		auto lightCamera = param.mLightCamera;
		auto cam = param.mCamera;
		if (!mSkipSpatialObjects)
		{
			if (param.mRenderPass == PASS_SHADOW)
			{
				for (auto& obj : mVisibleObjectsLight[lightCamera])
				{
					obj->Render(param, paramOut);
				}
			}
			else
			{
				auto& observers = mSelf->mObservers_[ISceneObserver::Timing];
				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->OnBeforeRenderingOpaques(mSelf, param, paramOut);
				}

				for (auto& obj : mVisibleObjectsMain[cam])
				{
					obj->Render(param, paramOut);
				}
			}
		}

		if (!(param.mRenderPass == RENDER_PASS::PASS_SHADOW || param.mRenderPass == RENDER_PASS::PASS_DEPTH))
		{
			if (mSkyRendering)
			{
				if (mSky)
				{
					mSky->Render(param, paramOut);
				}

			}

			for (auto it : mSelf->mObservers_[ISceneObserver::Timing]){
				auto observer = it.lock();
				if (observer){
					observer->OnBeforeRenderingTransparents(mSelf, param, paramOut);
				}
			}

			if (!mSkipSpatialObjects)
			{
				auto it = mVisibleTransparentObjects[cam].begin(), itEnd = mVisibleTransparentObjects[cam].end();
				for (; it != itEnd; it++)
				{
					(*it)->Render(param, paramOut);					
				}
			}
			{
				for (auto it : mObjects){
					auto obj = it.lock();
					if (obj){
						obj->Render(param, paramOut);						
					}
				}
			}
		}
	}

	/// Returns currently rendering pass which is RenderParam.mRenderPass when the Render() is called.
	int GetRenderPass() const{
		return mRenderPass;
	}


	bool AttachObject(SceneObjectPtr pObject){
		if (pObject->GetType() == SceneObjectType::SkySphere){
			Logger::Log(FB_ERROR_LOG_ARG, "You cannot attach sky sphere as an object. Use AttachSkySpherer function instead.");
			return false;
		}
		if (!ValueExistsInVector(mObjects, pObject)){
			mObjects.push_back(pObject);
			pObject->OnAttachedToScene(mSelfPtr.lock());
			return true;
		}

		return false;
	}

	bool DetachObject(SceneObject* pObject){
		bool deleted = false;
		for (auto it = mObjects.begin(); it != mObjects.end(); /**/){
			auto obj = it->lock();
			if (!obj){
				it = mObjects.erase(it);
				continue;
			}
			if (obj.get() == pObject){
				it = mObjects.erase(it);
				deleted = true;
				break;
			}
			else{
				++it;
			}
		}

		if (deleted) {
			pObject->OnDetachedFromScene(mSelfPtr.lock());
		}
		return deleted;
	}


	bool AttachSpatialObject(SpatialSceneObjectPtr pSpatialObject){
		if (!ValueExistsInVector(mSpatialObjects, pSpatialObject)){
			mSpatialObjects.push_back(pSpatialObject);
			pSpatialObject->OnAttachedToScene(mSelfPtr.lock());
			return true;
		}
		return false;
	}

	bool DetachSpatialObject(SpatialSceneObject* pSpatialObject){
		bool deleted = false;
		for (auto it = mSpatialObjects.begin(); it != mSpatialObjects.end(); /**/){
			auto obj = it->lock();
			if (!obj){
				it = mSpatialObjects.erase(it);
				continue;
			}
			if (obj.get() == pSpatialObject){
				it = mSpatialObjects.erase(it);
				deleted = true;
				break;
			}
			else{
				++it;
			}			
		}
		
		if (deleted) {
			pSpatialObject->OnDetachedFromScene(mSelfPtr.lock());
		}
		return deleted;
	}

	void SetSkipSpatialObjects(bool skip){
		mSkipSpatialObjects = skip;
	}

	void ClearEverySpatialObject(){
		mSpatialObjects.clear();
	}

	unsigned GetNumSpatialObjects() const{
		return mSpatialObjects.size();
	}

	const SPATIAL_OBJECTS_RAW* GetVisibleSpatialList(ICameraPtr cam){
		return &mVisibleObjectsMain[cam.get()];
	}

	void PrintSpatialObject(){
		for (auto it = mSpatialObjects.begin(); it != mSpatialObjects.end(); /**/){
			auto obj = it->lock();
			if (!obj){
				it = mSpatialObjects.erase(it);
				continue;
			}
			++it;
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("void Scene::PrintSpatialObject(): num:  %u", mSpatialObjects.size()).c_str());
		for (auto it : mSpatialObjects)
		{
			auto obj = it.lock();
			if (!obj)
				continue;
			auto name = obj->GetName();
			if (name && strlen(name) != 0)
			{
				Logger::Log(FB_DEFAULT_LOG_ARG_NO_LINE, "Spatial Object Name = %s	", name);
			}
			else
			{
				int gameType = obj->GetGameType();
				Logger::Log("GameType = %d\n", gameType);				
			}
		}
	}

	void AttachSky(SceneObjectPtr p){
		if (mSky)
			mSky->OnDetachedFromScene(mSelfPtr.lock());
		mSky = p;
		if (mSky){
			mSky->OnAttachedToScene(mSelfPtr.lock());
		}
	}

	void DetachSky(){
		if (mSky){
			mSky->OnDetachedFromScene(mSelfPtr.lock());
			mSky = 0;
		}
	}

	SceneObjectPtr GetSky(){
		return mSky;
	}

	void ToggleSkyRendering(){
		mSkyRendering = !mSkyRendering;
	}

	void SetSkyRendering(bool render){
		mSkyRendering = render;
	}

	bool GetSkyRendering(){
		return mSkyRendering;
	}

	const Vec3& GetWindVector() const{
		return mWindVector;
	}

	/** p is a MeshObject*/
	void AddCloudVolume(SpatialSceneObjectPtr p){
		mCloudVolumes.push_back(p);
	}

	void ClearClouds(){
		mCloudVolumes.clear();
	}

	void PreRenderCloudVolumes(const RenderParam& param, RenderParamOut* paramOut){
		param.mScene = mSelf;
		for (auto var : mCloudVolumes)
		{
			var->PreRender(param, paramOut);			
		}
	}

	void RenderCloudVolumes(const RenderParam& param, RenderParamOut* paramOut){
		param.mScene = mSelf;
		for (auto var : mCloudVolumes)
		{
			var->Render(param, paramOut);			
		}
	}


	const Color& GetFogColor() const{
		return mFogColor;
	}

	void SetFogColor(const Color& c){
		mFogColor = c;
	}

	void SetDrawClouds(bool e){
		mDrawClouds = e;
	}

	/** True if this scene is attached in the render target which is not related swap-chain.*/
	void SetRttScene(bool set){
		mRttScene = set;
	}

	bool IsRttScene() const{
		return mRttScene;
	}

	DirectionalLightPtr GetDirectionalLight(unsigned idx){
		assert(idx < 2);
		return mDirectionalLight[idx];
	}

	PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime,
		bool manualDeletion){
		assert(mPointLightMan);
		RefreshPointLight();
		return mPointLightMan->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
	}

	PointLightManagerPtr GetPointLightMan() const{
		return mPointLightMan;
	}

	void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst){
		mPointLightMan->GatherPointLightData(aabb, transform, plConst);
	}
	void RefreshPointLight(){
		mRefreshPointLight = true;
	}
	bool NeedToRefreshPointLight() const{
		return mRefreshPointLight;
	}
};

//---------------------------------------------------------------------------
ScenePtr Scene::Create(const char* name){
	if (!ValidCStringLength(name)){
		Logger::Log(FB_ERROR_LOG_ARG, "Scene should have a valid name.");
		return 0;
	}
	auto p =  ScenePtr(new Scene(name), [](Scene* obj){ delete obj; });
	p->mImpl->mSelfPtr = p;
	p->mImpl->Init();
	return p;
}

Scene::Scene(const char* name)
	:mImpl(new Impl(this, name))
{
}

Scene::~Scene(){

}

const char* Scene::GetName() const {
	return mImpl->GetName();
}

void Scene::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}

void Scene::AddSceneObserver(int ISceneObserverEnum, ISceneObserverPtr observer){
	mImpl->AddSceneObserver(ISceneObserverEnum, observer);
}

const Vec3& Scene::GetMainLightDirection(){
	return mImpl->GetMainLightDirection();
}

void Scene::SetLightDirection(DirectionalLightIndex::Enum idx, const Vec3& dir){
	return mImpl->SetLightDirection(idx, dir);
}


void Scene::GetDirectionalLightInfo(DirectionalLightIndex::Enum idx, DirectionalLightInfo& data){
	mImpl->GetDirectionalLightInfo(idx, data);
}

void Scene::PreRender(const RenderParam& prarm, RenderParamOut* paramOut) {
	mImpl->PreRender(prarm, paramOut);
}

void Scene::Render(const RenderParam& prarm, RenderParamOut* paramOut) {
	mImpl->Render(prarm, paramOut);
}

/// Returns currently rendering pass which is RenderParam.mRenderPass when the Render() is called.
int Scene::GetRenderPass() const {
	return mImpl->GetRenderPass();
}

bool Scene::AttachObjectFB(SceneObjectPtr object, SceneObject*) {
	return mImpl->AttachObject(object);
}

bool Scene::DetachObject(SceneObject* object) {
	return mImpl->DetachObject(object);
}

bool Scene::AttachObjectFB(SpatialSceneObjectPtr object, SpatialSceneObject*) {
	return mImpl->AttachSpatialObject(object);
}

bool Scene::DetachObject(SpatialSceneObject* object) {
	return mImpl->DetachSpatialObject(object);
}

void Scene::SetSkipSpatialObjects(bool skip) {
	mImpl->SetSkipSpatialObjects(skip);
}

void Scene::ClearEverySpatialObject() {
	mImpl->ClearEverySpatialObject();
}

unsigned Scene::GetNumSpatialObjects() const {
	return mImpl->GetNumSpatialObjects();
}

const Scene::SPATIAL_OBJECTS_RAW* Scene::GetVisibleSpatialList(ICameraPtr cam) {
	return mImpl->GetVisibleSpatialList(cam);
}

void Scene::PrintSpatialObject() {
	mImpl->PrintSpatialObject();
}

void Scene::AttachSky(SceneObjectPtr p) {
	mImpl->AttachSky(p);
}

void Scene::DetachSky() {
	mImpl->DetachSky();
}

SceneObjectPtr Scene::GetSky() {
	return mImpl->GetSky();
}

void Scene::ToggleSkyRendering() {
	mImpl->ToggleSkyRendering();
}

void Scene::SetSkyRendering(bool render) {
	mImpl->SetSkyRendering(render);
}

bool Scene::GetSkyRendering() {
	return mImpl->GetSkyRendering();
}

const Vec3& Scene::GetWindVector() const {
	return mImpl->GetWindVector();
}

void Scene::AddCloudVolume(SpatialSceneObjectPtr p) {
	mImpl->AddCloudVolume(p);
}

void Scene::ClearClouds() {
	mImpl->ClearClouds();
}

void Scene::PreRenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut) {
	mImpl->PreRenderCloudVolumes(prarm, paramOut);
}

void Scene::RenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut) {
	mImpl->RenderCloudVolumes(prarm, paramOut);
}

const Color& Scene::GetFogColor() const {
	return mImpl->GetFogColor();
}

void Scene::SetFogColor(const Color& c) {
	mImpl->SetFogColor(c);
}

void Scene::SetDrawClouds(bool e) {
	mImpl->SetDrawClouds(e);
}

void Scene::SetRttScene(bool set) {
	mImpl->SetRttScene(set);
}

bool Scene::IsRttScene() const {
	return mImpl->IsRttScene();
}

DirectionalLightPtr Scene::GetDirectionalLight(unsigned idx) {
	return mImpl->GetDirectionalLight(idx);
}

PointLightPtr Scene::CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion) {
	return mImpl->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
}

PointLightManagerPtr Scene::GetPointLightMan() const {
	return mImpl->GetPointLightMan();
}

void Scene::GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst) {
	mImpl->GatherPointLightData(aabb, transform, plConst);
}

void Scene::RefreshPointLight() {
	mImpl->RefreshPointLight();
}

bool Scene::NeedToRefreshPointLight() const {
	return mImpl->NeedToRefreshPointLight();
}