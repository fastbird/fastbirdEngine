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
#ifndef _Mat33_header_included_
#define _Mat33_header_included_

#include "Vec3.h"

namespace fb
{
	class Mat33
	{
	public:
	
		Real m[3][3];

		static const Mat33 IDENTITY;
		static const Mat33 ZERO;

		//-------------------------------------------------------------------
		Mat33();
		Mat33(Real m11, Real m12, Real m13,
			Real m21, Real m22, Real m23,
			Real m31, Real m32, Real m33);
		Mat33(const Vec3& x, const Vec3& y, const Vec3& z);

		//-------------------------------------------------------------------
		Mat33 operator-() const;
		Mat33 operator* (const Mat33& rMat) const;
		Vec3 operator* (const Vec3& rVec) const;
		Mat33 operator/ (Real k) const;
		void operator/=(Real k);
		Real* operator[](size_t iRow) const;
		bool operator==(const Mat33& other) const;

		//-------------------------------------------------------------------
		void MakeIdentity();		
		void FromAxisAngle(const Vec3& axis, Real radian);
		static Mat33 FromAxisAngleStatic(const Vec3& axis, Real radian);
		Mat33 Inverse() const;
		Mat33 Transpose() const;
		Mat33 ScaleAxis(const Vec3& scale) const;
		Vec3 Column(int index) const;
		void SetColumn(int index, const Vec3& v);
		bool IsSymmetric() const;
	};

	void write(std::ostream& stream, const Mat33& data);
	void read(std::istream& stream, Mat33& data);
}
#endif //_Mat33_header_included_