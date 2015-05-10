#pragma once
#include <CommonLib/SmartPtr.h>
#include <Engine/SpatialObject.h>
namespace fastbird
{
	class Vec3;
	class ILight : public SpatialObject
	{
	public:
		enum LIGHT_TYPE {
			LIGHT_TYPE_DIRECTIONAL,
			LIGHT_TYPE_POINT,
			LIGHT_TYPE_AMBIENT,
			LIGHT_TYPE_SPOT
		};
		virtual ~ILight(){}

		static ILight* CreateLight(LIGHT_TYPE type);

		// retrieve position or direction
		virtual LIGHT_TYPE GetLightType() const = 0;
		virtual const Vec3& GetPosition() const = 0;
		virtual const Vec3& GetDiffuse() const = 0;
		virtual const Vec3& GetSpecular() const = 0;
		virtual float GetIntensity() const = 0;
		virtual float GetAttenuation() const = 0;
		virtual float GetExponent() const = 0;

		virtual void AddTheta(float radian) = 0;
		virtual void AddPhi(float radian) = 0;

		virtual void SetPosition(const Vec3& pos) = 0;
		virtual void SetDiffuse(const Vec3& diffuse) = 0;
		virtual void SetSpecular(const Vec3& specular) = 0;
		virtual void SetIntensity(float intensity) = 0;
		virtual void SetAttenuation(float attenuation) = 0;
		virtual void SetExponent(float exponent) = 0;

		virtual void PrepareInterpolation(float destTheta, float destPhi, float destIntensity, const Vec3& destDiffuse, float time) = 0;
		virtual void AddInterpolTime(float time) = 0;
		virtual void Update(float dt) = 0;

		virtual const Vec3& GetInterpolDir(unsigned target)  const = 0;
		virtual float GetInterpolIntensity(unsigned target)  const = 0;
		virtual const Vec3& GetInterpolDiffuse(unsigned target)  const = 0;

		virtual float GetTheta() const = 0;
		virtual float GetPhi() const = 0;

		virtual void CopyLight(ILight* other) = 0;
	};
}