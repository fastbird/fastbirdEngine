#pragma once
#ifndef _Camera_Header_included_
#define _Camera_Header_included_

#include <Engine/ICamera.h>
#include <CommonLib/Math/Plane3.h>

namespace fastbird
{
	class Camera : public ICamera
	{
	public:
		enum FRUSTUM_PLANE
		{
			FRUSTUM_PLANE_NEAR   = 0,
			FRUSTUM_PLANE_FAR    = 1,
			FRUSTUM_PLANE_LEFT   = 2,
			FRUSTUM_PLANE_RIGHT  = 3,
			FRUSTUM_PLANE_TOP    = 4,
			FRUSTUM_PLANE_BOTTOM = 5
		};

		Camera();
		~Camera();

		//-------------------------------------------------------------------------
		// IObject
		//-------------------------------------------------------------------------
		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();

		//-------------------------------------------------------------------------
		virtual void SetOrthogonal(bool ortho);
		virtual void SetPos(const Vec3& pos);
		virtual const Vec3& GetPos() const;
		virtual void SetDir(const Vec3& dir);
		virtual void SetRot(const Quat& rot);
		virtual const Quat& GetRot() const { return mTransformation.GetRotation(); }
		virtual void SetTransformation(const Vec3& pos, const Quat& rot);
		virtual void SetTransform(const Transformation& t);
		virtual const Vec3 GetDir() const;
		virtual const Mat44& GetViewMat();
		virtual const Mat44& GetInvViewMat();
		virtual const Mat44& GetInvViewProjMat();
		virtual const Mat44& GetProjMat();
		virtual const Mat44& GetViewProjMat();
		// field of view in the y direction, in radians.
		virtual void SetFOV(float fov) { mFov = fov; mProjPropertyChanged = true;}
		virtual float GetFOV() const { return mFov; }
		// width / height
		virtual void SetAspectRatio(float ar) { mAspectRatio = ar; mProjPropertyChanged = true;}
		virtual float GetAspectRatio() const { return mAspectRatio; }
		// near/far view-plane
		virtual void SetNearFar(float nearPlane, float farPlane);
		virtual void GetNearFar(float& nearPlane, float& farPlane) const;
		// width and height of the view volume at the near view-plane
		virtual void SetWidth(float width) { mWidth = width; mProjPropertyChanged = true;}
		virtual void SetHeight(float height) { mHeight = height; mProjPropertyChanged = true;}
		virtual float GetWidth() const { return mWidth; }
		virtual float GetHeight() const { return mHeight; }
		virtual void SetName(const char* name) { mName = name; }
		virtual const char* GetName() const { return mName.c_str(); }
		virtual void Update();
		virtual bool IsCulled(BoundingVolume* pBV) const;
		virtual Ray3 ScreenPosToRay(long x, long y);
		virtual void SetYZSwap(bool enable){mYZSwap = enable; mProjPropertyChanged = true; }
		virtual const Vec3& GetFrustumMin() const { return mFrustumMin; }
		virtual const Vec3& GetFrustumMax() const { return mFrustumMax; }

	protected:
		void UpdateFrustum();


	private:

		bool mViewPropertyChanged;
		bool mProjPropertyChanged;
		bool mOrthogonal;
		Mat44 mViewMat;
		Mat44 mInvViewMat;
		Mat44 mProjMat;
		Mat44 mViewProjMat;
		Mat44 mInvViewProjMat;
		float mFov;
		float mAspectRatio;
		float mNear;
		float mFar;
		float mWidth;
		float mHeight;
		std::string mName;
		Plane3 mPlanes[6];
		Vec3 mFrustumMax;
		Vec3 mFrustumMin;
		bool mYZSwap;
	};
}

#endif //_Camera_Header_included_