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

#include "stdafx.h"
#include "Transformation.h"
#include "Math.h"
#include "Frustum.h"

namespace fb
{

	//----------------------------------------------------------------------------
	const Transformation Transformation::IDENTITY;

	//----------------------------------------------------------------------------
	TransformationPtr Transformation::Create() {
		return TransformationPtr(new Transformation, [](Transformation* obj) { delete obj; });
	}

	Transformation::Transformation()
		: mMat(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)
		, mT(0.0f, 0.0f, 0.0f)
		, mS(1.0f, 1.0f, 1.0f)
	{
		// default transform is the identity
		mIdentity = true;
		mRSSeperated = true;
		mUniformScale = true;
	}

	//----------------------------------------------------------------------------
	Transformation::~Transformation()
	{
	}

	Transformation::Transformation(const Quat& q)
		:Transformation()
	{
		SetRotation(q);
	}

	Transformation::Transformation(const Mat33& t)
		: Transformation()
	{
		SetRotation(t);

	}

	Transformation::Transformation(const TransformationTuple& t)
		: mMat(std::get<0>(t), std::get<1>(t), std::get<2>(t),
			std::get<3>(t), std::get<4>(t), std::get<5>(t),
			std::get<6>(t), std::get<7>(t), std::get<8>(t))
		, mR(std::get<9>(t), std::get<10>(t), std::get<11>(t), std::get<12>(t))
		, mT(std::get<13>(t), std::get<14>(t), std::get<15>(t))
		, mS(std::get<16>(t), std::get<17>(t), std::get<18>(t))
		, mIdentity(std::get<19>(t))
		, mRSSeperated(std::get<20>(t))
		, mUniformScale(std::get<21>(t))
	{
	}

	Transformation Transformation::FromScale(float x, float y, float z) {
		return FromScale(Vec3(x, y, z));
	}

	Transformation Transformation::FromScale(const Vec3& scale) {
		Transformation ret;
		ret.SetScale(scale);
		return ret;
	}

	Transformation Transformation::FromRotation(const Quat& rot) {
		Transformation ret;
		ret.SetRotation(rot);
		return ret;
	}
	Transformation Transformation::FromTranslation(float x, float y, float z) {
		return FromTranslation(Vec3(x, y, z));
	}
	Transformation Transformation::FromTranslation(const Vec3& translation)	{
		Transformation ret;
		ret.SetTranslation(translation);
		return ret;
	}

	//----------------------------------------------------------------------------
	void Transformation::MakeIdentity()
	{
		mMat = Mat33::IDENTITY;
		mT = Vec3::ZERO;
		mS = Vec3(1.0f, 1.0f, 1.0f);
		mIdentity = true;
		mRSSeperated = true;
		mUniformScale = true;
	}

	//----------------------------------------------------------------------------
	void Transformation::MakeUnitScale()
	{
		assert(mRSSeperated);

		mS = Vec3(1.0f, 1.0f, 1.0f);
		mUniformScale = true;
	}

