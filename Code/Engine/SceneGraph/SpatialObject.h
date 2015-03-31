#pragma once
#ifndef _fastbird_SpatialObject_header_included_
#define _fastbird_SpatialObject_header_included_

#include <Engine/Foundation/Object.h>
#include <CommonLib/Math/Transformation.h>

namespace fastbird
{
	class AnimationData;
	class FBCollisionShape;
	class SpatialObject : public Object
	{
	public:
		SpatialObject();
		virtual ~SpatialObject();

		virtual void Clone(IObject* cloned) const;
		virtual void SetDistToCam(float dist);
		virtual float GetDistToCam() const;
		virtual void SetPos(const Vec3& pos);
		virtual const Vec3& GetPos() const;
		virtual void SetRot(const Quat& rot);
		virtual void SetScale(const Vec3& scale); // use uniform only.
		virtual void SetDir(const Vec3& dir);
		virtual void SetDirAndRight(const Vec3& dir, const Vec3& right);
		virtual const Quat& GetRot() const;
		virtual void SetTransform(const Transformation& t);
		virtual void GetTransform(Mat44& outMat) const { return mTransformation.GetHomogeneous(outMat); }
		virtual const Transformation& GetTransform() const { return mTransformation; }
		virtual void AttachToScene();
		virtual void DetachFromScene();

		// IObject
		virtual void PreRender();
		
		Vec3 GetRight() { return mTransformation.GetRight(); }
		Vec3 GetForward() { return mTransformation.GetForward(); }
		Vec3 GetUp() { return mTransformation.GetUp();}

		const Vec3& GetPrevPos() const { return mPrevPos; }

		// own
		virtual unsigned GetNumCollisionShapes() const { return 0; }
		virtual bool HasCollisionShapes() const { return false; }
		virtual const FBCollisionShape* GetCollisionShape(unsigned idx) const { assert(0); return 0; }
		// Only called when broad collision is true.
		virtual bool CheckNarrowCollision(fastbird::BoundingVolume* pBV) const { return true; }
		virtual Ray3::IResult CheckNarrowCollisionRay(const Ray3& ray) const { return Ray3::IResult(false, 0.f); }
		virtual Vec3 GetRandomPosInVolume(const Vec3* nearWorld = 0) const { return Vec3::ZERO; }

		void SetAnimationData(const AnimationData& anim, const char* actionFile);
		AnimationData* GetAnimationData() const { return mAnimData; }
		Animation* GetAnimation() const { return mAnim; }

	private:
		friend class Camera;
		void AddCameraTargetingMe(ICamera* pCam);
		void RemoveCameraTargetingMe(ICamera* pCam);

	protected:
		Transformation mTransformation;
		Transformation mAnimatedTransformation;
		Vec3 mPrevPos;
		float mDistToCam;
		bool mTransformChanged;
		std::vector<ICamera*> mCameraTargetingMe;
		AnimationData* mAnimData;
		Animation* mAnim;
		bool mAnimOwner;

	};
}

#endif //_fastbird_ISpatialObject_header_included_