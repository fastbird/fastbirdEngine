#include <Physics/stdafx.h>
#include <Physics/Physics.h>
#include <Physics/mathConv.h>
#include <Physics/fbMotionState.h>
#include <Physics/RigidBodyImpl.h>
#include <Physics/ColShapes.h>
#include <Physics/IPhysicsInterface.h>
#include <Physics/RayResult.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>

namespace fastbird{

unsigned Physics::NextInternalColShapeId = 1;
Physics::Physics()
	: mRayGroup(0x40) // default of the current game under development
{

}

Physics::~Physics()
{
}


void Log(const char* szFmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	OutputDebugString(buf);
	OutputDebugString("\n");
	std::cout << buf << std::endl;
}
void Error(const char* szFmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	OutputDebugString(buf);
	OutputDebugString("\n");
	std::cout << buf << std::endl;
}

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
	auto physics = (Physics*)gFBPhysics;
	physics->_ReportCollisions();
	physics->_CheckCollisionShapeForDel(timeStep);
}

void Physics::Initilaize()
{
	///collision configuration contains default setup for memory, collision setup
	mCollisionConfiguration = FB_NEW_ALIGNED(btDefaultCollisionConfiguration, MemAlign);
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	mDispatcher = FB_NEW_ALIGNED(btCollisionDispatcher, MemAlign)(mCollisionConfiguration);

	mBroadphase = FB_NEW_ALIGNED(btDbvtBroadphase, MemAlign)();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = FB_NEW_ALIGNED(btSequentialImpulseConstraintSolver, MemAlign);
	mSolver = sol;

	mDynamicsWorld = FB_NEW_ALIGNED(btDiscreteDynamicsWorld, MemAlign)(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
	mDynamicsWorld->getDispatchInfo().m_useContinuous = true;

	mDynamicsWorld->setGravity(btVector3(0, 0, 0));
	mDynamicsWorld->setDebugDrawer(&mDebugDrawer);
	mDynamicsWorld->setInternalTickCallback(TickCallback);

	//auto& info = mDynamicsWorld->getSolverInfo();
	//info.m_splitImpulse = 1;
}

void Physics::Deinitilaize()
{
	if (mDynamicsWorld)
	{

		int i;
		for (i = mDynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
		{
			mDynamicsWorld->removeConstraint(mDynamicsWorld->getConstraint(i));
		}
		for (i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				FB_DEL_ALIGNED(body->getMotionState());
			}
			mDynamicsWorld->removeCollisionObject(obj);
			FB_DEL_ALIGNED(obj);
		}

		for (auto it = mColShapePendingDelete.begin(); it != mColShapePendingDelete.end();)
		{
			auto colShape = it->first;
			it = mColShapePendingDelete.erase(it);
			if (colShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
			{
				btCompoundShape* compound = (btCompoundShape*)(colShape);
				unsigned num = compound->getNumChildShapes();
				int idx = num - 1;
				while (idx >= 0)
				{
					auto shape = compound->getChildShape(idx);
					compound->removeChildShapeByIndex(idx);
					FB_DEL_ALIGNED(shape);
					--idx;
				}
			}
			FB_DEL_ALIGNED(colShape);
		}
	}
	//delete collision shapes
	/*for (int j = 0; j<mCollisionShapes.size(); j++)
	{
	btCollisionShape* shape = mCollisionShapes[j];
	delete shape;
	}
	mCollisionShapes.clear();*/

	assert(mColShapesRefs.empty());
	mColShapesRefs.clear();
	assert(mColShapes.empty());
	mColShapes.clear();

	FB_DEL_ALIGNED(mDynamicsWorld);

	FB_DEL_ALIGNED(mSolver);

	FB_DEL_ALIGNED(mBroadphase);

	FB_DEL_ALIGNED(mDispatcher);

	FB_DEL_ALIGNED(mCollisionConfiguration);
}

void Physics::Update(float dt)
{
	if (mDynamicsWorld)
	{
		mDynamicsWorld->stepSimulation(dt, 8);
		if (mDebugDrawer.getDebugMode() != 0)
		{
			mDynamicsWorld->debugDrawWorld();
		}
	}
}

void Physics::_ReportCollisions()
{
	// manifolds
	for (int i = 0; i < mDynamicsWorld->getDispatcher()->getNumManifolds(); i++)
	{
		btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		auto colObjA = contactManifold->getBody0();
		auto colObjB = contactManifold->getBody1();
		if (!colObjA || !colObjB)
			continue;
		int numContacts = contactManifold->getNumContacts();
		if (numContacts == 0)
			continue;

		const RigidBodyImpl* obA = (RigidBodyImpl*)colObjA->getUserPointer();
		const RigidBodyImpl* obB = (RigidBodyImpl*)colObjB->getUserPointer();

		if (!obA || !obB)
		{
			continue;
		}

		IPhysicsInterface* a = (IPhysicsInterface*)obA->GetPhysicsInterface();
		IPhysicsInterface* b = (IPhysicsInterface*)obB->GetPhysicsInterface();
		if (a && b)
		{
			a->AddCloseObjects(obB->GetGamePtr());
			b->AddCloseObjects(obA->GetGamePtr());
		}
		else
		{
			continue;
		}

		for (int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);

			btVector3 ptA = pt.getPositionWorldOnA();
			btVector3 ptB = pt.getPositionWorldOnB();
			btVector3 normal = ptA - ptB;
			normal.safeNormalize();
			float impulse = pt.getAppliedImpulse();
			if (impulse > 0)
			{
				IPhysicsInterface::CollisionContactInfo contactInfo(obB->GetGamePtr(), BulletToFB(ptB), BulletToFB(normal),
					impulse, pt.m_index0, pt.m_index1);

				bool processed = a->OnCollision(contactInfo);
				if (!processed)
				{
					IPhysicsInterface::CollisionContactInfo contactInfo(obA->GetGamePtr(), BulletToFB(ptA), BulletToFB(-normal),
						impulse, pt.m_index1, pt.m_index0);
					b->OnCollision(contactInfo);
				}
				break;
			}
		}
	}
}

void Physics::_CheckCollisionShapeForDel(float timeStep)
{
	for (auto it = mColShapePendingDelete.begin(); it != mColShapePendingDelete.end();)
	{
		auto colShape = it->first;
		it->second -= timeStep;
		if (it->second <= 0)
		{
			it = mColShapePendingDelete.erase(it);
			if (colShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
			{
				btCompoundShape* compound = (btCompoundShape*)(colShape);
				unsigned num = compound->getNumChildShapes();
				int idx = num - 1;
				while (idx >= 0)
				{
					auto shape = compound->getChildShape(idx);
					compound->removeChildShapeByIndex(idx);
					FB_DEL_ALIGNED(shape);
					--idx;
				}
			}
			FB_DEL_ALIGNED(colShape);
		}
		else
		{
			it++;
		}
	}
}

RigidBody* Physics::CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj)
{
	btCollisionShape* colShape = ParseCollisionFile(collisionFile);
	return _CreateRigidBodyInternal(colShape, mass, obj);
}

RigidBody* Physics::CreateRigidBody(IPhysicsInterface* obj)
{
	if (!obj)
	{
		Error(DEFAULT_DEBUG_ARG, "no physics interface is provided!");
		return 0;
	}
	auto colShape = CreateColShape(obj);
	if (!colShape)
		return 0;
	return _CreateRigidBodyInternal(colShape, obj->GetMass(), obj);
}

RigidBody* Physics::CreateTempRigidBody(CollisionShape* colShape)
{
	btVector3 localInertia(0, 0, 0);
	auto btcol = CreateBulletColShape(colShape);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, 0, btcol, localInertia);
	RigidBody* rigidBody = FB_NEW_ALIGNED(RigidBodyImpl, MemAlign)(rbInfo, 0, 0);
	return rigidBody;
}

