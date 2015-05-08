#pragma once
#include <Engine/Object.h>
namespace fastbird
{
class ISkySphere : public Object
{
public:
	static ISkySphere* CreateSkySphere(bool usingSmartPointer);
	void Delete(); // only when not using smart ptr.

	virtual void UpdateEnvironmentMap(const Vec3& origin) = 0;
	virtual void SetInterpolationData(unsigned index, const Vec4& data) = 0;
	virtual void PrepareInterpolation(float time, ISkySphere* startFrom) = 0;
	virtual void SetUseAlphaBlend(bool use) = 0;
	virtual void SetAlpha(float alpha) = 0;
	virtual float GetAlpha() const = 0;
};
}