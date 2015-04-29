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
		VectorMap<btCollisionShape*, float> mColShapePendingDelete;

		// private functions
		friend class RigidBody;
		friend class RigidBodyImpl;
		btCollisionShape* CreateColShape(IPhysicsInterface* shapeProvider);
		btCollisionShape* CreateColShape(const std::vector<CollisionShape*>& shapes);
		RigidBody* _CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj);
		BulletDebugDraw mDebugDrawer;

		//-------------------------------------------------------------------
		// Private Functions
		//-------------------------------------------------------------------
		btCollisionShape* CreateBulletColShape(CollisionShape* colShape);

		int mRayGroup;

	public:
		Physics::Physics();
		Physics::~Physics();
		virtual void Initilaize();
		virtual void Deinitilaize();
		virtual void Update(float dt);
		// internal only.
		void _ReportCollisions();
		void _CheckCollisionShapeForDel(float timeStep);
		btDynamicsWorld* _GetDynamicWorld() const { return mDynamicsWorld; }

		virtual RigidBody* CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj);
		virtual RigidBody* CreateRigidBody(IPhysicsInterface* obj);
		virtual RigidBody* CreateTempRigidBody(CollisionShape* colShape);
		virtual RigidBody* CreateTempRigidBody(const std::vector<CollisionShape*>& colShape);

		virtual void DeleteRigidBody(RigidBody* rigidBody);
		btCollisionShape* ParseCollisionFile(const char* collisionFile);
		
		virtual void RemoveConstraint(btTypedConstraint* constraint);

		virtual void AddRef(btCollisionShape* colShape);
		virtual void Release(btCollisionShape* colShape);
		virtual void SetDebugDrawer(IDebugDrawer* debugDrawer);
		virtual void SetDebugMode(int debugMode);

		
		virtual void AttachBodies(const std::vector<RigidBody*>& bodies);

		virtual void SetRayCollisionGroup(int group);
		virtual bool RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultClosest& result);
		virtual bool RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, RayResultWithObj& result);
		virtual bool RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultAll& result);
		virtual void GetAABBOverlaps(const AABB& aabb, unsigned colMask, unsigned limit, std::vector<void*>& ret, RigidBody* except);
		virtual float GetDistanceBetween(RigidBody* a, RigidBody* b);
	};
}