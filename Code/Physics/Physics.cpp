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

using namespace fastbird;

IPhysics* IPhysics::GetPhysics()
{
	static Physics* sp = 0;
	
	if (!sp)
	{
		sp = FB_NEW(Physics);
		sp->Initilaize();
	}

	return sp;
}

Physics::~Physics()
{
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

	mDynamicsWorld->setGravity(btVector3(0, 0, 0));
	mDynamicsWorld->setDebugDrawer(&mDebugDrawer);
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

	FB_DELETE(IPhysics::GetPhysics());
}

void Physics::Update(float dt)
{
	if (mDynamicsWorld)
	{
		mDynamicsWorld->stepSimulation(dt, 2);
		if (mDebugDrawer.getDebugMode() != 0)
		{
			mDynamicsWorld->debugDrawWorld();
		}
		ReportCollisions();
	}
}

void Physics::ReportCollisions()
{
	/*auto& pairs = mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->getOverlappingPairArray();
	auto numPairs = pairs.size();
	btManifoldArray manifoldArray;
	for (int i = 0; i < numPairs; ++i)
	{
		manifoldArray.clear();

		const auto& pair = pairs[i];
		auto* collisionPair = mDynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
		if (!collisionPair)
			continue;

		if (collisionPair->m_algorithm)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for (int j = 0; j < manifoldArray.size(); j++)
		{

		}
	}*/

	// manifolds
	int numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const RigidBodyImpl* obA = (RigidBodyImpl*)contactManifold->getBody0()->getUserPointer();
		const RigidBodyImpl* obB = (RigidBodyImpl*)contactManifold->getBody1()->getUserPointer();
		
		IPhysicsInterface* a = (IPhysicsInterface*)obA->GetPhysicsInterface();
		IPhysicsInterface* b = (IPhysicsInterface*)obB->GetPhysicsInterface();
		if (a && b)
		{
			a->AddCloseObjects(obB->GetGamePtr());
			b->AddCloseObjects(obA->GetGamePtr());
		}

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j<numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);

			btVector3 ptA = pt.getPositionWorldOnA();
			btVector3 ptB = pt.getPositionWorldOnB();
			btVector3 normal = ptA - ptB;
			normal.safeNormalize();
			float impulse = pt.getAppliedImpulse();
			if (impulse>0)
			{				
				if (a && b)
				{
					IPhysicsInterface::CollisionContactInfo contactInfo(obB->GetGamePtr(), BulletToFB(ptB), BulletToFB(normal),
						impulse, pt.m_index0, pt.m_index1);
						
					a->OnCollision(contactInfo);
				}
				contactManifold->clearManifold();
				break;
			}
		}
	}
}

RigidBody* Physics::CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj)
{
	btCollisionShape* colShape = ParseCollisionFile(collisionFile);
	return _CreateRigidBodyInternal(colShape, mass, obj);
}

RigidBody* Physics::CreateRigidBody(IPhysicsInterface* obj, float mass)
{
	auto colShape = CreateColShape(obj);
	if (!colShape)
		return 0;
	return _CreateRigidBodyInternal(colShape, mass, obj);
}

RigidBody* Physics::CreateRigidBody(CollisionShape* colShape, IPhysicsInterface* obj, float mass)
{
	auto btcol = CreateBulletColShape(colShape);
	return _CreateRigidBodyInternal(btcol, mass, obj);
}

