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
#include "BulletDebugDraw.h"
#include "IDebugDrawer.h"

using namespace fb;

BulletDebugDraw::BulletDebugDraw()
: mDrawer(0)
, mDebugMode(DBG_NoDebug)
{
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	if (mDrawer)
		mDrawer->DrawLine(BulletToFB(from), BulletToFB(to), BulletToFB(color));
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
{
	if (mDrawer)
		mDrawer->DrawLine(BulletToFB(from), BulletToFB(to), BulletToFB(fromColor), BulletToFB(toColor));
}

void BulletDebugDraw::drawSphere(const btVector3& p, btScalar radius, const btVector3& color)
{
	if (mDrawer)
		mDrawer->DrawSphere(BulletToFB(p), radius, BulletToFB(color));
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
	int lifeTime, const btVector3& color)
{
	btVector3 to = PointOnB + normalOnB * 1;//distance;
	const btVector3&from = PointOnB;
	drawLine(from, to, color);
}


void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
	Logger::Log(FB_ERROR_LOG_ARG, warningString);
}



void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
	if (mDrawer)
		mDrawer->Draw3DText(BulletToFB(location), textString);
}

void BulletDebugDraw::setDebugMode(int debugMode)
{
	mDebugMode = debugMode;
}

int BulletDebugDraw::getDebugMode() const
{
	return mDebugMode;
}


void BulletDebugDraw::SetCallback(IDebugDrawer* p)
{
	mDrawer = p;
}