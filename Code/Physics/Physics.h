#pragma once
#include <Physics/IPhysics.h>
#include <Physics/BulletDebugDraw.h>

namespace fastbird
{
	struct FBFilterCallback;
	class BulletDebugDraw;
	class Physics : public IPhysics
	{
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btBroadphaseInterface*	mBroadphase;
		btCollisionDispatcher*	mDispatcher;
		btConstraintSolver*	mSolver;
		btDiscreteDynamicsWorld* mDynamicsWorld;

		VectorMap<std::string, btCollisionShape*> mColShapes;
		VectorMap<btCollisionShape*, unsigned> mColShapesRefs;

		// private functions
		friend class RigidBody;
		friend class RigidBodyImpl;
		btCollisionShape* CreateColShape(IPhysicsInterface* shapeProvider);
		RigidBody* _CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj);
		BulletDebugDraw mDebugDrawer;

		//-------------------------------------------------------------------
		// Private Functions
		//-------------------------------------------------------------------
		void ReportCollisions();
		btCollisionShape* CreateBulletColShape(CollisionShape* colShape);

	public:
		Physics::~Physics();
		virtual void Initilaize();
		virtual void Deinitilaize();
		virtual void Update(float dt);
		virtual RigidBody* CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj);
		virtual RigidBody* CreateRigidBody(IPhysicsInterface* obj, float mass);
		virtual RigidBody* CreateRigidBody(CollisionShape* colShape, IPhysicsInterface* obj, float mass);
		virtual void DeleteRigidBody(RigidBody* rigidBody);
		btCollisionShape* ParseCollisionFile(const char* collisionFile);
		
		virtual void RemoveConstraint(btTypedConstraint* constraint);

		virtual void AddRef(btCollisionShape* colShape);
		virtual void Release(btCollisionShape* colShape);
		virtual void SetDebugDrawer(IDebugDrawer* debugDrawer);
		virtual void SetDebugMode(int debugMode);

		virtual void AttachBodies(const std::vector<RigidBody*>& bodies);
		virtual bool RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultClosest& result);
		virtual bool RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, RayResultWithObj& result);
		virtual bool RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultAll& result);
		virtual void GetAABBOverlaps(const AABB& aabb, unsigned colMask, unsigned limit, std::vector<void*>& ret, RigidBody* except);
	};
}