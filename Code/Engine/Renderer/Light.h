#include <Engine/StdAfx.h>
#include <Engine/ILight.h>
#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	//------------------------------------------------------------------------
	class DirectionalLight : public ILight
	{
	public:
		DirectionalLight();

		//--------------------------------------------------------------------
		// ILight Interfaces
		//--------------------------------------------------------------------
		virtual LIGHT_TYPE GetLightType() const { return LIGHT_TYPE_DIRECTIONAL; }
		virtual const Vec3& GetPosition() const { return mDirection; }
		virtual const Vec3& GetDiffuse() const { return mDiffuse; }
		virtual const Vec3& GetSpecular() const { return mSpecular; }
		virtual float GetIntensity() const { return mIntensity; }
		virtual float GetAttenuation() const { return 0.0f; }
		virtual float GetExponent() const { return 0.0f; }

		virtual void SetPosition(const Vec3& pos);
		virtual void SetDiffuse(const Vec3& diffuse);
		virtual void SetSpecular(const Vec3& specular);
		virtual void SetIntensity(float intensity);
		virtual void SetAttenuation(float attenuation){}
		virtual void SetExponent(float exponent) {}

	private:
		Vec3 mDirection;
		Vec3 mDiffuse;
		Vec3 mSpecular;
		float mIntensity;
	};
}
