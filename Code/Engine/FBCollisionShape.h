#pragma once
#include <CommonLib/FBColShape.h>
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Math/Transformation.h>
namespace fastbird
{
	class IMeshObject;
	class FBCollisionShape
	{
	public:
		FBCollisionShape(FBColShape::Enum e, const Transformation& t, IMeshObject* colMesh);
		~FBCollisionShape();

		void SetCollisionMesh(IMeshObject* colMesh) { mColMesh = colMesh; }
		IMeshObject* GetCollisionMesh() const { return mColMesh; }
		BoundingVolume* GetBV() const { return mBV; }
		FBColShape::Enum GetColShape() const { return mColShape; }
		Vec3 GetOffset() const{
			return mTransformation.GetTranslation();
		}
		Quat GetRot() const{
			return mTransformation.GetRotation();
		}
		Vec3 GetScale() const{
			return mTransformation.GetScale();
		}

		typedef std::pair<bool, float> IResult;
		IResult intersects(const Ray3& ray, const Transformation& objT) const;
		bool TestCollision(fastbird::BoundingVolume* pBV, const Transformation& objT) const;
		Vec3 GetRandomPosInVolume(const Vec3* nearWorld, const Transformation& objT) const;

	private:
		FBColShape::Enum mColShape;
		SmartPtr<BoundingVolume> mBV;
		SmartPtr<IMeshObject> mColMesh;
		Transformation mTransformation;


	};
}