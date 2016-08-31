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
#include "FBMathLib/Vec3.h"

namespace fb {
	FB_DECLARE_SMART_PTR(Intersection);
	class Intersection {
	protected:
		Vec3 mIntersectionPoint{};
		Vec3 mNormal{};
		Real mIntersectionLength=0.f;		
		bool mIsTangent=false;		
		void* mUserData=0;		

	public:
		/// Unknown intersection point.
		/// Use this when collision occured but the exact point is unknown.
		static const Intersection DummyIntersection;
		// usually use this
		typedef std::vector<Intersection> Array;
			

		Intersection();
		Intersection(const Vec3& intersectionPoint, bool isTangent);
		Intersection(const Vec3& intersectionPoint, const Vec3& normal);
		Intersection(const Vec3& intersectionPoint, Real intersectionLength, bool isTangent);
		/// pointer is not managed by this class.
		void SetUserData(void* data);
		void* GetUserData() const;

		Vec3 GetIntersectionPoint() const;
		void SetIntersectionPoint(const Vec3& intersectionPoint);

		bool IsTangent() const;
		void SetTangent(bool tangent);

		Real GetIntersectionLength() const;
		/// returning the merged list of intersections, sorted by increasing distance 
		/// from the reference point.
		static Array sort(const Vec3& refPoint, const Array& listA, const Array& listB);
		bool operator == (const Intersection& other) const;
		std::string ToString() const;
	};
}