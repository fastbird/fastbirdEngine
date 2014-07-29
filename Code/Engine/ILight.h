#pragma once
#include <CommonLib/SmartPtr.h>
namespace fastbird
{
	class Vec3;
	class ILight : public ReferenceCounter
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

		virtual void SetPosition(const Vec3& pos) = 0;
		virtual void SetDiffuse(const Vec3& diffuse) = 0;
		virtual void SetSpecular(const Vec3& specular) = 0;
		virtual void SetIntensity(float intensity) = 0;
		virtual void SetAttenuation(float attenuation) = 0;
		virtual void SetExponent(float exponent) = 0;
	};
}