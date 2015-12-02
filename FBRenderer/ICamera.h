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
	class BoundingVolume;
	class Mat44;
	class ICamera{
	public:
		enum MatrixType {
			View,
			InverseView,
			InverseViewProj,
			Proj, // Projection
			InverseProj,
			ViewProj,
			NumMatrices,
		};
		virtual const Mat44& GetMatrix(MatrixType type) = 0;
		virtual const Transformation& GetTransformation() const = 0;
		virtual bool IsCulled(BoundingVolume* pBV) const = 0;
		virtual const Vec3& GetPosition() const = 0;
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