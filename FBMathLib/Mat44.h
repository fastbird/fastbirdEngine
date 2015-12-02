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

#include "Mat33.h"
#include "Vec3.h"
#include "Vec4.h"

namespace fb
{
	class Mat44
	{
	public:

		Real m[4][4];

		static const Mat44 IDENTITY;

		//-------------------------------------------------------------------------
		Mat44();
		Mat44(Real m11, Real m12, Real m13, Real m14,
			Real m21, Real m22, Real m23, Real m24,
			Real m31, Real m32, Real m33, Real m34,
			Real m41, Real m42, Real m43, Real m44);
		Mat44(const Mat33& mat33, const Vec3& translation);

		//-------------------------------------------------------------------------
		Vec4 operator*(const Vec4& columnVector) const;
		Vec3 operator*(const Vec3& columnVector) const;
		Mat44 operator* (const Mat44& other) const;
		Mat44& operator*=(const Mat44& other);
		bool operator==(const Mat44& other) const;
		Real* operator[](size_t iRow) const;

		//-------------------------------------------------------------------------
		 void MakeIdentity();
		 void MakeTranslation(const Vec3& pos);
		Mat44 Inverse() const;
		Mat44 InverseAffine() const;
		bool IsAffine(void) const;
		Mat44 Transpose(void) const;		
		const Vec4& Row(size_t iRow) const;		
		void SetTranslation(const Vec3& pos);
		Vec3 GetTranslation() const;
		Mat33 To33() const;
	};

#if defined(FB_DOUBLE_PRECISION)
	class Mat44f{
	public:
		float m[4][4];

		Mat44f();
		Mat44f(const Mat44& other);
		Mat44f& operator = (const Mat44& other);
		void MakeIdentity();
	};
#else
	typedef Mat44 Mat44f;
#endif
}