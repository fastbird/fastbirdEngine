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
namespace fb{
	class Ray;
	class BoundingVolume;
	class Mat44;
	class Transformation;
	class Frustum;
	class Vec3;
	FB_DECLARE_SMART_PTR(ISpatialObject);
	class ICamera{
	public:
		enum MatrixType {
			View,
			InverseView,
			InverseViewProj,
			Proj, // Projection
			ProjBeforeSwap,
			InverseProj,
			ViewProj,
			NumMatrices,
		};
		virtual const Mat44& GetMatrix(MatrixType type) = 0;		
		virtual const Transformation& GetTransformation() const = 0;
		virtual bool IsCulled(BoundingVolume* pBV) const = 0;
		virtual const Vec3& GetPosition() const = 0;
		virtual const Vec3 GetDirection() const = 0;
		virtual Real ComputePixelSizeAtDistance(Real distance) = 0;		
		/// Vertical fov
		virtual void SetFOV(Real radians) = 0;
		virtual Real GetFOV() const = 0;
		virtual Real GetAspectRatio() const = 0;
		virtual Real GetTanHalfFOV() const = 0;
		virtual const Frustum& GetFrustum() = 0;
		virtual const Frustum& GetFrustumLocal() = 0;
		virtual Real GetNear() const = 0;
		virtual Real GetFar() const = 0;
		virtual void GetNearFar(Real& nearPlane, Real& farPlane) const = 0;
		virtual void SetNearFar(Real nearPlane, Real farPlane) = 0;
		virtual void SetNear(Real n) = 0;
		virtual void SetFar(Real f) = 0;
		virtual Ray ScreenPosToRay(long x, long y) = 0;
		virtual Vec3 ScreenToNDC(const Vec2I& screenPos) = 0;		
		virtual void SetPosition(const Vec3& pos) = 0;
		virtual void SetDirection(const Vec3& dir) = 0;
		virtual void SetTarget(ISpatialObjectPtr pObj) = 0;
		virtual ISpatialObjectPtr GetTarget() const = 0;
		virtual void SetTargetPos(const Vec3& pos) = 0;
		virtual void SetMaxDistToTarget(Real dist) = 0;
		virtual void SetMinDistToTarget(Real dist) = 0;
		virtual void SetProportionalMove(bool enable) = 0;
		virtual size_t ComputeHash() const = 0;
		/*
		virtual Vec2I WorldToScreen(const Vec3& pos) = 0;
		
		virtual void GetNearFar(Real& nearPlane, Real& farPlane) const = 0;
		virtual Real GetFOV() const = 0;
		virtual void SetWidth(Real width) = 0;
		virtual void SetHeight(Real height) = 0;
		virtual Real GetWidth() const = 0;
		virtual Real GetHeight() const = 0;
		virtual void SetCurrent(bool cur) = 0;*/
	};
}