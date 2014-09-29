#pragma once
#include <CommonLib/ColShape.h>
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Math/Transformation.h>
namespace fastbird
{
	class CollisionShape
	{
	public:
		CollisionShape(ColShape::Enum e, const Transformation& t);
		~CollisionShape();

		BoundingVolume* GetBV() const { return mBV; }

		typedef std::pair<bool, float> IResult;
		IResult intersects(const Ray3& ray, const Transformation& objT) const;

	private:
		SmartPtr<BoundingVolume> mBV;
		Transformation mTransformation;


	};
}