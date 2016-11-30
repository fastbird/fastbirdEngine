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
#include "PointLight.h"
#include "Scene.h"
using namespace fb;
class PointLight::Impl{
public:
	SceneWeakPtr mScene;
	Vec3 mPosition;
	Vec3 mColor;
	Vec3 mColorPowered; // multiplied by intensity;
	Real mIntensity;
	Real mRange;
	Real mLifeTime; // -1 is infinite
	Real mAlpha;
	bool mManualDeletion;
	bool mEnabled;

	Impl(ScenePtr scene, const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion)
		: mScene(scene)
		, mPosition(pos)
		, mRange(range)
		, mColor(color)
		, mColorPowered(color*intensity)
		, mIntensity(intensity)
		, mLifeTime(lifeTime)
		, mManualDeletion(manualDeletion)
		, mAlpha(1.0f)
		, mEnabled(true)
	{
	}

	const Vec3& GetPosition() const{
		return mPosition;
	}

	void SetPosition(const Vec3& pos)
	{
		mPosition = pos;
		auto scene = mScene.lock();
		scene->RefreshPointLight();
	}

	void SetRange(Real range)
	{
		mRange = range;
	}

	void SetColorAndIntensity(const Vec3& color, Real intensity)
	{
		mColor = color;
		mIntensity = intensity;
		mColorPowered = color * intensity;
	}

	Real GetRange() const
	{
		return mRange * mAlpha;
	}

	const Vec3& GetColor() const
	{
		return mColor;
	}

	const Vec3& GetColorPowered() const{
		return mColorPowered;
	}

	Real GetIntensity() const
	{
		return mIntensity * mAlpha;
	}

	void SetLifeTime(Real lifeTime)
	{
		mLifeTime = lifeTime;
	}

	Real GetLifeTime() const
	{
		return mLifeTime;
	}

	void SetManualDeletion(bool manual)
	{
		mManualDeletion = manual;
	}

	bool GetManualDeletion() const
	{
		return mManualDeletion;
	}

	void SetAlpha(Real alpha)
	{
		mAlpha = alpha;
		mColorPowered = mColor * mIntensity * alpha;
	}

	void SetEnabled(bool enable)
	{
		if (mEnabled == enable)
			return;
		mEnabled = enable;
		auto scene = mScene.lock();
		if (scene){
			scene->RefreshPointLight();
		}
	}

	bool GetEnabled() const { 
		return mEnabled; 
	}

	Real GetOlder(Real time){
		return mLifeTime -= time;
	}

	float GetIntensityScoreAtRange(float dist) {
		dist = std::min(mRange, dist);
		return (mRange - dist) * mIntensity;
	}
};
//---------------------------------------------------------------------------
PointLightPtr PointLight::Create(ScenePtr scene, const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion){
	return PointLightPtr(new PointLight(scene, pos, range, color, intensity, lifeTime, manualDeletion), [](PointLight* obj){ delete obj; });
}

PointLight::PointLight(ScenePtr scene, const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion)
	:mImpl(new Impl(scene, pos, range, color, intensity, lifeTime, manualDeletion))
{
}

PointLight::~PointLight(){

}

const Vec3& PointLight::GetPosition() const{
	return mImpl->GetPosition();
}

void PointLight::SetPosition(const Vec3& pos){
	mImpl->SetPosition(pos);
}

void PointLight::SetRange(Real range){
	mImpl->SetRange(range);
}

void PointLight::SetColorAndIntensity(const Vec3& color, Real intensity){
	mImpl->SetColorAndIntensity(color, intensity);
}

Real PointLight::GetRange() const{
	return mImpl->GetRange();
}

const Vec3& PointLight::GetColor() const{
	return mImpl->GetColor();
}

const Vec3& PointLight::GetColorPowered() const{
	return mImpl->GetColorPowered();
}

Real PointLight::GetIntensity() const{
	return mImpl->GetIntensity();
}

void PointLight::SetLifeTime(Real lifeTime){
	mImpl->SetLifeTime(lifeTime);
}

Real PointLight::GetLifeTime() const{
	return mImpl->GetLifeTime();
}

void PointLight::SetManualDeletion(bool manual){
	mImpl->SetManualDeletion(manual);
}

bool PointLight::GetManualDeletion() const{
	return mImpl->GetManualDeletion();
}

void PointLight::SetAlpha(Real alpha){
	mImpl->SetAlpha(alpha);
}

bool PointLight::GetEnabled() const{
	return mImpl->GetEnabled();
}

void PointLight::SetEnabled(bool enable){
	mImpl->SetEnabled(enable);
}

Real PointLight::GetOlder(Real time){
	return mImpl->GetOlder(time);
}

float PointLight::GetIntensityScoreAtRange(float dist) {
	return mImpl->GetIntensityScoreAtRange(dist);
}