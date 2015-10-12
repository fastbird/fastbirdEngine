#pragma once
#include <Physics/IPhysics.h>
#include <Physics/BulletDebugDraw.h>

namespace fastbird
{
	struct FBFilterCallback;
	class BulletDebugDraw;
	struct BulletFilterCallback;
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
		btCollisionShape* CreateColShapeForGroup(IPhysicsInterface* shapeProvider, const Vec3I& groupIdx);
		btCollisionShape* CreateColShape(CollisionShape* colShapes[], unsigned num, bool forceCompound);
		RigidBody* _CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj, bool createMotionSTate);
		BulletDebugDraw mDebugDrawer;


		//-------------------------------------------------------------------
		// Private Functions
		//-------------------------------------------------------------------
		btCollisionShape* CreateBulletColShape(CollisionShape* colShape);

		int mRayGroup;

		static unsigned NextInternalColShapeId;
		VectorMap<unsigned, btCollisionShape*> mInternalShapes;

		BulletFilterCallback* mFilterCallback;
		bool mEnabled;


	public:
		Physics::Physics();
		Physics::~Physics();
		
		static NeedCollisionForConvexCallback sNeedCollisionForConvexCallback;

		virtual void Initilaize();
		virtual void Deinitilaize();
		virtual void Update(float dt);
		virtual void EnablePhysics();
		virtual void DisablePhysics();

		// internal only.
		void _ReportCollisions();
		void _CheckCollisionShapeForDel(float timeStep);
		btDynamicsWorld* _GetDynamicWorld() const { return mDynamicsWorld; }

		virtual RigidBody* CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj);
		virtual RigidBody* CreateRigidBody(IPhysicsInterface* obj);
		virtual RigidBody* CreateRigidBodyForGroup(IPhysicsInterface* colProvider, const Vec3I& groupIdx);
		virtual RigidBody* CreateTempRigidBody(CollisionShape* colShape);
		virtual RigidBody* CreateTempRigidBody(CollisionShape*  shapes[], unsigned num);

		virtual void DeleteRigidBody(RigidBody* rigidBody);
		btCollisionShape* ParseCollisionFile(const char* collisionFile);
		
		virtual void RemoveConstraint(btTypedConstraint* constraint);

		virtual void AddRef(btCollisionShape* colShape);
		virtual void Release(btCollisionShape* colShape);
		virtual void SetDebugDrawer(IDebugDrawer* debugDrawer);
		virtual void SetDebugMode(int debugMode);

		
		virtual void AttachBodies(RigidBody* bodies[], unsigned num);
		virtual void AttachBodiesAlways(RigidBody* bodies[], unsigned num);

		virtual void SetRayCollisionGroup(int group);
		virtual bool RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int additionalRayGroup, int mask, RayResultClosest& result, void* excepts[] = 0, unsigned numExcepts = 0);
		virtual bool RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, RayResultWithObj& result);
		virtual RayResultAll* RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, int mask);
		virtual void Release(RayResultAll* r);
		virtual unsigned GetAABBOverlaps(const AABB& aabb, unsigned colMask, RigidBody* ret[], unsigned limit, RigidBody* except);
		virtual float GetDistanceBetween(RigidBody* a, RigidBody* b);

		virtual unsigned CreateBTSphereShape(float radius);
		virtual void DeleteBTShape(unsigned id);
		virtual void DrawDebugInfo();

		//-------------------------------------------------------------------
		// collision shape manager
		//-------------------------------------------------------------------
		virtual BoxShape* CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		virtual SphereShape* CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr = 0);
		virtual CylinderShape* CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		virtual CapsuleShape* CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr = 0);
		virtual MeshShape* CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
			bool staticObj, void* userPtr = 0);
		virtual MeshShape* CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
			const Vec3& scale, void* userPtr = 0);
		virtual void DestroyShape(CollisionShape* shape);


		virtual void RegisterFilterCallback(IFilterCallback* callback, NeedCollisionForConvexCallback func);

		virtual void SetEngine(IEngine* engine);
	};
}