RigidBody* Physics::CreateTempRigidBody(CollisionShape*  shapes[], unsigned num)
{
	btVector3 localInertia(0, 0, 0);
	auto btcol = CreateColShape(shapes, num);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, 0, btcol, localInertia);
	RigidBody* rigidBody = FB_NEW_ALIGNED(RigidBodyImpl, MemAlign)(rbInfo, 0, 0);
	return rigidBody;
}

RigidBody* Physics::_CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj)
{
	fbMotionState* motionState = 0;
	bool dynamic = mass != 0.0f;
	btVector3 localInertia(0, 0, 0);
	if (dynamic)
	{
		colShape->calculateLocalInertia(mass, localInertia);
		if (obj)
			motionState = FB_NEW_ALIGNED(fbMotionState, MemAlign)(obj);

	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		mass, motionState, colShape, localInertia);
	rbInfo.m_angularDamping = obj->GetAngularDamping();
	rbInfo.m_linearDamping = obj->GetLinearDamping();
	RigidBody* rigidBody = 0;
	rigidBody = FB_NEW_ALIGNED(RigidBodyImpl, MemAlign)(rbInfo, mDynamicsWorld, obj);
	rigidBody->SetPhysicsInterface(obj);
	return rigidBody;
}

btCollisionShape* Physics::CreateBulletColShape(CollisionShape* colShape)
{
	switch (colShape->mType)
	{
	case CollisionShapes::Box:
	{
		auto shape = (BoxShape*)colShape;
		auto e = FBToBullet(shape->mExtent * shape->mScale);
		return FB_NEW_ALIGNED(btBoxShape, MemAlign)(e);
		break;
	}
	case CollisionShapes::Sphere:
	{
		auto shape = (SphereShape*)colShape;
		return FB_NEW_ALIGNED(btSphereShape, MemAlign)(shape->mRadius * shape->mScale.x);
		break;
	}
	case CollisionShapes::Cylinder:
	{
		auto shape = (CylinderShape*)colShape;
		return FB_NEW_ALIGNED(btCylinderShape, MemAlign)(FBToBullet(shape->mExtent * shape->mScale));
		break;
	}
	case CollisionShapes::Capsule:
	{
		auto shape = (CapsuleShape*)colShape;
		return FB_NEW_ALIGNED(btCapsuleShape, MemAlign)(shape->mRadius * shape->mScale.x, shape->mHeight * shape->mScale.x);
		break;
	}
	case CollisionShapes::StaticMesh:
	{
		auto shape = (MeshShape*)colShape;
		auto btshape = FB_NEW_ALIGNED(btBvhTriangleMeshShape, MemAlign)(shape->GetTriangleMesh(), true);
		return btshape;
	}
	case CollisionShapes::DynamicMesh:
	{
		auto shape = (MeshShape*)colShape;
		auto btshape = FB_NEW_ALIGNED(btGImpactMeshShape, MemAlign)(shape->GetTriangleMesh());
		return btshape;
	}
	case CollisionShapes::Convex:
	{
		auto shape = (MeshShape*)colShape;
		auto btshape = FB_NEW_ALIGNED(btConvexHullShape, MemAlign)(&shape->mVertices[0].x,
			shape->mNumVertices, 12);
		return btshape;
	}
	default:
		assert(0);
	}
	return 0;
}

