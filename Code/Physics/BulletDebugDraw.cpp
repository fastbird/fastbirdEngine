#include <Physics/stdafx.h>
#include <Physics/BulletDebugDraw.h>
#include <Physics/IDebugDrawer.h>

using namespace fastbird;

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
	Error(warningString);
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