RigidBody* Physics::_CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj)
{
	fbMotionState* motionState = FB_NEW_ALIGNED(fbMotionState, MemAlign)(obj);
	bool dynamic = mass != 0.0f;
	btVector3 localInertia(0, 0, 0);
	if (dynamic)
	{
		colShape->calculateLocalInertia(mass, localInertia);
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
								 auto e = FBToBullet(shape->mExtent);
								 return FB_NEW_ALIGNED(btBoxShape, MemAlign)(e);
								 break;
	}
	case CollisionShapes::Sphere:
	{
									auto shape = (SphereShape*)colShape;
									return FB_NEW_ALIGNED(btSphereShape, MemAlign)(shape->mRadius);
									break;
	}
	case CollisionShapes::Cylinder:
	{
									  auto shape = (CylinderShape*)colShape;
									  return FB_NEW_ALIGNED(btCylinderShape, MemAlign)(FBToBullet(shape->mExtent));
									  break;
	}
	case CollisionShapes::Capsule:
	{
									 auto shape = (CapsuleShape*)colShape;
									 return FB_NEW_ALIGNED(btCapsuleShape, MemAlign)(shape->mRadius, shape->mHeight);
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

	auto num = shapeProvider->GetNumColShapes();
	if (num == 0)
		return 0;
	if (num > 1)
	{
		btCompoundShape* compound = FB_NEW_ALIGNED(btCompoundShape, MemAlign);
		for (unsigned i = 0; i < num; ++i)
		{
			auto colShape = shapeProvider->GetShape(i);
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
		auto colShape = shapeProvider->GetShape(0);
		return CreateBulletColShape(colShape);
	}
	assert(0);
	return 0;
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
			CollisionShapes::Enum colShape = CollisionShapes::ConverToEnum(sz);
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

void Physics::SetDebugDrawer(IDebugDrawer* debugDrawer)
{
	mDebugDrawer.SetCallback(debugDrawer);
}

void Physics::SetDebugMode(int debugMode)
{
	mDebugDrawer.setDebugMode(debugMode);
}

void Physics::AttachBodies(const std::vector<RigidBody*>& bodies)
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
	unsigned numBodies = bodies.size();
	for (unsigned i = 0; i < numBodies-1; ++i)
	{
		RigidBodyImpl* body = (RigidBodyImpl*)bodies[i];
		for (unsigned t = i + 1; t < numBodies; ++t)
		{
			mDynamicsWorld->contactPairTest(body, (RigidBodyImpl*)bodies[t], ccallback);
		}
	}	
}

bool Physics::RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultClosest& result)
{
	auto from = FBToBullet(fromWorld);
	auto to = FBToBullet(toWorld);
	struct MyclosestRayResultCallBack : public btCollisionWorld::ClosestRayResultCallback
	{
		int mIndex;

		MyclosestRayResultCallBack(const btVector3&	rayFromWorld, const btVector3&	rayToWorld)
		:btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld)
		, mIndex(-1)
		{}

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
	cb.m_collisionFilterMask = mask;
	mDynamicsWorld->rayTest(from, to, cb);
	if (cb.hasHit())
	{
		result.mRigidBody = (RigidBody*)(cb.m_collisionObject->getUserPointer());
		result.mHitPointWorld = BulletToFB(cb.m_hitPointWorld);
		result.mHitNormalWorld = BulletToFB(cb.m_hitNormalWorld);
		result.mIndex = cb.mIndex;
		return true;
	}
	return false;
}

bool Physics::RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, RayResultWithObj& result)
{
	auto from = FBToBullet(fromWorld);
	auto to = FBToBullet(toWorld);
	
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
	ObjectHitsRayResultCallback cb(from, to, result.mTargetBody);
	RigidBodyImpl* rigidBody = (RigidBodyImpl*)result.mTargetBody;
	cb.m_collisionFilterMask = rigidBody->getCollisionFlags();
	//cb.m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
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

bool Physics::RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int mask, RayResultAll& result)
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
	mDynamicsWorld->rayTest(from, to, cb);
	if (cb.hasHit())
	{
		unsigned numObjects = cb.m_collisionObjects.size();
		for (unsigned i = 0; i < numObjects; i++)
		{
			result.AddResult((RigidBody*)cb.m_collisionObjects[i]->getUserPointer(), 
				BulletToFB(cb.m_hitPointWorld[i]), 
				BulletToFB(cb.m_hitNormalWorld[i]), cb.mIndex[i]);
		}
		return true;
	}
	return false;
}

///The AABBOverlapCallback is used to collect object that overlap with a given bounding box defined by aabbMin and aabbMax. 
struct	AABBOverlapCallback : public btBroadphaseAabbCallback
{
	btVector3 m_queryAabbMin;
	btVector3 m_queryAabbMax;
	RigidBody* mExcept;
	std::vector<void*>& mRet;
	int mLimit;
	unsigned mColMask;

	int m_numOverlap;
	AABBOverlapCallback(const btVector3& aabbMin, const btVector3& aabbMax, unsigned colMask,
		int limit, std::vector<void*>& ret, RigidBody* except)
		: m_queryAabbMin(aabbMin), m_queryAabbMax(aabbMax), m_numOverlap(0), mExcept(except), mRet(ret)
		, mLimit(limit), mColMask(colMask)
	{}
	virtual bool	process(const btBroadphaseProxy* proxy)
	{
		if (mRet.size() >= (unsigned)mLimit)
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
				mRet.push_back(gamePtr);
			}
		}		
		return true;
	}
};

void Physics::GetAABBOverlaps(const AABB& aabb, unsigned colMask, unsigned limit, std::vector<void*>& ret, RigidBody* except)
{
	btVector3 min = FBToBullet(aabb.GetMin());
	btVector3 max = FBToBullet(aabb.GetMax());
	AABBOverlapCallback callback(min, max, colMask, limit, ret, except);
	mDynamicsWorld->getBroadphase()->aabbTest(min, max, callback);
}