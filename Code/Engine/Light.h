#include <Engine/StdAfx.h>
#include <Engine/ILight.h>
#include <Engine/ICamera.h>
#include <CommonLib/Math/Vec3.h>
namespace fastbird
{
	//------------------------------------------------------------------------
	class DirectionalLight : public ILight, public ICameraListener
	{
	public:
		DirectionalLight();
		virtual ~DirectionalLight();

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

		virtual void AddTheta(float radian);
		virtual void AddPhi(float radian);

		virtual void SetPosition(const Vec3& pos);
		virtual void SetDiffuse(const Vec3& diffuse);
		virtual void SetSpecular(const Vec3& specular);
		virtual void SetIntensity(float intensity);
		virtual void SetAttenuation(float attenuation){}
		virtual void SetExponent(float exponent) {}

		virtual void PrepareInterpolation(float destTheta, float destPhi, float destIntensity, const Vec3& destDiffuse, float time);
		virtual void AddInterpolTime(float time);
		virtual const Vec3& GetInterpolDir(unsigned target) const;
		virtual float GetInterpolIntensity(unsigned target) const;
		virtual const Vec3& GetInterpolDiffuse(unsigned target) const;
		virtual float GetTheta() const;
		virtual float GetPhi() const;

		virtual void Update(float dt);

		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();

		virtual ICamera* GetCamera() const { return mLightCamera; }

		void SetUpCamera();

		virtual void OnViewMatChanged();
		virtual void OnProjMatChanged();

		virtual void CopyLight(ILight* other);

	private:
		Vec3 mDirection;
		Vec3 mDiffuse;
		Vec3 mSpecular;
		float mTheta;
		float mPhi;
		float mIntensity;

		// for interpolation
		float mInterpolPhi[2]; // 0 cur, //1 dest
		float mInterpolTheta[2];
		float mInterpolIntensity[2];
		Vec3 mInterpolDiffuse[2];
		float mInterpolTime[2];
		bool mInterpolating;

		ICamera* mLightCamera;
		bool mDuplicatedLightCamera;

	};
}
