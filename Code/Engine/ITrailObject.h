#pragma once
#include <Engine/Object.h>

namespace fastbird{
	class ITrailObject : public Object{
	public:
		virtual ~ITrailObject(){}

		//for billboard trail - automatically face to the camera
		virtual void AddPoint(const Vec3& worldPos) = 0;
		virtual void SetWidth(float width) = 0;

		// for manual trail
		virtual void AddPoint(const Vec3& worldPosA, const Vec3& worldPosB) = 0;

		virtual void SetMaxPoints(unsigned num) = 0;
		virtual void Clear() = 0;

	};
}