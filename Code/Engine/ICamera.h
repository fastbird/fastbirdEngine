#pragma once
#ifndef _ICamera_header_included_
#define _ICamera_header_included_

#include <CommonLib/SmartPtr.h>
#include <Engine/SceneGraph/SpatialObject.h>

namespace fastbird
{
	class BoundingVolume;
	class ICamera : public SpatialObject
	{
	public:
		virtual ~ICamera(){}
		virtual void SetOrthogonal(bool ortho) = 0;
		virtual void SetPos(const Vec3& pos) = 0;
		virtual const Vec3& GetPos() const = 0;
		virtual void SetDir(const Vec3& dir) = 0;
		virtual const Vec3 GetDir() const = 0;
		virtual void SetCamTransform(const Vec3& pos, const Quat& rot) = 0;
		virtual const Mat44& GetViewMat() = 0;
		virtual const Mat44& GetInvViewMat() = 0;
		virtual const Mat44& GetInvViewProjMat() = 0;
		virtual const Mat44& GetProjMat() = 0;
		virtual const Mat44& GetViewProjMat() = 0;
		
		// field of view in the y direction, in radians.
		virtual void SetFOV(float fov) = 0;
		virtual float GetFOV() const = 0;
		// width / height
		virtual void SetAspectRatio(float ar) = 0;
		virtual float GetAspectRatio() const = 0;
		// near/far view-plane
		virtual void SetNearFar(float nearPlane, float farPlane) = 0;
		virtual void GetNearFar(float& nearPlane, float& farPlane) const = 0;
		// width and height of the view volume at the near view-plane
		virtual void SetWidth(float width) = 0;
		virtual void SetHeight(float height) = 0;
		virtual float GetWidth() const = 0;
		virtual float GetHeight() const = 0;
		virtual void SetName(const char* name) = 0;
		virtual const char* GetName() const = 0;
		virtual void ProcessInputData() = 0;
		virtual void Update() = 0;

		virtual bool IsCulled(BoundingVolume* pBV) const = 0;
		virtual Ray3 ScreenPosToRay(long x, long y) = 0;

		virtual void SetYZSwap(bool enable) = 0;

		// you usually don't need to use this function
		// recommend you to create your own camera manager;
		// this is for engine internal - particle editor.
		virtual void SetTarget(SpatialObject* pObj) = 0;

		virtual void SetCurrent(bool cur) = 0;
		virtual void SetCameraIndex(size_t idx) = 0;
		virtual void SetEnalbeInput(bool enable) = 0;

		// internal use only
		virtual void OnInputFromEngine(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard) = 0;
	};
}

#endif//_ICamera_header_included_