	//----------------------------------------------------------------------------
	Real Transformation::GetNorm() const
	{
		if (mRSSeperated)
		{
			Real fMax = abs(mS.x);
			if (abs(mS.y) > fMax)
			{
				fMax = abs(mS.y);
			}
			if (abs(mS.z) > fMax)
			{
				fMax = abs(mS.z);
			}
			return fMax;
		}

		// approximation of the maximum scale.
		Real fMaxRowSum =
			abs(mMat[0][0]) +
			abs(mMat[0][1]) +
			abs(mMat[0][2]);

		Real fRowSum =
			abs(mMat[1][0]) +
			abs(mMat[1][1]) +
			abs(mMat[1][2]);

		if (fRowSum > fMaxRowSum)
			fMaxRowSum = fRowSum;

		fRowSum =
			abs(mMat[2][0]) +
			abs(mMat[2][1]) +
			abs(mMat[2][2]);

		if (fRowSum > fMaxRowSum)
		{
			fMaxRowSum = fRowSum;
		}

		return fMaxRowSum;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetRotation(const Mat33& r)
	{
		mMat = r;
		mR.FromRotationMatrix(r);
		mIdentity = false;
		mRSSeperated = true;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetRotation(const Quat& r)
	{
		mR = r;
		r.ToRotationMatrix(mMat);
		mIdentity = false;
		mRSSeperated = true;
	}

	void Transformation::SetDirection(const Vec3& dir)
	{
		Vec3 forward = dir;
		Vec3 right;
		if (forward == Vec3::UNIT_Z || forward == -Vec3::UNIT_Z)
		{
			right = Vec3::UNIT_X;
		}
		else
		{
			right = forward.Cross(Vec3::UNIT_Z);
		}
		Vec3 up = right.Cross(forward);


		right.Normalize();
		up.Normalize();

		Mat33 rot;
		rot.SetColumn(0, right);
		rot.SetColumn(1, forward);
		rot.SetColumn(2, up);
		SetRotation(rot);
	}

	void Transformation::SetDirectionAndRight(const Vec3& dir, const Vec3& right)
	{
		Vec3 forward = dir;
		Vec3 up = right.Cross(forward);

		up.Normalize();

		Mat33 rot;
		rot.SetColumn(0, right);
		rot.SetColumn(1, forward);
		rot.SetColumn(2, up);
		SetRotation(rot);
	}

	//----------------------------------------------------------------------------
	void Transformation::AddRotation(const Quat& addR)
	{
		Quat ret = addR * mR;
		SetRotation(ret);
	}

	const Quat& Transformation::GetRotation() const {
		assert(mRSSeperated);
		return mR;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetTranslation(const Vec3& t)
	{
		mT = t;
		mIdentity = false;
	}

	void Transformation::AddTranslation(const Vec3& addT)
	{
		mT += addT;
		mIdentity = false;
	}

	const Vec3& Transformation::GetTranslation() const {
		return mT;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetScale(const Vec3& s)
	{
		assert(mRSSeperated && s.x != 0.0f && s.y != 0.0f
			&& s.z != 0.0f);

		Vec3 fixedScale = FixPrecisionScaleVector(s);
		mS = fixedScale;

		mIdentity = false;
		if (fixedScale.x == fixedScale.y && fixedScale.x == fixedScale.z)
			mUniformScale = true;
		else
			mUniformScale = false;
	}

	void Transformation::AddScale(const Vec3& s)
	{
		assert(mRSSeperated && s.x != 0.0f && s.y != 0.0f
			&& s.z != 0.0f);

		Vec3 fixedScale = FixPrecisionScaleVector(s);
		mS *= fixedScale;
		mIdentity = false;
		if (fixedScale.x == fixedScale.y && fixedScale.x == fixedScale.z)
			mUniformScale = true;
		else
			mUniformScale = false;
	}

	const Vec3& Transformation::GetScale() const {
		assert(mRSSeperated);
		return mS;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetUniformScale(Real fScale)
	{
		assert(mRSSeperated && fScale != 0.0f);
		mS = Vec3(fScale, fScale, fScale);
		mIdentity = false;
		mUniformScale = true;
	}

	Real Transformation::GetUniformScale() const {
		assert(mRSSeperated && mUniformScale);
		return mS.x;
	}

	//----------------------------------------------------------------------------
	void Transformation::SetMatrix(const Mat33& mat)
	{
		mMat = mat;
		mIdentity = false;
		mRSSeperated = false;
		mUniformScale = false;
	}

	const Mat33& Transformation::GetMatrix() const {
		assert(mRSSeperated);
		return mMat;
	}

	//----------------------------------------------------------------------------
	Vec3 Transformation::ApplyForward(const Vec3& p) const
	{
		if (mIdentity)
		{
			// Y = X
			return p;
		}

		if (mRSSeperated)
		{
			// Y = R*S*X + T
			Vec3 kOutput(mS.x*p.x, mS.y*p.y,
				mS.z*p.z);
			kOutput = mMat*kOutput + mT;
			return kOutput;
		}

		// Y = M*X + T
		Vec3 kOutput = mMat*p + mT;
		return kOutput;
	}

	Vec3 Transformation::ApplyForwardDir(const Vec3& dir) const
	{
		if (mIdentity)
			return dir;

		if (mRSSeperated)
		{
			return mMat * dir;			
		}
		else {
			// R*S* Y
			// inverse transposes
			// Y * (R*S)^-1			
			return dir * mMat.Inverse();
		}
	}

	//----------------------------------------------------------------------------
	void Transformation::ApplyForward(int iQuantity, const Vec3* points,
		Vec3* outputs) const
	{
		if (mIdentity)
		{
			// Y = X
			size_t uiSize = iQuantity*sizeof(Vec3);
			memcpy(outputs, points, uiSize);
		}
		else
		{
			int i;
			if (mRSSeperated)
			{
				// Y = R*S*X + T
				for (i = 0; i < iQuantity; i++)
				{
					outputs[i].x = mS.x*points[i].x;
					outputs[i].y = mS.y*points[i].y;
					outputs[i].z = mS.z*points[i].z;
					outputs[i] = mMat*outputs[i] + mT;
				}
			}
			else
			{
				// Y = M*X + T
				for (i = 0; i < iQuantity; i++)
				{
					outputs[i] = mMat*points[i] + mT;
				}
			}
		}
	}

	AABB Transformation::ApplyForward(const AABB& aabb) const
	{
		Vec3 m2 = ApplyForward(aabb.GetMin());
		Vec3 x2 = ApplyForward(aabb.GetMax());

		Vec3 newMin(std::min(m2.x, x2.x), std::min(m2.y, x2.y), std::min(m2.z, x2.z));
		Vec3 newMax(std::max(m2.x, x2.x), std::max(m2.y, x2.y), std::max(m2.z, x2.z));
		AABB newAABB;
		newAABB.SetMin(newMin);
		newAABB.SetMax(newMax);
		return newAABB;
	}

	Ray Transformation::ApplyForward(const Ray& r, bool scaleLength) const
	{
		if (mIdentity)		
			return r;

		if (!scaleLength) {
			auto newPos = ApplyForward(r.GetOrigin());
			auto newDir = ApplyForwardDir(r.GetDir());
			return Ray(newPos, newDir);
		}
		else {
			auto newPosA = ApplyForward(r.GetOrigin());
			auto newPosB = ApplyForward(r.GetPointB());
			return Ray::FromSegment(newPosA, newPosB);
		}
	}

	Frustum Transformation::ApplyForward(const Frustum& f) const {
		Frustum ret = f;
		ret.mOrigin = ApplyForward(f.mOrigin);
		ret.mCenter= ApplyForward(f.mCenter);
		
		for (int i = 0; i < Frustum::NumPlanes; ++i) {
			ret.mPlanes[i] = ApplyForward(f.mPlanes[i]);
		}
		auto forward = ret.mPlanes[Frustum::FRUSTUM_PLANE_NEAR].mNormal;
		auto rightTemp = ret.mPlanes[Frustum::FRUSTUM_PLANE_LEFT].mNormal;
		auto up = forward.Cross(rightTemp);
		up.Normalize();
		auto right = forward.Cross(up);
		ret.mOrientation = Quat(right, forward, up);

		return ret;
	}

	//----------------------------------------------------------------------------
	Plane Transformation::ApplyForward(const Plane& inputPlane) const
	{
		if (mIdentity)
		{
			return inputPlane;
		}

		Plane output;
		Vec3 pointInPlane = inputPlane.mNormal * inputPlane.mConstant;
		auto newPointInPlane = ApplyForward(pointInPlane);
		output.mNormal = ApplyForwardDir(inputPlane.mNormal);
		output.mConstant = output.mNormal.Dot(newPointInPlane);

		return output;		
	}

	//----------------------------------------------------------------------------
	Vec3 Transformation::ApplyInverse(const Vec3& p) const
	{
		if (mIdentity)
		{
			// X = Y
			return p;
		}

		Vec3 outputPoint = p - mT;
		if (mRSSeperated)
		{
			// X = S^{-1}*R^t*(Y - T)
			outputPoint = outputPoint*mMat;
			if (mUniformScale)
			{
				outputPoint /= GetUniformScale();
			}
			else
			{
				outputPoint /= mS;
			}
		}
		else
		{
			// X = M^{-1}*(Y - T)
			outputPoint = mMat.Inverse()*outputPoint;
		}

		return outputPoint;
	}

	Vec3 Transformation::ApplyInverseDir(const Vec3& dir) const
	{
		if (mIdentity)
		{
			return dir;
		}
		
		if (mRSSeperated)
		{
			// X = S^{-1} * R^t * Y
			if (mUniformScale)
			{
				// (RS)^-1
				// S^-1 * R^-1
				return dir * mMat / GetUniformScale();
			}
			else
			{
				// (((RS)^-1)^t)^-1
				// (((RS)^-1)^-1)^t
				// (RS)^t
				// S^t * R^t
				return dir * mMat * mS;
			}
		}
		else
		{			
			// (((RS)^-1)^t)^-1
			// (((RS)^-1)^-1)^t
			// (RS)^t
			return dir * mMat;
		}
	}

	//----------------------------------------------------------------------------
	void Transformation::ApplyInverse(int iQuantity, const Vec3* points,
		Vec3* outputs) const
	{
		if (mIdentity)
		{
			// X = Y
			size_t uiSize = iQuantity*sizeof(Vec3);
			memcpy(outputs, points, uiSize);
			return;
		}

		Vec3 kDiff;
		int i;
		if (mRSSeperated)
		{
			// X = S^{-1}*R^t*(Y - T)
			if (mUniformScale)
			{
				Real fInvScale = 1.0f / GetUniformScale();
				for (i = 0; i < iQuantity; i++)
				{
					kDiff = points[i] - mT;
					outputs[i] = fInvScale*(kDiff*mMat);
				}
			}
			else
			{
				for (i = 0; i < iQuantity; i++)
				{
					kDiff = points[i] - mT;
					outputs[i] = kDiff*mMat;
					outputs[i] /= mS;
				}
			}
		}
		else
		{
			// X = M^{-1}*(Y - T)
			Mat33 kInverse = mMat.Inverse();
			for (i = 0; i < iQuantity; i++)
			{
				kDiff = points[i] - mT;
				outputs[i] = kInverse*kDiff;
			}
		}
	}

	//----------------------------------------------------------------------------
	Ray Transformation::ApplyInverse(const Ray& r, bool scaleLength) const
	{
		if (mIdentity)		
			return r;
		if (!scaleLength) {
			return Ray(ApplyInverse(r.GetOrigin()), ApplyInverseDir(r.GetDirection()));
		}
		else {
			return Ray::FromSegment(ApplyInverse(r.GetOrigin()), ApplyInverseDir(r.GetPointB()));
		}
	}

	Plane Transformation::ApplyInverse(const Plane& inputPlane) const {
		if (mIdentity)
		{
			return inputPlane;
		}
		Plane output;
		Vec3 pointInPlane = inputPlane.mNormal * inputPlane.mConstant;
		auto newPointInPlane = ApplyInverse(pointInPlane);
		output.mNormal = ApplyInverseDir(inputPlane.mNormal);
		output.mConstant = output.mNormal.Dot(newPointInPlane);

		return output;
	}

	Frustum Transformation::ApplyInverse(const Frustum& f) const {
		Frustum ret = f;
		ret.mOrigin = ApplyInverse(f.mOrigin);
		ret.mCenter = ApplyInverse(f.mCenter);

		for (int i = 0; i < Frustum::NumPlanes; ++i) {
			ret.mPlanes[i] = ApplyInverse(f.mPlanes[i]);
		}
		auto forward = ret.mPlanes[Frustum::FRUSTUM_PLANE_NEAR].mNormal;
		auto rightTemp = ret.mPlanes[Frustum::FRUSTUM_PLANE_LEFT].mNormal;
		auto up = forward.Cross(rightTemp);
		up.Normalize();
		auto right = forward.Cross(up);
		ret.mOrientation = Quat(right, forward, up);

		return ret;
	}

	//----------------------------------------------------------------------------
	Vec3 Transformation::InvertVector(const Vec3& inputVector) const
	{
		if (mIdentity)
		{
			// X = Y
			return inputVector;
		}

		Vec3 outputVector;
		if (mRSSeperated)
		{
			// X = S^{-1}*R^t*Y
			outputVector = inputVector*mMat;
			if (mUniformScale)
			{
				outputVector /= GetUniformScale();
			}
			else
			{
				outputVector /= mS;
			}
		}
		else
		{
			// X = M^{-1}*Y
			outputVector = mMat.Inverse()*inputVector;
		}

		return outputVector;
	}

	//----------------------------------------------------------------------------
	void Transformation::Product(const Transformation& rkA,
		const Transformation& rkB)
	{
		if (rkA.IsIdentity())
		{
			*this = rkB;
			return;
		}

		if (rkB.IsIdentity())
		{
			*this = rkA;
			return;
		}

		if (rkA.mRSSeperated && rkB.mRSSeperated)
		{
			if (rkA.mUniformScale)
			{
				SetRotation(rkA.mR*rkB.mR);

				SetTranslation(rkA.GetUniformScale()*(
					rkA.mMat*rkB.mT) + rkA.mT);

				if (rkB.IsUniformScale())
				{
					SetUniformScale(rkA.GetUniformScale()*rkB.GetUniformScale());
				}
				else
				{
					SetScale(rkA.GetUniformScale()*rkB.GetScale());
				}

				return;
			}
		}

		// In all remaining cases, the matrix cannot be written as R*S*X+T.
		Mat33 kMA = (rkA.mRSSeperated ?
			rkA.mMat.ScaleAxis(rkA.mS) :
			rkA.mMat);

		Mat33 kMB = (rkB.mRSSeperated ?
			rkB.mMat.ScaleAxis(rkB.mS) :
			rkB.mMat);

		SetMatrix(kMA*kMB);
		SetTranslation(kMA*rkB.mT + rkA.mT);
	}

	//----------------------------------------------------------------------------
	Transformation Transformation::operator* (const Transformation& t) const
	{
		Transformation result;
		result.Product(*this, t);
		return result;
	}

	//----------------------------------------------------------------------------
	void Transformation::Inverse(Transformation& rkInverse) const
	{
		if (mIdentity)
		{
			rkInverse = *this;
			return;
		}

		if (mRSSeperated)
		{
			if (mUniformScale)
			{
				rkInverse.mMat = GetMatrix().Transpose() / GetUniformScale();
			}
			else
			{
				Mat33 kRS = mMat.ScaleAxis(mS);
				rkInverse.mMat = kRS.Inverse();
			}
		}
		else
		{
			rkInverse.mMat = mMat.Inverse();
		}

		rkInverse.mT = -(rkInverse.mMat*mT);
		if (!mRSSeperated)
		{
			rkInverse.mRSSeperated = false;
		}
		else
		{
			rkInverse.mR = Quat(rkInverse.mMat);
			if (mS.x != 1.0f)
			{
				rkInverse.mR.Normalise();
			}
		}
		if (!mUniformScale)
			rkInverse.mUniformScale = false;

		rkInverse.mIdentity = false;
	}

	Transformation Transformation::Inverse() const
	{
		Transformation rkInverse;

		if (mIdentity)
		{
			rkInverse = *this;
			return rkInverse;
		}

		if (mRSSeperated)
		{
			if (mUniformScale)
			{
				rkInverse.mMat = GetMatrix().Transpose() / GetUniformScale();
			}
			else
			{
				Mat33 kRS = mMat.ScaleAxis(mS);
				rkInverse.mMat = kRS.Inverse();
			}
		}
		else
		{
			rkInverse.mMat = mMat.Inverse();
		}

		rkInverse.mT = -(rkInverse.mMat*mT);
		if (!mRSSeperated)
		{
			rkInverse.mRSSeperated = false;
		}
		else
		{
			rkInverse.mR = Quat(rkInverse.mMat);
			if (mS.x != 1.0f)
			{
				rkInverse.mR.Normalise();
			}
		}
		if (!mUniformScale)
			rkInverse.mUniformScale = false;

		rkInverse.mIdentity = false;

		return rkInverse;
	}

	//----------------------------------------------------------------------------
	void Transformation::GetHomogeneous(Mat44& rkHMatrix) const
	{
		if (mRSSeperated)
		{
			rkHMatrix[0][0] = mS[0] * mMat[0][0];
			rkHMatrix[1][0] = mS[0] * mMat[1][0];
			rkHMatrix[2][0] = mS[0] * mMat[2][0];
			rkHMatrix[3][0] = 0.0f;

			rkHMatrix[0][1] = mS[1] * mMat[0][1];
			rkHMatrix[1][1] = mS[1] * mMat[1][1];
			rkHMatrix[2][1] = mS[1] * mMat[2][1];
			rkHMatrix[3][1] = 0.0f;

			rkHMatrix[0][2] = mS[2] * mMat[0][2];
			rkHMatrix[1][2] = mS[2] * mMat[1][2];
			rkHMatrix[2][2] = mS[2] * mMat[2][2];
			rkHMatrix[3][2] = 0.0f;
		}
		else
		{
			rkHMatrix[0][0] = mMat[0][0];
			rkHMatrix[1][0] = mMat[1][0];
			rkHMatrix[2][0] = mMat[2][0];
			rkHMatrix[3][0] = 0.0f;
			rkHMatrix[0][1] = mMat[0][1];
			rkHMatrix[1][1] = mMat[1][1];
			rkHMatrix[2][1] = mMat[2][1];
			rkHMatrix[3][1] = 0.0f;
			rkHMatrix[0][2] = mMat[0][2];
			rkHMatrix[1][2] = mMat[1][2];
			rkHMatrix[2][2] = mMat[2][2];
			rkHMatrix[3][2] = 0.0f;
		}

		rkHMatrix[0][3] = mT[0];
		rkHMatrix[1][3] = mT[1];
		rkHMatrix[2][3] = mT[2];
		rkHMatrix[3][3] = 1.0f;
	}
#if defined(FB_DOUBLE_PRECISION)
	void Transformation::GetHomogeneous(Mat44f& hm) const {
		Mat44 homo;
		GetHomogeneous(homo);
		hm = homo;
	}
#endif

	Vec3 Transformation::GetRight() const
	{
		return mMat.Column(0);
	}
	Vec3 Transformation::GetForward() const
	{
		return mMat.Column(1);
	}
	Vec3 Transformation::GetUp() const
	{
		return mMat.Column(2);
	}

	bool Transformation::operator==(const Transformation& other) const
	{
		if (mRSSeperated)
		{
			return mR == other.mR && mS == other.mS && mT == other.mT;
		}
		else
		{
			return mMat == other.mMat && mT == other.mT;
		}
	}

	bool Transformation::operator!=(const Transformation& other) const {
		return !operator==(other);
	}

	Transformation::operator TransformationTuple() const {
		return std::make_tuple(
			mMat[0][0], mMat[0][1], mMat[0][2],
			mMat[1][0], mMat[1][1], mMat[1][2],
			mMat[2][0], mMat[2][1], mMat[2][2],
			mR.w, mR.x, mR.y, mR.z,
			mT.x, mT.y, mT.z,
			mS.x, mS.y, mS.y,
			mIdentity, mRSSeperated, mUniformScale);
	}

	void write(std::ostream& stream, const Transformation& data) {
		write(stream, data.mMat);
		write(stream, data.mR);
		write(stream, data.mT);
		write(stream, data.mS);		
		stream.write((char*)&data.mIdentity, sizeof(data.mIdentity));
		stream.write((char*)&data.mRSSeperated, sizeof(data.mRSSeperated));
		stream.write((char*)&data.mUniformScale, sizeof(data.mUniformScale));
	}

	void read(std::istream& stream, Transformation& data) {
		read(stream, data.mMat);
		read(stream, data.mR);
		read(stream, data.mT);
		read(stream, data.mS);
		stream.read((char*)&data.mIdentity, sizeof(data.mIdentity));
		stream.read((char*)&data.mRSSeperated, sizeof(data.mRSSeperated));
		stream.read((char*)&data.mUniformScale, sizeof(data.mUniformScale));
	}

}