btCollisionShape* Physics::CreateColShape(IPhysicsInterface* shapeProvider)
{
	if_assert_fail(shapeProvider)
		return 0;

	CollisionShape* shapes[2048];
	auto num = shapeProvider->GetShapes(shapes);
	assert(num < 2048);
	return CreateColShape(shapes, num);
}

btCollisionShape* Physics::CreateColShape(CollisionShape* shapes[], unsigned num)
{
	if (num==0)
	{
		Error(DEFAULT_DEBUG_ARG, "No collision shapes!");
		return 0;
	}
	if (num > 1 || shapes[0]->mPos != Vec3::ZERO || shapes[0]->mRot != Quat::IDENTITY)
	{
		btCompoundShape* compound = FB_NEW_ALIGNED(btCompoundShape, MemAlign);
		if (compound == 0){
			Error("FB_NEW_ALIGNED failed!");
		}
		for (unsigned i = 0; i < num; ++i)
		{
			auto colShape = shapes[i];
			if (colShape)
			{
				btCollisionShape* btshape = CreateBulletColShape(colShape);

				btTransform t;
				t.setIdentity();
				t.setRotation(FBToBullet(colShape->mRot));
				t.setOrigin(FBToBullet(colShape->mPos));
				compound->addChildShape(t, btshape);
				if (btshape)
				{
					btshape->setUserPointer(colShape->mUserPtr);
				}
			}
		}
		return compound;
	}
	else
	{
		auto colShape = shapes[0];
		return CreateBulletColShape(colShape);
	}
}

void Physics::DeleteRigidBody(RigidBody* rigidBody)
{
	FB_DEL_ALIGNED(rigidBody);
}

