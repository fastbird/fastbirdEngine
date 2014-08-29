#include <Engine/StdAfx.h>
#include <Engine/Renderer/Light.h>
#include <Engine/IEngine.h>

using namespace fastbird;

//----------------------------------------------------------------------------
ILight* ILight::CreateLight(LIGHT_TYPE lt)
{
	switch(lt)
	{
	case LIGHT_TYPE_DIRECTIONAL:
		return FB_NEW(DirectionalLight);
		break;
	default:
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Creating unidentified light type!");
		assert(0);
	};
	assert(0);
	return 0;
}

//----------------------------------------------------------------------------
DirectionalLight::DirectionalLight()
: mTheta(0)
, mPhi(0)
, mInterpolating(false)
{

}

//----------------------------------------------------------------------------
void DirectionalLight::SetPosition(const Vec3& pos)
{
	mDirection = pos;
	mDirection.Normalize();

	mTheta = acos(mDirection.z);
	mPhi = atan2(mDirection.y, mDirection.x);
	mPhi += mPhi < 0 ? TWO_PI : 0;
}

//----------------------------------------------------------------------------
void DirectionalLight::SetDiffuse(const Vec3& diffuse)
{
	mDiffuse = diffuse;
}

//----------------------------------------------------------------------------
void DirectionalLight::SetSpecular(const Vec3& specular)
{
	mSpecular = specular;
}

//----------------------------------------------------------------------------
void DirectionalLight::SetIntensity(float intensity)
{
	mIntensity = intensity;
}

//----------------------------------------------------------------------------
void DirectionalLight::AddTheta(float radian)
{
	if (radian == 0.f)
		return;
	mTheta += radian;
	if (mTheta > PI)
	{
		mTheta = mTheta - PI;
	}

	mDirection = SphericalToCartesian(mTheta, mPhi);

}

//----------------------------------------------------------------------------
void DirectionalLight::AddPhi(float radian)
{
	if (radian == 0.f)
		return;
	mPhi += radian;
	if (mPhi >= TWO_PI)
	{
		mPhi = mPhi - TWO_PI;
	}

	mDirection = SphericalToCartesian(mTheta, mPhi);
}

//----------------------------------------------------------------------------
void DirectionalLight::PrepareInterpolation(float destTheta, float destPhi, float destIntensity, const Vec3& destDiffuse, float time)
{
	mInterpolPhi[0] = mPhi;
	mInterpolTheta[0] = mTheta;
	mInterpolIntensity[0] = mIntensity;
	mInterpolDiffuse[0] = mDiffuse;

	mInterpolPhi[1] = destPhi;
	mInterpolTheta[1] = destTheta;
	mInterpolIntensity[1] = destIntensity;
	mInterpolDiffuse[1] = destDiffuse;

	mInterpolTime[0] = time;
	mInterpolTime[1] = 0.f;
	mInterpolating = true;
}

void DirectionalLight::AddInterpolTime(float time)
{
	mInterpolTime[1] += time;
}

const Vec3& DirectionalLight::GetInterpolDir(unsigned target) const
{
	assert(target <= 1);
	return SphericalToCartesian(mInterpolTheta[target], mInterpolPhi[target]);	
}
float DirectionalLight::GetInterpolIntensity(unsigned target) const
{
	assert(target <= 1);
	return mInterpolIntensity[target];
}
const Vec3& DirectionalLight::GetInterpolDiffuse(unsigned target) const
{
	assert(target <= 1);
	return mInterpolDiffuse[target];
}

float DirectionalLight::GetTheta() const
{
	return mTheta;
}
float DirectionalLight::GetPhi() const
{
	return mPhi;
}

void DirectionalLight::Update(float dt)
{
	if (mInterpolating)
	{
		mInterpolTime[1] += gFBEnv->pTimer->GetDeltaTime();
		if (mInterpolTime[1] > mInterpolTime[0])
		{
			mInterpolTime[1] = mInterpolTime[0];
			mInterpolating = false;
		}

		float normTime = mInterpolTime[1] / mInterpolTime[0];
		mPhi = Lerp(mInterpolPhi[0], mInterpolPhi[1], normTime);
		mTheta = Lerp(mInterpolTheta[0], mInterpolTheta[1], normTime);
		mIntensity = Lerp(mInterpolIntensity[0], mInterpolIntensity[1], normTime);
		mDiffuse = Lerp(mInterpolDiffuse[0], mInterpolDiffuse[1], normTime);
		mDirection = SphericalToCartesian(mTheta, mPhi);
	}
}

//---------------------------------------------------------------------------
void DirectionalLight::PreRender()
{
	
}

//---------------------------------------------------------------------------
void DirectionalLight::Render()
{

}

//---------------------------------------------------------------------------
void DirectionalLight::PostRender()
{

}