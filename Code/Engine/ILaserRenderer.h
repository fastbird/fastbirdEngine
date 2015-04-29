#pragma once
#include <Engine/SpatialObject.h>
namespace fastbird
{
	class ILaserRenderer : public SpatialObject
	{
	public:
		static ILaserRenderer* CreateLaserRenderer();

		void AddLaser(const fastbird::Vec3& from, const fastbird::Vec3& to, float thickness, const fastbird::Color& color);
	};
}