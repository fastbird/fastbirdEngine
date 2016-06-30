/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#ifndef _Camera_Header_included_
#define _Camera_Header_included_
#include "FBCommonHeaders/Observable.h"
#include "FBMathLib/Math.h"
#include "ICameraObserver.h"
#include "ICamera.h"

namespace fb
{
	class Frustum;
	FB_DECLARE_SMART_PTR(IInputInjector);
	FB_DECLARE_SMART_PTR(IMouse);
	FB_DECLARE_SMART_PTR(IKeyboard);	
	FB_DECLARE_SMART_PTR(ICameraObserver);
	FB_DECLARE_SMART_PTR(Camera);
	class FB_DLL_RENDERER Camera : public ICamera, public Observable<ICameraObserver>
	{
		FB_DECLARE_PIMPL(Camera);
		Camera();

	public:
		enum ObserverEvents{
			TransformChanged,
		};

		static CameraPtr Create();

		Camera(const Camera& other);
		~Camera();
		Camera& operator= (const Camera& other);

		CameraPtr Clone();
		void SetMainCamera(bool main);
		bool IsMainCamera() const;
		//-------------------------------------------------------------------------
		void SetOrthogonal(bool ortho);
		void SetPosition(const Vec3& pos);
		const Vec3& GetPosition() const;
		void SetDirection(const Vec3& dir);
		void SetDirrectionAndRight(const Vec3& dir, const Vec3& right);
		void SetRotation(const Quat& rot);
		const Quat& GetRotation() const;
		Vec3 GetRight() const;
		Vec3 GetUp() const;
		void SetTransformation(const Vec3& pos, const Quat& rot);
		void SetTransformation(const Transformation& t);
		const Transformation& GetTransformation() const;
		const Vec3 GetDirection() const;
		const Mat44& GetMatrix(MatrixType type);
		
		// vertical field of view, in radians.
		void SetFOV(Real fov) OVERRIDE;
		Real GetFOV() const OVERRIDE;
		Real GetAspectRatio() const OVERRIDE;
		Real GetTanHalfFOV() const;
		// width / height
		void SetAspectRatio(Real ar);		
		// near/far view-plane
		void SetNearFar(Real nearPlane, Real farPlane);
		void GetNearFar(Real& nearPlane, Real& farPlane) const;
		Real GetNear() const;
		Real GetFar() const;		
		void SetNear(Real n);
		void SetFar(Real f);
		// width and height of the view volume at the near view-plane		
		void SetWidth(Real width);
		void SetHeight(Real height);
		Real GetWidth() const;
		Real GetHeight() const;
		void SetOrthogonalData(float left, float top, float right, float bottom);
		void SetName(const char* name);
		const char* GetName() const;
		void RefreshTransform();
		void Update(float dt);

		bool IsCulled(BoundingVolume* pBV) const;
		Ray ScreenPosToRay(long x, long y);
		Vec3 ScreenToNDC(const Vec2I& screenPos);
		Vec2I WorldToScreen(const Vec3& worldPos);
		void SetYZSwap(bool enable);
		void SetDistanceFromTarget(Real dist);
		void SetTarget(ISpatialObjectPtr pObj);
		ISpatialObjectPtr GetTarget() const;
		void SetTargetPos(const Vec3& pos);
		void SetCurrent(bool cur);
		bool GetCurrent() const;
		void SetCameraIndex(size_t idx);
		void SetEnalbeInput(bool enable);
		void SetInitialDistToTarget(Real dist);
		void SetMaxDistToTarget(Real dist);
		void SetMinDistToTarget(Real dist);
		void SetProportionalMove(bool enable);
		void ProcessInputData();
		const Frustum& GetFrustum();
		const Frustum& GetFrustumLocal();

		void SetOverridingCamera(CameraPtr cam);
		CameraPtr GetOverridingCamera() const;
		Real ComputePixelSizeAtDistance(Real distance);
		void SetRenderFrustum(bool renderFrustum);
		bool GetRenderFrustum() const;
		void RenderFrustum();		

		size_t ComputeHash() const OVERRIDE;

		//-------------------------------------------------------------------
		// InputConsumer From Renderer
		//-------------------------------------------------------------------
		void ConsumeInput(IInputInjectorPtr injector);
		
	protected:
		void UpdateFrustum();
		
	};
}

#endif //_Camera_Header_included_