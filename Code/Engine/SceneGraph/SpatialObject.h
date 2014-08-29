#pragma once
#ifndef _fastbird_SpatialObject_header_included_
#define _fastbird_SpatialObject_header_included_

#include <Engine/Foundation/Object.h>
#include <CommonLib/Math/Transformation.h>

namespace fastbird
{
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
		virtual const Quat& GetRot() const;
		virtual void SetTransform(const Transformation& t);
		virtual void GetTransform(Mat44& outMat) const { return mTransformation.GetHomogeneous(outMat); }
		virtual const Transformation& GetTransform() const { return mTransformation; }
		virtual void AttachToScene();
		virtual void DetachFromScene();
		
		Vec3 GetRight() { return mTransformation.GetRight(); }
		Vec3 GetForward() { return mTransformation.GetForward(); }
		Vec3 GetUp() { return mTransformation.GetUp();}

		void CameraTargetingYou(ICamera* pCam) { mCameraTargeting = pCam; }
		const Vec3& GetPrevPos() const { return mPrevPos; }

	protected:
		Transformation mTransformation;
		Vec3 mPrevPos;
		float mDistToCam;
		bool mTransformChanged;
		ICamera* mCameraTargeting;

	};
}

#endif //_fastbird_ISpatialObject_header_included_