btCollisionShape* Physics::ParseCollisionFile(const char* collisionFile)
{
	if (!collisionFile || strlen(collisionFile) == 0)
	{
		Error("noCollision file!");
		return 0;
	}
	std::string lowerPath(collisionFile);
	ToLowerCase(lowerPath);
	auto it = mColShapes.Find(lowerPath);
	if (it != mColShapes.end())
		return it->second;

	tinyxml2::XMLDocument doc;
	int errId = doc.LoadFile(collisionFile);
	if (errId)
	{
		Error("Physics::ParseAction ParseCollisionFile %s : %s", collisionFile, doc.GetErrorStr1());
		Error("	%s", doc.GetErrorStr2());
		assert(0);
		return 0;
	}
	auto csElem = doc.FirstChildElement("CollisionShape");
	btCompoundShape* compoundShape = 0;
	const char* sz = csElem->Attribute("compound");
	if (sz)
	{
		if (StringConverter::parseBool(sz))
		{
			compoundShape = FB_NEW_ALIGNED(btCompoundShape, MemAlign)();
		}
	}

	auto shapeElem = csElem->FirstChildElement("Shape");
	while (shapeElem)
	{
		sz = shapeElem->Attribute("type");
		if (sz)
		{
			CollisionShapes::Enum colShape = CollisionShapes::ConvertToEnum(sz);
			btVector3 origin;
			btQuaternion rot;
			sz = shapeElem->Attribute("origin");
			if (sz)
			{
				Vec3 o = StringConverter::parseVec3(sz);
				origin = FBToBullet(o);
			}
			sz = shapeElem->Attribute("rot");
			if (sz)
			{
				Quat r = StringConverter::parseQuat(sz);
				rot = FBToBullet(r);
			}
			btCollisionShape* btColShape = 0;
			switch (colShape)
			{
			case CollisionShapes::Box:
			{
				Vec3 extent(1, 1, 1);
				sz = shapeElem->Attribute("extent");
				extent = StringConverter::parseVec3(sz);
				btColShape = FB_NEW_ALIGNED(btBoxShape, MemAlign)(FBToBullet(extent));
				break;
			}
			case CollisionShapes::Sphere:
			{
				float radius = 1.0f;
				sz = shapeElem->Attribute("radius");
				radius = StringConverter::parseReal(sz);
				btColShape = FB_NEW_ALIGNED(btSphereShape, MemAlign)(radius);
				break;

			}
			case CollisionShapes::Cylinder:
			{
				Vec3 extent(1, 1, 1);
				sz = shapeElem->Attribute("extent");
				extent = StringConverter::parseVec3(sz);
				btColShape = FB_NEW_ALIGNED(btCylinderShape, MemAlign)(FBToBullet(extent));

				break;
			}
			case CollisionShapes::Capsule:
			{
				float radius = 1.0f;
				float height = 1.0f;
				sz = shapeElem->Attribute("radius");
				radius = StringConverter::parseReal(sz);
				sz = shapeElem->Attribute("height");
				height = StringConverter::parseReal(sz);
				btColShape = FB_NEW_ALIGNED(btCapsuleShape, MemAlign)(radius, height);
				break;
			}

			}

			if (compoundShape)
			{
				btTransform t;
				t.setIdentity();
				t.setRotation(rot);
				t.setOrigin(origin);
				compoundShape->addChildShape(t, btColShape);
			}
			else
			{
				return btColShape;
			}
		}

		shapeElem = csElem->FirstChildElement("Shape");
	}
	assert(compoundShape);
	return compoundShape;

}

void Physics::RemoveConstraint(btTypedConstraint* constraint)
{
	mDynamicsWorld->removeConstraint(constraint);
	FB_DEL_ALIGNED(constraint);
}

void Physics::AddRef(btCollisionShape* colShape)
{
	if (!colShape)
		return;
	auto it = mColShapesRefs.Find(colShape);
	if (it == mColShapesRefs.end())
	{
		mColShapesRefs[colShape] = 1;
		return;
	}

	++it->second;
}

void Physics::Release(btCollisionShape* colShape)
{
	if (!colShape)
		return;

	auto it = mColShapesRefs.Find(colShape);
	if (it == mColShapesRefs.end())
	{
		Error("Physics::Release() : cannot find the colshape!");
		return;
	}

	--it->second;
	if (it->second == 0)
	{
		mColShapesRefs.erase(it);
		mColShapePendingDelete.Insert(std::make_pair(colShape, 3.f));
	}
}

void Physics::SetDebugDrawer(IDebugDrawer* debugDrawer)
{
	mDebugDrawer.SetCallback(debugDrawer);
}

void Physics::SetDebugMode(int debugMode)
{
	mDebugDrawer.setDebugMode(debugMode);
}

