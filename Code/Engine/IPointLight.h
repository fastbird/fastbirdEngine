#pragma once
#include <CommonLib/SmartPtr.h>
#include <Engine/SpatialObject.h>

namespace fastbird
{
	class IPointLight : public SpatialObject
	{
	public:
		virtual ~IPointLight(){}

		virtual void SetPos(const Vec3& pos) = 0;
		virtual void SetRange(float range) = 0;
		virtual void SetColorAndIntensity(const Vec3& color, float intensity) = 0;
		virtual float GetRange() const = 0;
		virtual const Vec3& GetColor() const = 0;
		virtual float GetIntensity() const = 0;

		virtual void SetLifeTime(float lifeTime) = 0;
		virtual float GetLifeTime() const = 0;

		virtual void SetManualDeletion(bool manual) = 0;
		virtual bool GetManualDeletion() const = 0;
		virtual void SetAlpha(float alpha) = 0;
		virtual void SetEnabled(bool enable) = 0;
	};
}