#pragma once
namespace fastbird
{
	class IDebugDrawer;

	class BulletDebugDraw : public btIDebugDraw
	{
		int mDebugMode;
		IDebugDrawer* mDrawer;

	public:
		BulletDebugDraw();

		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
		virtual void	drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
		virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
			int lifeTime, const btVector3& color);

		virtual void reportErrorWarning(const char* warningString);
		virtual void draw3dText(const btVector3& location, const char* textString);
		virtual void setDebugMode(int debugMode);
		virtual int getDebugMode() const;

		void SetCallback(IDebugDrawer* p);
	};
}