void Physics::AttachBodies(RigidBody* bodies[], unsigned num)
{
	struct ContactCallBack : public btCollisionWorld::ContactResultCallback
	{
		btDiscreteDynamicsWorld* mWorld;

		ContactCallBack(btDiscreteDynamicsWorld* world)
			:mWorld(world)
		{
			assert(world);
		}

		virtual	btScalar	addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
		{
			btRigidBody* body0 = (btRigidBody*)colObj0Wrap->getCollisionObject();
			btRigidBody* body1 = (btRigidBody*)colObj1Wrap->getCollisionObject();

			btTransform globalFrame;
			globalFrame.setIdentity();
			globalFrame.setOrigin((cp.getPositionWorldOnA() + cp.getPositionWorldOnB()) * .5f);
			btTransform trA = body0->getWorldTransform().inverse() * globalFrame;
			btTransform trB = body1->getWorldTransform().inverse() * globalFrame;
			float totalMass = 1.0f / body0->getInvMass() + 1.0f / body1->getInvMass();

			btFixedConstraint* fixed = FB_NEW_ALIGNED(btFixedConstraint, MemAlign)(*body0, *body1, trA, trB);
			fixed->setBreakingImpulseThreshold(totalMass);
			fixed->setOverrideNumSolverIterations(30);
			mWorld->addConstraint(fixed, true);
			return 1.0f;
		}
	};

	ContactCallBack ccallback(mDynamicsWorld);
	for (unsigned i = 0; i < num - 1; ++i)
	{
		RigidBodyImpl* body = (RigidBodyImpl*)bodies[i];
		for (unsigned t = i + 1; t < num; ++t)
		{
			mDynamicsWorld->contactPairTest(body, (RigidBodyImpl*)bodies[t], ccallback);
		}
	}
}

void Physics::SetRayCollisionGroup(int group)
{
	mRayGroup = group;
}

bool Physics::RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultClosest& result, void* excepts[], unsigned numExcepts)
{
	auto from = FBToBullet(fromWorld);
	auto to = FBToBullet(toWorld);
	struct MyclosestRayResultCallBack : public btCollisionWorld::ClosestRayResultCallback
	{
		int mIndex;
		void** mExcepts;
		unsigned mNumExcepts;
		MyclosestRayResultCallBack(const btVector3&	rayFromWorld, const btVector3&	rayToWorld)
			:btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld)
			, mIndex(-1), mExcepts(0), mNumExcepts(0)
		{}

		virtual bool needsCollision(btBroadphaseProxy* proxy0) const
		{
			btCollisionObject* colObj = (btCollisionObject*)proxy0->m_clientObject;
			if (mExcepts && colObj)
			{
				auto rigidBody = (RigidBody*)colObj->getUserPointer();
				if (rigidBody){
					if (std::find(&mExcepts[0], &mExcepts[mNumExcepts], rigidBody->GetGamePtr()) != &mExcepts[mNumExcepts])
						return false;
				}
			}
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			return collides;
		}

		virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			//caller already does the filter on the m_closestHitFraction
			btAssert(rayResult.m_hitFraction <= m_closestHitFraction);

			m_closestHitFraction = rayResult.m_hitFraction;
			m_collisionObject = rayResult.m_collisionObject;
			if (normalInWorldSpace)
			{
				m_hitNormalWorld = rayResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
			}
			m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
			auto colShape = m_collisionObject->getCollisionShape();
			if (colShape->isCompound())
			{
				mIndex = rayResult.m_localShapeInfo->m_triangleIndex;
			}
			return rayResult.m_hitFraction;
		}

	};
	MyclosestRayResultCallBack cb(from, to);
	cb.m_collisionFilterGroup = mRayGroup;
	cb.m_collisionFilterMask = mask;
	cb.mExcepts = excepts;
	cb.mNumExcepts = numExcepts;
	mDynamicsWorld->rayTest(from, to, cb);
	if (cb.hasHit())
	{
		result.mRigidBody = (RigidBody*)(cb.m_collisionObject->getUserPointer());
		result.mHitPointWorld = BulletToFB(cb.m_hitPointWorld);
		result.mHitNormalWorld = BulletToFB(cb.m_hitNormalWorld);
		result.mIndex = cb.mIndex;
		return true;
	}
	else{
		result.mRigidBody = 0;
	}
	return false;
}

