#include <Engine/StdAfx.h>
#include <Engine/Renderer/PointLight.h>

namespace fastbird
{
	PointLight::PointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime, bool manualDeletion)
		: mRange(range)
		, mColor(color)
		, mColorPowered(color)
		, mIntensity(intensity)
		, mLifeTime(lifeTime)
		, mManualDeletion(manualDeletion)
		, mAlpha(1.0f)
		, mEnabled(true)
	{
		mColorPowered *= intensity;
		SetPos(pos);
	}

	void PointLight::SetPos(const Vec3& pos)
	{
		SpatialObject::SetPos(pos);
		gFBEnv->pRenderer->RefreshPointLight();
	}

	void PointLight::SetRange(float range)
	{
		mRange = range;
	}

	void PointLight::SetColorAndIntensity(const Vec3& color, float intensity)
	{
		mColor = color;
		mIntensity = intensity;
		mColorPowered = color * intensity;
	}

	float PointLight::GetRange() const
	{
		return mRange * mAlpha;
	}

	const Vec3& PointLight::GetColor() const
	{
		return mColor;
	}

	float PointLight::GetIntensity() const
	{
		return mIntensity * mAlpha;
	}

	void PointLight::SetLifeTime(float lifeTime)
	{
		mLifeTime = lifeTime;
	}

	float PointLight::GetLifeTime() const
	{
		return mLifeTime;
	}

	void PointLight::SetManualDeletion(bool manual)
	{
		mManualDeletion = manual;
	}

	bool PointLight::GetManualDeletion() const
	{
		return mManualDeletion;
	}

	void PointLight::SetAlpha(float alpha)
	{
		mAlpha = alpha;
		mColorPowered = mColor * mIntensity * alpha;
	}

	void PointLight::SetEnabled(bool enable)
	{
		if (mEnabled == enable)
			return;
		mEnabled = enable;
		gFBEnv->pRenderer->RefreshPointLight();
	}
}