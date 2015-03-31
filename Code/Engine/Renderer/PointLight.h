#pragma once
#include <Engine/IPointLight.h>

namespace fastbird
{
	class PointLight : public IPointLight
	{
		friend class PointLightMan;
		Vec3 mColor;
		Vec3 mColorPowered; // multiplied by intensity;
		float mIntensity;
		float mRange;
		float mLifeTime; // -1 is infinite
		float mAlpha;
		bool mManualDeletion;
		bool mEnabled;

	public:
		PointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime, bool manualDeletion);
		virtual ~PointLight(){}

		//-------------------------------------------------------------------
		// IObject
		//-------------------------------------------------------------------
		virtual void PreRender(){}
		virtual void Render(){}
		virtual void PostRender(){}

		//-------------------------------------------------------------------
		// OWN
		//-------------------------------------------------------------------
		virtual void SetPos(const Vec3& pos);
		virtual void SetRange(float range);
		virtual void SetColorAndIntensity(const Vec3& color, float intensity);

		virtual float GetRange() const;
		virtual const Vec3& GetColor() const;
		virtual float GetIntensity() const;

		virtual void SetLifeTime(float lifeTime);
		virtual float GetLifeTime() const;

		virtual void SetManualDeletion(bool manual);
		virtual bool GetManualDeletion() const;

		virtual void SetAlpha(float alpha);
		virtual bool GetEnabled() const { return mEnabled; }
		virtual void SetEnabled(bool enable);
	};
}