bool Physics::RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, RayResultWithObj& result)
{
	struct ObjectHitsRayResultCallback : public btCollisionWorld::AllHitsRayResultCallback
	{
		int mIndex;
		RigidBodyImpl* mTarget;
		btVector3 mHitNormalWorld;
		btVector3 mHitPointWorld;
		float mHitFraction;

		ObjectHitsRayResultCallback(const btVector3&	rayFromWorld, const btVector3&	rayToWorld, RigidBody* targetObj)
			:btCollisionWorld::AllHitsRayResultCallback(rayFromWorld, rayToWorld)
		{
			mTarget = (RigidBodyImpl*)targetObj;
			mHitFraction = 1.0f;
			mIndex = -1;
			assert(mTarget);
		}

		virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			if (rayResult.m_collisionObject->getUserPointer() != mTarget)
				return mHitFraction;

			if (mHitFraction > rayResult.m_hitFraction)
			{
				mHitFraction = rayResult.m_hitFraction;
				m_collisionObject = rayResult.m_collisionObject;
				if (normalInWorldSpace)
				{
					mHitNormalWorld = rayResult.m_hitNormalLocal;
				}
				else
				{
					///need to transform normal into worldspace
					mHitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
				}
				mHitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
				auto colShape = m_collisionObject->getCollisionShape();
				if (colShape->isCompound())
					mIndex = rayResult.m_localShapeInfo->m_triangleIndex;
				else
					mIndex = -1;
			}

			return mHitFraction;
		}
	};

	if (!result.mTargetBody)
		return false;

	auto from = FBToBullet(fromWorld);
	auto to = FBToBullet(toWorld);

	ObjectHitsRayResultCallback cb(from, to, result.mTargetBody);
	RigidBodyImpl* rigidBody = (RigidBodyImpl*)result.mTargetBody;
	cb.m_collisionFilterMask = -1;
	cb.m_collisionFilterGroup = -1;
	//cb.m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
	if (from == to)
		return false;
	mDynamicsWorld->rayTest(from, to, cb);
	if (cb.hasHit())
	{
		result.mRigidBody = (RigidBody*)(cb.m_collisionObject->getUserPointer());
		result.mHitPointWorld = BulletToFB(cb.mHitPointWorld);
		result.mHitNormalWorld = BulletToFB(cb.mHitNormalWorld);
		result.mIndex = cb.mIndex;

		return true;
	}
	return false;
}

RayResultAll* Physics::RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int mask)
{
	auto from = FBToBullet(fromWorld);
	auto to = FBToBullet(toWorld);

	struct MyAllHitsRayResultCallback : public btCollisionWorld::AllHitsRayResultCallback
	{
		btAlignedObjectArray<int> mIndex;

		MyAllHitsRayResultCallback(const btVector3&	rayFromWorld, const btVector3&	rayToWorld)
			:btCollisionWorld::AllHitsRayResultCallback(rayFromWorld, rayToWorld)
		{
		}

		virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			m_collisionObject = rayResult.m_collisionObject;
			m_collisionObjects.push_back(rayResult.m_collisionObject);
			btVector3 hitNormalWorld;
			if (normalInWorldSpace)
			{
				hitNormalWorld = rayResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
			}
			m_hitNormalWorld.push_back(hitNormalWorld);
			btVector3 hitPointWorld;
			hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
			m_hitPointWorld.push_back(hitPointWorld);
			m_hitFractions.push_back(rayResult.m_hitFraction);
			auto colShape = m_collisionObject->getCollisionShape();
			if (colShape->isCompound())
				mIndex.push_back(rayResult.m_localShapeInfo->m_triangleIndex);
			else
				mIndex.push_back(-1);
			return m_closestHitFraction;
		}
	};

	MyAllHitsRayResultCallback cb(from, to);
	cb.m_collisionFilterMask = mask;
	cb.m_collisionFilterGroup = mRayGroup;
	mDynamicsWorld->rayTest(from, to, cb);
	if (cb.hasHit())
	{
		RayResultAll* result = FB_NEW(RayResultAll);
		unsigned numObjects = cb.m_collisionObjects.size();
		for (unsigned i = 0; i < numObjects; i++)
		{
			result->AddResult((RigidBody*)cb.m_collisionObjects[i]->getUserPointer(),
				BulletToFB(cb.m_hitPointWorld[i]),
				BulletToFB(cb.m_hitNormalWorld[i]), cb.mIndex[i]);
		}
		return result;
	}

	return 0;
}

