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
		return new DirectionalLight();
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
{

}

//----------------------------------------------------------------------------
void DirectionalLight::SetPosition(const Vec3& pos)
{
	mDirection = pos;
	mDirection.Normalize();
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