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
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/level.hpp>
#include "Mat33.h"
#include "Vec3.h"
#include "Quat.h"
#include "Plane.h"
#include "Ray.h"

namespace fb
{
	class Frustum;
	class Plane;
	class Mat44;
	typedef std::tuple <
		// rotation
		Real, Real, Real,
		Real, Real, Real,
		Real, Real, Real,
		// quaternion
		Real, Real, Real, Real,
		// translation
		Real, Real, Real,
		// scale
		Real, Real, Real, 
		// itentity, RSSeperated, UniformScale
		bool, bool, bool> TransformationTuple;
	FB_DECLARE_SMART_PTR(Transformation);
	typedef std::vector<Transformation> Transformations;
	class Transformation
	{
	private:

		Mat33 mMat;
		Quat mR;
		Vec3 mT;
		Vec3 mS;
		bool mIdentity, mRSSeperated, mUniformScale;

	public:

		static const Transformation IDENTITY;

		static TransformationPtr Create();
		Transformation();
		~Transformation();
		Transformation(const Quat& q);
		Transformation(const Mat33& t);
		Transformation(const TransformationTuple& t);
		
		static Transformation FromScale(float x, float y, float z);
		static Transformation FromScale(const Vec3& scale);
		static Transformation FromRotation(const Quat& rot);
		static Transformation FromTranslation(float x, float y, float z);
		static Transformation FromTranslation(const Vec3& translation);

		void MakeIdentity ();
		void MakeUnitScale ();
		bool IsIdentity () const{
			return mIdentity;
		}
		bool IsRSSeperated() const{
			return mRSSeperated;
		}
		bool IsUniformScale () const{
			return mRSSeperated && mUniformScale;
		}

		void SetRotation (const Mat33& r);
		void SetRotation (const Quat& r);
		void SetDirection(const Vec3& dir);
		void SetDirectionAndRight(const Vec3& dir, const Vec3& right);
		void AddRotation (const Quat& addR);
		const Quat& GetRotation() const;
		void SetMatrix (const Mat33& mat);
		const Mat33& GetMatrix() const;
		void SetTranslation(const Vec3& t);
		void AddTranslation(const Vec3& addT);
		const Vec3& GetTranslation() const;
		void SetScale (const Vec3& s);
		void AddScale(const Vec3& s);
		const Vec3& GetScale() const;
		void SetUniformScale (Real fScale);
		Real GetUniformScale() const;

		Real GetNorm () const;
		Vec3 ApplyForward (const Vec3& p) const;
		Vec3 ApplyForwardDir(const Vec3& dir) const;
		void ApplyForward (int iQuantity, const Vec3* points, Vec3* output) const;
		Plane ApplyForward(const Plane& p) const;
		AABB ApplyForward(const AABB& aabb) const;
		Ray ApplyForward(const Ray& r, bool scaleLength) const;
		Frustum ApplyForward(const Frustum& f) const;

		// X = M^{-1}*(Y-T) where Y is the input and X is the output.
		Vec3 ApplyInverse (const Vec3& p) const;
		Vec3 ApplyInverseDir(const Vec3& dir) const;
		void ApplyInverse (int iQuantity, const Vec3* points, Vec3* output) const;
		Ray ApplyInverse(const Ray& r, bool scaleLength) const;		
		Plane ApplyInverse(const Plane& p) const;
		Frustum ApplyInverse(const Frustum& f) const;

		Vec3 InvertVector (const Vec3& v) const;

		void Product (const Transformation& a, const Transformation& b);
		Transformation operator* (const Transformation& t) const;			
		
		void Inverse (Transformation& t) const;
		Transformation Inverse() const;
		void GetHomogeneous (Mat44& hm) const;
#if defined(FB_DOUBLE_PRECISION)
		void GetHomogeneous(Mat44f& hm) const;
#endif

		Vec3 GetRight() const;
		Vec3 GetForward() const;
		/// Same with GetFoward()
		Vec3 GetUp() const;	

		bool operator==(const Transformation& other) const;	
		bool operator!=(const Transformation& other) const;
		operator TransformationTuple() const;

		// for serialization
		const Mat33& _GetMat33() { return mMat; }
		const Quat& _GetQuat() { return mR; }
		const Vec3& _GetT() { return mT; }
		const Vec3& _GetS() { return mS; }
		bool _GetIdentity() { return mIdentity; }
		bool _GetRSSeperated() { return mRSSeperated; }
		bool _GetUniformScale() { return mUniformScale; }		

		size_t ComputeHash() const;
	};
}

BOOST_CLASS_IMPLEMENTATION(fb::Transformation, boost::serialization::primitive_type);