void Physics::Release(RayResultAll* r){
	FB_DELETE(r);
}

///The AABBOverlapCallback is used to collect object that overlap with a given bounding box defined by aabbMin and aabbMax. 
struct	AABBOverlapCallback : public btBroadphaseAabbCallback
{
	btVector3 m_queryAabbMin;
	btVector3 m_queryAabbMax;
	RigidBody* mExcept;
	void** mRet;
	int mLimit;
	int mCurSize;
	unsigned mColMask;

	int m_numOverlap;
	AABBOverlapCallback(const btVector3& aabbMin, const btVector3& aabbMax, unsigned colMask,
		void* ret[], int limit, RigidBody* except)
		: m_queryAabbMin(aabbMin), m_queryAabbMax(aabbMax), m_numOverlap(0), mExcept(except), mRet(ret)
		, mLimit(limit), mColMask(colMask), mCurSize(0)
	{}
	virtual bool	process(const btBroadphaseProxy* proxy)
	{
		if (mCurSize >= mLimit)
			return false;

		btVector3 proxyAabbMin, proxyAabbMax;
		btCollisionObject* colObj0 = (btCollisionObject*)proxy->m_clientObject;
		RigidBody* rigidBody = (RigidBody*)colObj0->getUserPointer();
		if (!rigidBody || rigidBody == mExcept)
			return true;
		if ((proxy->m_collisionFilterGroup & mColMask) == 0)
			return true;

		auto gamePtr = rigidBody->GetGamePtr();
		if (gamePtr)
		{
			colObj0->getCollisionShape()->getAabb(colObj0->getWorldTransform(), proxyAabbMin, proxyAabbMax);
			if (TestAabbAgainstAabb2(proxyAabbMin, proxyAabbMax, m_queryAabbMin, m_queryAabbMax))
			{
				mRet[mCurSize++] = gamePtr;
			}
		}
		return true;
	}
};

unsigned Physics::GetAABBOverlaps(const AABB& aabb, unsigned colMask, void* ret[], unsigned limit, RigidBody* except)
{
	btVector3 min = FBToBullet(aabb.GetMin());
	btVector3 max = FBToBullet(aabb.GetMax());
	AABBOverlapCallback callback(min, max, colMask, ret, limit, except);
	mDynamicsWorld->getBroadphase()->aabbTest(min, max, callback);
	return callback.mCurSize;
}

