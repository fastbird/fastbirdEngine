#pragma once

#include <Engine/SceneGraph/SpatialObject.h>
namespace fastbird
{
	class CLASS_DECLSPEC_ENGINE IDustRenderer : public SpatialObject
	{
	public:
		static IDustRenderer* CreateDustRenderer();

		virtual void InitDustRenderer(const Vec3& min, const Vec3& max, size_t count, 
			const Color& cmin,
			const Color& cmax, 
			float normalizeDist) = 0;

		virtual const Vec3& GetMin() const = 0;
	};
}