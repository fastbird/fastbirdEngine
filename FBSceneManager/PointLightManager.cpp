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
#include "PointLightManager.h"
#include "PointLight.h"
#include "SceneManager.h"
#include "SceneManagerOptions.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/RendererOptions.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

class PointLightManager::Impl{
public:
	typedef std::vector< PointLightWeakPtr > PointLights;
	SceneWeakPtr mScene;
	PointLights mPointLights;

	void SetScene(ScenePtr scene){
		mScene = scene;
	}

	PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion)
	{
		if (mPointLights.size() > 20)
			return nullptr;

		auto scene = mScene.lock();
		if (!scene){
			Logger::Log(FB_ERROR_LOG_ARG, "No scene");
			return 0;
		}
		auto newLight = PointLight::Create(scene, pos, range, color, intensity, lifeTime, manualDeletion);
		mPointLights.push_back(newLight);
		return newLight;
	}

	void Update(Real dt)
	{
		static std::vector<unsigned> deleted;

		for (auto it = mPointLights.begin(); it != mPointLights.end();){
			auto p = it->lock();
			if (!p){
				it = mPointLights.erase(it);
				continue;
			}

			if (p->GetLifeTime() > 0)
			{
				auto lifeTime = p->GetOlder(dt);
				if (lifeTime <= 0 && !p->GetManualDeletion())
				{
					it = mPointLights.erase(it);
					continue;
				}
			}
			++it;
		}		
	}

	void GatherPointLightData(const BoundingVolume* boundingVolume, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst)
	{	
		if (SceneManager::GetInstance().GetOptions()->r_noPointLight) {
			plConst->gPointLightColor[0].w = 0;
			return;
		}

		struct GatheredData
		{
			GatheredData(Real intensity, unsigned idx)
				:mIntensity(intensity), mIndex(idx)
			{

			}
			Real mIntensity;
			unsigned mIndex;
		};
		static std::vector<GatheredData> gathered;
		gathered.reserve(50);

		unsigned i = 0;
		for (auto it = mPointLights.begin(); it != mPointLights.end(); /**/)
		{		
			IteratingWeakContainer(mPointLights, it, p);
			if (!p->GetEnabled())
				continue;
			Ray ray(p->GetPosition(), transform.GetTranslation() - p->GetPosition());
			Ray localRay = transform.ApplyInverse(ray, false);
			auto iresult = localRay.Intersects(boundingVolume);
			Real dist = iresult.second;
			Real range = p->GetRange();
			if (dist < range)
			{
				auto intensity = p->GetIntensityScoreAtRange(dist);
				gathered.push_back(GatheredData(intensity, i));
			}
			++i;
		}

		std::sort(gathered.begin(), gathered.end(), [](const GatheredData& a, const GatheredData& b){
			return a.mIntensity > b.mIntensity;
		}
		);

		plConst->gPointLightColor[0].w = 0;
		int count = std::min(3, (int)gathered.size());
		unsigned validNumber = 0;
		for (int i = 0; i < count; i++)
		{
			PointLightPtr p = mPointLights[gathered[i].mIndex].lock();
			if (p){
				plConst->gPointLightPos[validNumber] = Vec4(p->GetPosition(), p->GetRange());
				plConst->gPointLightColor[validNumber] = Vec4(p->GetColorPowered(), (Real)count);
				++validNumber;
			}
		}

		gathered.clear();
	}

	unsigned GetNumPointLights() const
	{
		return mPointLights.size();
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(PointLightManager);

PointLightManager::PointLightManager()
	:mImpl(new Impl)
{
}

void PointLightManager::SetScene(ScenePtr scene){
	mImpl->SetScene(scene);
}

PointLightPtr PointLightManager::CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion) {
	return mImpl->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
}

void PointLightManager::Update(Real dt) {
	mImpl->Update(dt);
}

void PointLightManager::GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst) {
	mImpl->GatherPointLightData(aabb, transform, plConst);
}

unsigned PointLightManager::GetNumPointLights() const {
	return mImpl->GetNumPointLights();
}