float Physics::GetDistanceBetween(RigidBody* a, RigidBody* b)
{
	if (!a || !b)
	{
		return FLT_MAX;
	}
	float distance = FLT_MAX;
	RigidBodyImpl* aImpl = (RigidBodyImpl*)a;
	RigidBodyImpl* bImpl = (RigidBodyImpl*)b;
	auto aColShape = aImpl->getCollisionShape();
	auto bColShape = bImpl->getCollisionShape();
	btCompoundShape* aCompound = aColShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE ? (btCompoundShape*)aColShape : 0;
	btCompoundShape* bCompound = bColShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE ? (btCompoundShape*)bColShape : 0;
	if (aCompound && bCompound)
	{
		unsigned anum = aCompound->getNumChildShapes();
		unsigned bnum = bCompound->getNumChildShapes();
		for (unsigned ai = 0; ai < anum; ai++)
		{
			for (unsigned bi = 0; bi < bnum; bi++)
			{
				auto achild = aCompound->getChildShape(ai);
				assert(achild->getShapeType() >= BOX_SHAPE_PROXYTYPE && achild->getShapeType() < CONCAVE_SHAPES_START_HERE);
				auto bchild = bCompound->getChildShape(bi);
				assert(bchild->getShapeType() >= BOX_SHAPE_PROXYTYPE && bchild->getShapeType() < CONCAVE_SHAPES_START_HERE);
				btVoronoiSimplexSolver sGjkSimplexSolver;
				btGjkEpaPenetrationDepthSolver epaSolver;
				btPointCollector gjkOutput;
				btGjkPairDetector convexConvex((btConvexShape*)achild, (btConvexShape*)bchild, &sGjkSimplexSolver, &epaSolver);
				btGjkPairDetector::ClosestPointInput input;
				input.m_transformA = aImpl->getWorldTransform() * aCompound->getChildTransform(ai);
				input.m_transformB = bImpl->getWorldTransform() * bCompound->getChildTransform(bi);

				convexConvex.getClosestPoints(input, gjkOutput, 0);
				distance = std::min(gjkOutput.m_distance, distance);
			}
		}
	}

	if (!aCompound && bCompound)
	{
		std::swap(aCompound, bCompound);
		std::swap(a, b);
		std::swap(aImpl, bImpl);
		std::swap(aColShape, bColShape);
	}

	if (aCompound && !bCompound)
	{
		unsigned anum = aCompound->getNumChildShapes();
		for (unsigned ai = 0; ai < anum; ai++)
		{
			auto achild = aCompound->getChildShape(ai);
			assert((achild->getShapeType() >= BOX_SHAPE_PROXYTYPE && achild->getShapeType() < CONCAVE_SHAPES_START_HERE) || bColShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE);
			assert((bColShape->getShapeType() >= BOX_SHAPE_PROXYTYPE && bColShape->getShapeType() < CONCAVE_SHAPES_START_HERE) || bColShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE);
			btVoronoiSimplexSolver sGjkSimplexSolver;
			btGjkEpaPenetrationDepthSolver epaSolver;
			btPointCollector gjkOutput;
			btGjkPairDetector convexConvex((btConvexShape*)achild, (btConvexShape*)bColShape, &sGjkSimplexSolver, &epaSolver);
			btGjkPairDetector::ClosestPointInput input;
			input.m_transformA = aImpl->getWorldTransform() * aCompound->getChildTransform(ai);
			input.m_transformB = bImpl->getWorldTransform();

			convexConvex.getClosestPoints(input, gjkOutput, 0);
			distance = std::min(gjkOutput.m_distance, distance);
		}
	}

	else if (!aCompound && !bCompound)
	{
		//assert(aColShape->getShapeType() >= BOX_SHAPE_PROXYTYPE && aColShape->getShapeType() < CONCAVE_SHAPES_START_HERE);
		//assert(bColShape->getShapeType() >= BOX_SHAPE_PROXYTYPE && bColShape->getShapeType() < CONCAVE_SHAPES_START_HERE);
		btVoronoiSimplexSolver sGjkSimplexSolver;
		btGjkEpaPenetrationDepthSolver epaSolver;
		btPointCollector gjkOutput;
		btGjkPairDetector convexConvex((btConvexShape*)aColShape, (btConvexShape*)bColShape, &sGjkSimplexSolver, &epaSolver);
		btGjkPairDetector::ClosestPointInput input;
		input.m_transformA = aImpl->getWorldTransform();
		input.m_transformB = bImpl->getWorldTransform();

		convexConvex.getClosestPoints(input, gjkOutput, 0);
		distance = std::min(gjkOutput.m_distance, distance);
	}


	return distance;
}

unsigned Physics::CreateBTSphereShape(float radius){
	auto shape = FB_NEW_ALIGNED(btSphereShape, MemAlign)(radius);
	mInternalShapes[NextInternalColShapeId] = shape;
	auto ret = NextInternalColShapeId++;
	return ret;
}

void Physics::DeleteBTShape(unsigned id){
	auto it = mInternalShapes.Find(id);
	if (it != mInternalShapes.end()){
		FB_DEL_ALIGNED(it->second);
		mInternalShapes.erase(it);
	}
}

BoxShape* Physics::CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr)
{
	return CollisionShapeMan::CreateBoxShape(pos, rot, actorScale, extent, userPtr);
}
SphereShape* Physics::CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr)
{
	return CollisionShapeMan::CreateSphereShape(pos, rot, actorScale, radius, userPtr);
}
CylinderShape* Physics::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr)
{
	return CollisionShapeMan::CreateCylinderShape(pos, rot, actorScale, extent, userPtr);
}
CapsuleShape* Physics::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr )
{
	return CollisionShapeMan::CreateCylinderShape(pos, rot, actorScale, radius, height, userPtr);
}
MeshShape* Physics::CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
	bool staticObj, void* userPtr)
{
	return CollisionShapeMan::CreateMeshShape(pos, rot, vertices, numVertices, scale, staticObj, userPtr);
}
MeshShape* Physics::CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
	const Vec3& scale, void* userPtr)
{
	return CollisionShapeMan::CreateConvexMeshShape(pos, rot, vertices, numVertices, scale, userPtr);
}
void Physics::DestroyShape(CollisionShape* shape)
{
	CollisionShapeMan::DestroyShape(shape);
}

}