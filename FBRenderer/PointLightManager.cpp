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
#include "FBCommonHeaders/Helpers.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

class PointLightManager::Impl{
public:
	typedef std::vector< PointLightWeakPtr > PointLights;
	PointLights mPointLights;

	PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion)
	{
		auto newLight = PointLight::Create(pos, range, color, intensity, lifeTime, manualDeletion);
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

	void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst)
	{
		struct GatheredData
		{
			GatheredData(Real distSQ, unsigned idx)
				:mDistanceSQ(distSQ), mIndex(idx)
			{

			}
			Real mDistanceSQ;
			unsigned mIndex;
		};
		static std::vector<GatheredData> gathered;
		gathered.reserve(50);

		unsigned i = 0;
		for (auto& it : mPointLights)
		{			
			auto p = it.lock();
			if (!p->GetEnabled())
				continue;
			Ray3 ray(p->GetPosition(), transform.GetTranslation() - p->GetPosition());
			Ray3 localRay = transform.ApplyInverse(ray);
			auto iresult = localRay.Intersects(aabb);
			Real distSQ = Squared(iresult.second);
			Real range = p->GetRange();
			if (distSQ < (range*range))
			{
				gathered.push_back(GatheredData(distSQ, i));
			}
			++i;
		}

		std::sort(gathered.begin(), gathered.end(), [](const GatheredData& a, const GatheredData& b){
			return a.mDistanceSQ < b.mDistanceSQ;
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

