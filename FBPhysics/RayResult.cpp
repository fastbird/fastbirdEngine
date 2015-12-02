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
#include "RayResult.h"
using namespace fb;

RayResultClosest::RayResultClosest()
	: mRigidBody(0), mHitNormalWorld(0, 1, 0), mHitPointWorld(0, 0, 0), mIndex(-1)
{

}

RayResultClosest::RayResultClosest(RigidBody* body, const Vec3& hitPoint, const Vec3& normal, int index)
	:mRigidBody(body), mHitPointWorld(hitPoint), mHitNormalWorld(normal), mIndex(index)
{

}


RayResultWithObj::RayResultWithObj(RigidBody* body)
	: mTargetBody(body), mRigidBody(0), mHitNormalWorld(0, 1, 0), mHitPointWorld(0, 0, 0), mIndex(-1)
{

}

RayResultAll::RayResultAll()
	:mCurSize(0)
{

}

RayResultAll::~RayResultAll()
{
	for (unsigned i = 0; i < mCurSize; ++i)
	{
		mRayResults[i]->~RayResultClosest();
		free(mRayResults[i]);
	}
}

void RayResultAll::AddResult(RigidBody* rigidBody, const Vec3& hitPoint, const Vec3& hitNormal, int index)
{
	if (mCurSize >= SIZE)
		return;

	mRayResults[mCurSize++] = (RayResultClosest*)malloc(sizeof(RayResultClosest));

	new (mRayResults[mCurSize - 1]) RayResultClosest(rigidBody, hitPoint, hitNormal, index);
}