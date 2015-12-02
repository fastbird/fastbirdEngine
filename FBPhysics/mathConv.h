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
#include "FBMathLib/Math.h"
namespace fb
{
	inline btQuaternion FBToBullet(const Quat& rot)
	{
		return btQuaternion(rot.x, rot.y, rot.z, rot.w);
	}

	inline btVector3 FBToBullet(const Vec3& vec)
	{
		return btVector3(vec.x, vec.y, vec.z);
	}

	inline btTransform FBToBullet(const Mat44& mat)
	{
		float m[16];
		m[0] = mat[0][0];
		m[1] = mat[1][0];
		m[2] = mat[2][0];
		m[3] = mat[3][0];

		m[4] = mat[0][1];
		m[5] = mat[1][1];
		m[6] = mat[2][1];
		m[7] = mat[3][1];

		m[8] = mat[0][2];
		m[9] = mat[1][2];
		m[10] = mat[2][2];
		m[11] = mat[3][2];

		m[12] = mat[0][3];
		m[13] = mat[1][3];
		m[14] = mat[2][3];
		m[15] = mat[3][3];
		btTransform bt;
		bt.setFromOpenGLMatrix(m);

		return bt;
	}

	inline void BulletToFB(const btVector3& bv, Vec3& dest)
	{
		dest.x = bv.x();
		dest.y = bv.y();
		dest.z = bv.z();
	}

	inline void BulletToFB(const btTransform& bt, Vec3& dest)
	{
		BulletToFB(bt.getOrigin(), dest);
	}

	inline Vec3 BulletToFB(const btVector3& bv)
	{
		return Vec3(bv.x(), bv.y(), bv.z());
	}

	inline void BulletToFB(const btQuaternion& bq, Quat& dest)
	{
		dest.x = bq.x();
		dest.y = bq.y();
		dest.z = bq.z();
		dest.w = bq.w();
	}
	
	inline Quat BulletToFB(const btQuaternion& bq)
	{
		return Quat(bq.w(), bq.x(), bq.y(), bq.z());
		
	}

	inline void BulletToFB(const btTransform& bt, Quat& rot)
	{
		btQuaternion btRot = bt.getRotation();
		BulletToFB(btRot, rot);
	}

	inline void BulletToFB(const btTransform& bt, Mat44& mat)
	{
		float m[16];
		bt.getOpenGLMatrix(m);
		mat[0][0] = m[0];
		mat[1][0] = m[1];
		mat[2][0] = m[2];
		mat[3][0] = m[3];

		mat[0][1] = m[4];
		mat[1][1] = m[5];
		mat[2][1] = m[6];
		mat[3][1] = m[7];

		mat[0][2] = m[8];
		mat[1][2] = m[9];
		mat[2][2] = m[10];
		mat[3][2] = m[11];

		mat[0][3] = m[12];
		mat[1][3] = m[13];
		mat[2][3] = m[14];
		mat[3][3] = m[15];
	}

	inline void BulletToFB(const btTransform& bt, Transformation& mat)
	{
		Quat rot;
		BulletToFB(bt, rot);

		Vec3 pos;
		BulletToFB(bt, pos);

		mat.MakeIdentity();
		mat.SetRotation(rot);
		mat.SetTranslation(pos);
	}
}