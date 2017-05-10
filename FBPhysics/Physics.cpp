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
#include "Physics.h"
#include "fbMotionState.h"
#include "RigidBodyImpl.h"
#include "ColShapes.h"
#include "IPhysics.h"
#include "IPhysicsInterface.h"
#include "RayResult.h"
#include "IFilterCallback.h"
#include "BulletFilterCallback.h"
#include "BulletDebugDraw.h"
#include "FBFileSystem/FileSystem.h"
#include "FBCommonHeaders/Helpers.h"
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
using namespace fb;

                                                         // shape index for compound
using AABBResultType = std::vector<std::pair<RigidBody*, unsigned>>;
///The AABBOverlapCallback is used to collect object that overlap with a given bounding box defined by aabbMin and aabbMax. 
struct	AABBOverlapCallback : public btBroadphaseAabbCallback
{
	btVector3 m_queryAabbMin;
	btVector3 m_queryAabbMax;
	RigidBody* mExcept;
	AABBResultType& mRet;
	int mLimit;
	unsigned mColMask;

	int m_numOverlap;
	AABBOverlapCallback(const btVector3& aabbMin, const btVector3& aabbMax, unsigned colMask,
		AABBResultType& ret, int limit, RigidBody* except)
		: m_queryAabbMin(aabbMin), m_queryAabbMax(aabbMax), m_numOverlap(0), mExcept(except), mRet(ret)
		, mLimit(limit), mColMask(colMask)
	{}
	virtual bool	process(const btBroadphaseProxy* proxy)
	{
		if ((int)mRet.size() >= mLimit)
			return false;

		btVector3 proxyAabbMin, proxyAabbMax;
		btCollisionObject* colObj0 = (btCollisionObject*)proxy->m_clientObject;
		RigidBody* rigidBody = (RigidBody*)colObj0->getUserPointer();
		if (!rigidBody || rigidBody == mExcept)
			return true;
		if ((proxy->m_collisionFilterGroup & mColMask) == 0)
			return true;
		auto colshape = colObj0->getCollisionShape();
		if (colshape->isCompound()) {
			btCompoundShape* cshape= (btCompoundShape*)colshape;
			auto numchildren = cshape->getNumChildShapes();
			for (int c = 0; c < numchildren && (int)mRet.size() < mLimit; ++c) {
				auto child = cshape->getChildShape(c);
				child->getAabb(colObj0->getWorldTransform() * cshape->getChildTransform(c),
					proxyAabbMin, proxyAabbMax);
				if (TestAabbAgainstAabb2(proxyAabbMin, proxyAabbMax, m_queryAabbMin, m_queryAabbMax))
				{
					mRet.push_back({ rigidBody, c });										
				}
			}
		}
		else {
			colshape->getAabb(colObj0->getWorldTransform(), proxyAabbMin, proxyAabbMax);
			if (TestAabbAgainstAabb2(proxyAabbMin, proxyAabbMax, m_queryAabbMin, m_queryAabbMax))
			{
				mRet.push_back({ rigidBody, -1 });				
			}
		}

		return true;
	}
};

void TickCallback(btDynamicsWorld *world, btScalar timeStep);
bool ConvexResultNeedCollision(btCollisionObject* a, btCollisionObject* b){
	if (Physics::sNeedCollisionForConvexCallback){
		RigidBody* rigidBodyA = (RigidBody*)a->getUserPointer();
		RigidBody* rigidBodyB = (RigidBody*)b->getUserPointer();
		if (!rigidBodyA || !rigidBodyB)
			return true; // don't care

		return Physics::sNeedCollisionForConvexCallback(rigidBodyA, rigidBodyB);
	}

	return true;
}

std::thread::id main_thread_id;

std::thread::id Physics::get_main_thread_id() {
	return main_thread_id;
}

bool Physics::is_main_thread() {
	return main_thread_id == std::this_thread::get_id();
}

class Physics::Impl{
public:
	Physics* mSelf;
	btDefaultCollisionConfiguration* mCollisionConfiguration;
	btBroadphaseInterface*	mBroadphase;
	btCollisionDispatcher*	mDispatcher;
	btConstraintSolver*	mSolver;
	btDiscreteDynamicsWorld* mDynamicsWorld;

	std::unordered_map<std::string, btCollisionShape*> mColShapes;
	std::unordered_map<btCollisionShape*, unsigned> mColShapesRefs;
	std::unordered_map<btCollisionShape*, float> mColShapePendingDelete;

	// private functions
	friend class RigidBody;
	friend class RigidBodyImpl;
	BulletDebugDraw mDebugDrawer;

	int mRayGroup;

	static unsigned NextInternalColShapeId;
	std::unordered_map<unsigned, btCollisionShape*> mInternalShapes;

	BulletFilterCallback* mFilterCallback;
	bool mEnabled;
	std::string mPhysicsId;

	//-------------------------------------------------------------------
	Impl(Physics* self)
		: mSelf(self)
		, mRayGroup(0x40) // default of the current game under development
		, mFilterCallback(0)
		, mEnabled(true)
	{
		main_thread_id = std::this_thread::get_id();
		Initilaize();
		auto filepath = "_FBPhysics.log";
		FileSystem::BackupFile(filepath, 5, "Backup_Log");
		Logger::Init(filepath);
	}
	~Impl(){
		Deinitialize();
		Logger::Release();
	}


	void Initilaize(){
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
		mDynamicsWorld->setInternalTickCallback(TickCallback, mSelf);

		//auto& solverInfo = mDynamicsWorld->getSolverInfo();
		//solverInfo.m_splitImpulse = 0;
		//solverInfo.m_splitImpulsePenetrationThreshold = -0.02;
	}

	void Deinitialize(){
		if (mDynamicsWorld)
		{
			RegisterFilterCallback(0, 0);

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
					FB_DELETE_ALIGNED(body->getMotionState());
				}
				mDynamicsWorld->removeCollisionObject(obj);
				FB_DELETE_ALIGNED(obj);
			}

			for (auto it = mColShapePendingDelete.begin(); it != mColShapePendingDelete.end();)
			{
				auto curIt = it++;
				auto colShape = curIt->first;
				mColShapePendingDelete.erase(curIt);
				if (colShape->isCompound())
				{
					btCompoundShape* compound = (btCompoundShape*)(colShape);
					unsigned num = compound->getNumChildShapes();
					int idx = num - 1;
					while (idx >= 0)
					{
						auto shape = compound->getChildShape(idx);
						compound->removeChildShapeByIndex(idx);
						FB_DELETE_ALIGNED(shape);
						--idx;
					}
				}
				FB_DELETE_ALIGNED(colShape);
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

		FB_DELETE_ALIGNED(mDynamicsWorld);

		FB_DELETE_ALIGNED(mSolver);

		FB_DELETE_ALIGNED(mBroadphase);

		FB_DELETE_ALIGNED(mDispatcher);

		FB_DELETE_ALIGNED(mCollisionConfiguration);
	}

	void Update(float dt){
		if (mEnabled && mDynamicsWorld)
		{
			mDynamicsWorld->stepSimulation(dt, 12);
			if (mDebugDrawer.getDebugMode() != 0)
			{
				mDynamicsWorld->debugDrawWorld();
			}
		}
	}

	void EnablePhysics(){
		mEnabled = true;
	}

	void DisablePhysics(){
		mEnabled = false;
	}

	void SetPhysicsId(const char* id) {
		if (ValidCString(id))
			mPhysicsId = id;
	}

	const char* GetPhysicsId() {
		return mPhysicsId.c_str();
	}

	btCollisionShape* CreateBulletColShape(CollisionShapePtr colShape){
		switch (colShape->mType)
		{
		case CollisionShapes::Box:
		{
			auto shape = std::dynamic_pointer_cast<BoxShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a BoxShape!");
				return 0;
			}
			auto e = FBToBullet(shape->mExtent * shape->mScale);
			return FB_NEW_ALIGNED(btBoxShape, MemAlign)(e);
			break;
		}
		case CollisionShapes::Sphere:
		{
			auto shape = std::dynamic_pointer_cast<SphereShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a SphereShape");
				return 0;
			}
			return FB_NEW_ALIGNED(btSphereShape, MemAlign)(shape->mRadius * shape->mScale.x);
			break;
		}
		case CollisionShapes::Cylinder:
		{
			auto shape = std::dynamic_pointer_cast<CylinderShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a CylinderShape");
				return 0;
			}
			return FB_NEW_ALIGNED(btCylinderShape, MemAlign)(FBToBullet(shape->mExtent * shape->mScale));
			break;
		}
		case CollisionShapes::Capsule:
		{
			auto shape = std::dynamic_pointer_cast<CapsuleShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a CapsuleShape.");
				return 0;
			}
			return FB_NEW_ALIGNED(btCapsuleShape, MemAlign)(shape->mRadius * shape->mScale.x, shape->mHeight * shape->mScale.x);
			break;
		}
		case CollisionShapes::StaticMesh:
		{
			auto shape = std::dynamic_pointer_cast<MeshShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a MeshShape");
				return 0;
			}
			auto btshape = FB_NEW_ALIGNED(btBvhTriangleMeshShape, MemAlign)(shape->GetTriangleMesh(), true);
			return btshape;
		}
		case CollisionShapes::DynamicMesh:
		{
			auto shape = std::dynamic_pointer_cast<MeshShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a MeshShape");
				return 0;
			}
			auto btshape = FB_NEW_ALIGNED(btGImpactMeshShape, MemAlign)(shape->GetTriangleMesh());
			return btshape;
		}
		case CollisionShapes::Convex:
		{
			auto shape = std::dynamic_pointer_cast<MeshShape>(colShape);
			if (!shape){
				Logger::Log(FB_ERROR_LOG_ARG, "Is not a ConvexMeshShape");
				return 0;
			}
			auto btshape = FB_NEW_ALIGNED(btConvexHullShape, MemAlign)(&shape->mVertices[0].x,
				shape->mNumVertices, 12);
			return btshape;
		}
		default:
			assert(0);
		}
		return 0;
	}

	btCollisionShape* CreateColShape(IPhysicsInterface* shapeProvider){
		if (!shapeProvider){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}

		CollisionShapePtr shapes[500];
		auto num = shapeProvider->GetShapes(shapes, 500);
		if (num >= 500){
			Logger::Log(FB_ERROR_LOG_ARG, "Too many col shapes!");
		}

		return CreateColShape(shapes, num, shapeProvider->ForceCompound());
	}

	btCollisionShape* CreateColShapeForGroup(IPhysicsInterface* shapeProvider, const Vec3I& groupIdx){
		if (!shapeProvider){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}

		CollisionShapePtr shapes[500];
		auto num = shapeProvider->GetShapesForGroup(groupIdx, shapes, 500);
		if (num >= 500){
			Logger::Log(FB_ERROR_LOG_ARG, "Too many col shapes!");
		}

		return CreateColShape(shapes, num, shapeProvider->ForceCompound());
	}

	btCollisionShape* CreateColShape(CollisionShapePtr colShapes[], unsigned num, bool forceCompound){
		if (num == 0)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No collision shapes!");
			return 0;
		}
		if (forceCompound || num > 1 || colShapes[0]->mPos != Vec3::ZERO || colShapes[0]->mRot != Quat::IDENTITY)
		{
			btCompoundShape* compound = FB_NEW_ALIGNED(btCompoundShape, MemAlign);
			if (compound == 0){
				Logger::Log(FB_ERROR_LOG_ARG, "FB_NEW_ALIGNED failed!");
			}
			for (unsigned i = 0; i < num; ++i)
			{
				auto colShape = colShapes[i];
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
					if (i == 0) {
						compound->setUserPointer(colShape->mUserPtr);
					}
				}
			}
			return compound;
		}
		else
		{
			auto colShape = colShapes[0];
			auto btshape = CreateBulletColShape(colShape);
			if (btshape) {
				btshape->setUserPointer(colShape->mUserPtr);
			}
			return btshape;
		}
	}

	RigidBodyPtr _CreateRigidBodyInternal(btCollisionShape* colShape, float mass, IPhysicsInterface* obj, bool createMotionSTate){
		fbMotionState* motionState = 0;
		bool dynamic = mass != 0.0f;
		btVector3 localInertia(0, 0, 0);
		if (dynamic)
		{
			colShape->calculateLocalInertia(mass, localInertia);
			if (obj && createMotionSTate)
				motionState = FB_NEW_ALIGNED(fbMotionState, MemAlign)(obj);			
		}
		
		btRigidBody::btRigidBodyConstructionInfo rbInfo(
			mass, motionState, colShape, localInertia);
		rbInfo.m_angularDamping = obj->GetAngularDamping();
		rbInfo.m_linearDamping = obj->GetLinearDamping();		
		auto rigidBody = RigidBodyImpl::Create(rbInfo, mSelf);
		rigidBody->SetPhysicsInterface(obj);
		return rigidBody;
	}

	RigidBodyPtr CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj){
		btCollisionShape* colShape = ParseCollisionFile(collisionFile);
		return _CreateRigidBodyInternal(colShape, mass, obj, true);
	}

	RigidBodyPtr CreateRigidBody(IPhysicsInterface* obj){
		if (!obj)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "no physics interface is provided!");			
			return 0;
		}
		auto colShape = CreateColShape(obj);
		if (!colShape)
			return 0;
		return _CreateRigidBodyInternal(colShape, obj->GetMass(), obj, true);
	}

	RigidBodyPtr CreateRigidBodyForGroup(IPhysicsInterface* colProvider, const Vec3I& groupIdx){
		if (!colProvider){
			Logger::Log(FB_ERROR_LOG_ARG, "no physics interface is provided!");
			return 0;
		}

		auto colShape = CreateColShapeForGroup(colProvider, groupIdx);
		if (!colShape)
			return 0;
		auto rigidBody = _CreateRigidBodyInternal(colShape, colProvider->GetMassForGroup(groupIdx), colProvider, groupIdx == Vec3I::ZERO ? true : false);
		if (rigidBody)	{
			rigidBody->SetPhysicsInterface(colProvider, groupIdx);
		}
		return rigidBody;
	}

	RigidBodyPtr CreateTempRigidBody(CollisionShapePtr colShape){
		btVector3 localInertia(0, 0, 0);
		auto btcol = CreateBulletColShape(colShape);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, 0, btcol, localInertia);
		return RigidBodyImpl::Create(rbInfo, mSelf);		
	}

	RigidBodyPtr CreateTempRigidBody(CollisionShapePtr shapes[], unsigned num){
		btVector3 localInertia(0, 0, 0);
		auto btcol = CreateColShape(shapes, num, false);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, 0, btcol, localInertia);
		return RigidBodyImpl::Create(rbInfo, mSelf);
	}

	btCollisionShape* ParseCollisionFile(const char* collisionFile){
		if (!collisionFile || strlen(collisionFile) == 0)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "noCollision file!");
			return 0;
		}
		std::string lowerPath(collisionFile);
		ToLowerCase(lowerPath);
		auto it = mColShapes.find(lowerPath);
		if (it != mColShapes.end())
			return it->second;

		auto pdoc = FileSystem::LoadXml(collisionFile);
		tinyxml2::XMLDocument& doc = *pdoc;		
		if (doc.Error())
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Physics::ParseAction ParseCollisionFile %s : %s", collisionFile, doc.GetErrorStr1()).c_str());			
			Logger::Log(doc.GetErrorString().c_str());
			assert(0);
			return 0;
		}
		auto csElem = doc.FirstChildElement("CollisionShape");
		btCompoundShape* compoundShape = 0;
		const char* sz = csElem->Attribute("compound");
		if (sz)
		{
			if (StringConverter::ParseBool(sz))
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
					Vec3 o = StringMathConverter::ParseVec3(sz);
					origin = FBToBullet(o);
				}
				sz = shapeElem->Attribute("rot");
				if (sz)
				{
					Quat r = StringMathConverter::ParseQuat(sz);
					rot = FBToBullet(r);
				}
				btCollisionShape* btColShape = 0;
				switch (colShape)
				{
				case CollisionShapes::Box:
				{
					Vec3 extent(1, 1, 1);
					sz = shapeElem->Attribute("extent");
					extent = StringMathConverter::ParseVec3(sz);
					btColShape = FB_NEW_ALIGNED(btBoxShape, MemAlign)(FBToBullet(extent));
					break;
				}
				case CollisionShapes::Sphere:
				{
					float radius = 1.0f;
					sz = shapeElem->Attribute("radius");
					radius = StringConverter::ParseReal(sz);
					btColShape = FB_NEW_ALIGNED(btSphereShape, MemAlign)(radius);
					break;

				}
				case CollisionShapes::Cylinder:
				{
					Vec3 extent(1, 1, 1);
					sz = shapeElem->Attribute("extent");
					extent = StringMathConverter::ParseVec3(sz);
					btColShape = FB_NEW_ALIGNED(btCylinderShape, MemAlign)(FBToBullet(extent));

					break;
				}
				case CollisionShapes::Capsule:
				{
					float radius = 1.0f;
					float height = 1.0f;
					sz = shapeElem->Attribute("radius");
					radius = StringConverter::ParseReal(sz);
					sz = shapeElem->Attribute("height");
					height = StringConverter::ParseReal(sz);
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


	void RemoveConstraint(btTypedConstraint* constraint){
		mDynamicsWorld->removeConstraint(constraint);
		FB_DELETE_ALIGNED(constraint);
	}


	void AddRef(btCollisionShape* colShape){
		if (!colShape)
			return;
		auto it = mColShapesRefs.find(colShape);
		if (it == mColShapesRefs.end())
		{
			mColShapesRefs[colShape] = 1;
			return;
		}

		++it->second;
	}

	void Release(btCollisionShape* colShape){
		if (!colShape)
			return;

		auto it = mColShapesRefs.find(colShape);
		if (it == mColShapesRefs.end())
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Physics::Release() : cannot find the colshape!");
			return;
		}

		--it->second;
		if (it->second == 0)
		{
			mColShapesRefs.erase(it);
			mColShapePendingDelete[colShape] = 3.f;
		}
	}

	void SetDebugDrawer(IDebugDrawer* debugDrawer){
		mDebugDrawer.SetCallback(debugDrawer);
	}

	void SetDebugMode(int debugMode){
		mDebugDrawer.setDebugMode(debugMode);
	}



	void AttachBodies(RigidBodyPtr bodies[], unsigned num){
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
			RigidBodyImpl* body = (RigidBodyImpl*)bodies[i].get();
			for (unsigned t = i + 1; t < num; ++t)
			{
				mDynamicsWorld->contactPairTest(body, (RigidBodyImpl*)bodies[t].get(), ccallback);
			}
		}
	}

	void AttachBodiesAlways(RigidBodyPtr bodies[], unsigned num){
		if (num < 2)
			return;
		auto master = bodies[0];
		auto btMaster = dynamic_cast<btRigidBody*>(master.get());
		assert(btMaster);
		auto masterPos = btMaster->getWorldTransform().getOrigin();
		for (unsigned i = 1; i < num; ++i){
			auto target = bodies[i];
			auto btTarget = dynamic_cast<btRigidBody*>(target.get());
			assert(btTarget);
			auto targetPos = btTarget->getWorldTransform().getOrigin();
			auto center = (targetPos + masterPos)*0.5f;

			btTransform globalFrame;
			globalFrame.setIdentity();
			globalFrame.setOrigin(center);
			btTransform trA = btMaster->getWorldTransform().inverse() * globalFrame;
			btTransform trB = btTarget->getWorldTransform().inverse() * globalFrame;


			auto* fixed = FB_NEW_ALIGNED(btFixedConstraint, MemAlign)(*btMaster, *btTarget, trA, trB);
			fixed->setAngularLowerLimit(btVector3(0, 0, 0));
			fixed->setAngularUpperLimit(btVector3(0, 0, 0));
			//fixed->setOverrideNumSolverIterations(30);
			mDynamicsWorld->addConstraint(fixed, true);
		}
	}


	void SetRayCollisionGroup(int group){
		mRayGroup = group;
	}

	bool RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int additionalRayGroup, int mask, RayResultClosest& result, void* excepts[] = 0, unsigned numExcepts = 0){
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
		cb.m_collisionFilterGroup = mRayGroup + additionalRayGroup;
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

	bool RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, RayResultWithObj& result){
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

	RayResultAll* RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, int mask){
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
		cb.m_collisionFilterGroup = mRayGroup + additionalGroupFlag;
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

	void Release(RayResultAll* r){
		FB_DELETE(r);
	}

	

	unsigned GetAABBOverlaps(const AABB& aabb, unsigned colMask, 
		RigidBody* ret[], unsigned index[],
		unsigned limit, RigidBody* except)
	{
		btVector3 min = FBToBullet(aabb.GetMin());
		btVector3 max = FBToBullet(aabb.GetMax());
		AABBResultType result;
		result.reserve(limit);
		AABBOverlapCallback callback(min, max, colMask, result, limit, except);
		mDynamicsWorld->getBroadphase()->aabbTest(min, max, callback);
		for (size_t i = 0; i < result.size(); ++i) {
			ret[i] = result[i].first;
			index[i] = result[i].second;
		}

		return result.size();
	}

	float GetDistanceBetween(RigidBodyPtr a, RigidBodyPtr b, Vec3* outNormalOnB,
		Vec3* outDirToB){
		if (!a || !b)
		{
			return FLT_MAX;
		}
		float distance = FLT_MAX;
		RigidBodyImpl* aImpl = (RigidBodyImpl*)a.get();
		RigidBodyImpl* bImpl = (RigidBodyImpl*)b.get();
		auto aColShape = aImpl->getCollisionShape();
		auto bColShape = bImpl->getCollisionShape();
		btCompoundShape* aCompound = aColShape->isCompound() ? (btCompoundShape*)aColShape : 0;
		btCompoundShape* bCompound = bColShape->isCompound() ? (btCompoundShape*)bColShape : 0;
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
					if (gjkOutput.m_distance < distance) {
						distance = gjkOutput.m_distance;
						if (outNormalOnB)
							*outNormalOnB = BulletToFB(gjkOutput.m_normalOnBInWorld);
						if (outDirToB) {
							*outDirToB = BulletToFB(
								(input.m_transformB.getOrigin() - input.m_transformA.getOrigin()).normalized());
						}
					}
				}
			}
		}

		bool swapped = false;
		if (!aCompound && bCompound)
		{
			std::swap(aCompound, bCompound);
			std::swap(a, b);
			std::swap(aImpl, bImpl);
			std::swap(aColShape, bColShape);
			swapped = true;
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
				if (gjkOutput.m_distance < distance) {
					distance = gjkOutput.m_distance;
					if (outNormalOnB) {
						if (swapped) {
							*outNormalOnB = -BulletToFB(gjkOutput.m_normalOnBInWorld);
						}
						else {
							*outNormalOnB = BulletToFB(gjkOutput.m_normalOnBInWorld);
						}
					}
					if (outDirToB) {
						*outDirToB = BulletToFB(
							(input.m_transformB.getOrigin() - input.m_transformA.getOrigin()).normalized());
					}
				}				
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
			if (gjkOutput.m_distance < distance) {
				distance = gjkOutput.m_distance;
				if (outNormalOnB) {
					if (swapped) {
						*outNormalOnB = -BulletToFB(gjkOutput.m_normalOnBInWorld);
					}
					else {
						*outNormalOnB = BulletToFB(gjkOutput.m_normalOnBInWorld);
					}
				}
				if (outDirToB) {
					*outDirToB = BulletToFB(
						(input.m_transformB.getOrigin() - input.m_transformA.getOrigin()).normalized());
				}
			}
		}

		return distance;
	}


	unsigned CreateBTSphereShape(float radius){
		auto shape = FB_NEW_ALIGNED(btSphereShape, MemAlign)(radius);
		mInternalShapes[NextInternalColShapeId] = shape;
		auto ret = NextInternalColShapeId++;
		return ret;
	}

	void DeleteBTShape(unsigned id){
		auto it = mInternalShapes.find(id);
		if (it != mInternalShapes.end()){
			FB_DELETE_ALIGNED(it->second);
			mInternalShapes.erase(it);
		}
	}

	void DrawDebugInfo(){
		/*if (mDynamicsWorld){
			auto num = mDynamicsWorld->getNumCollisionObjects();
			Vec2I pos(20, 48);
			char buf[256];
			sprintf_s(buf, "Num RigidBodies : %d", num);
			gFBEnv->pRenderer->DrawText(pos, buf, Color::White);
			pos.y += 24;
			num = mDynamicsWorld->getNumConstraints();
			sprintf_s(buf, "Num Constraints : %d", num);
			gFBEnv->pRenderer->DrawText(pos, buf, Color::White);

		}*/
	}

	void ResetConstraintsSolver(){
		mDynamicsWorld->getConstraintSolver()->reset();
	}

	bool NeedToCollides(RigidBodyPtr a, RigidBodyPtr b){
		return a->CheckCollideWith(b);
	}

	void RegisterFilterCallback(IFilterCallback* callback, NeedCollisionForConvexCallback func){
		if (mFilterCallback){
			mDynamicsWorld->getPairCache()->setOverlapFilterCallback(0);
			FB_SAFE_DELETE(mFilterCallback);
			mDynamicsWorld->SetConvexResultNeedCollisionCallback(0);
		}

		if (callback){
			mFilterCallback = FB_NEW(BulletFilterCallback)(callback);
			mDynamicsWorld->getPairCache()->setOverlapFilterCallback(mFilterCallback);
		}

		Physics::sNeedCollisionForConvexCallback = func;
		mDynamicsWorld->SetConvexResultNeedCollisionCallback(ConvexResultNeedCollision);
	}

	// internal only.
	void _ReportCollisions(){
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
				a->AddCloseObjects((RigidBody*)obB);
				b->AddCloseObjects((RigidBody*)obA);
			}
			else
			{
				continue;
			}

			for (int j = 0; j < numContacts; j++)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				//if (pt.m_lifeTime == 0)
				//continue;

				btVector3 ptA = pt.getPositionWorldOnA();
				btVector3 ptB = pt.getPositionWorldOnB();
				//btVector3 normal = ptA - ptB;
				//normal.safeNormalize();
				float impulse = pt.getAppliedImpulse();
				if (impulse > 0)
				{
					IPhysicsInterface::CollisionContactInfo contactInfo((RigidBody*)obA, (RigidBody*)obB, 
						BulletToFB(pt.m_localPointA),
						BulletToFB(pt.m_localPointB),
						BulletToFB(pt.m_positionWorldOnA),
						BulletToFB(pt.m_positionWorldOnB),
						BulletToFB(pt.m_normalWorldOnB),
						pt.m_distance1,
						impulse, 
						pt.m_index0, 
						pt.m_index1);

					bool processed = a->OnCollision(contactInfo);
					if (!processed)
					{
						contactInfo.SwapAB();						
						b->OnCollision(contactInfo);
					}
					break;
				}
			}
		}
	}

	void _CheckCollisionShapeForDel(float timeStep){
		for (auto it = mColShapePendingDelete.begin(); it != mColShapePendingDelete.end();)
		{
			auto curIt = it++;
			auto colShape = curIt->first;
			curIt->second -= timeStep;
			if (curIt->second <= 0)
			{
				curIt = mColShapePendingDelete.erase(curIt);
				if (colShape->isCompound())
				{
					btCompoundShape* compound = (btCompoundShape*)(colShape);
					unsigned num = compound->getNumChildShapes();
					int idx = num - 1;
					while (idx >= 0)
					{
						auto shape = compound->getChildShape(idx);
						compound->removeChildShapeByIndex(idx);
						FB_DELETE_ALIGNED(shape);
						--idx;
					}
				}
				FB_DELETE_ALIGNED(colShape);
			}
			else
			{
				it++;
			}
		}
	}

	btDynamicsWorld* _GetDynamicWorld() const{
		return mDynamicsWorld;
	}
};

IPhysicsPtr IPhysics::Create(){
	return Physics::Create();
}

FB_IMPLEMENT_STATIC_CREATE(Physics);

NeedCollisionForConvexCallback Physics::sNeedCollisionForConvexCallback = 0;
unsigned Physics::Impl::NextInternalColShapeId = 1;

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
	auto physics = (Physics*)world->getWorldUserInfo();	
	physics->_ReportCollisions();
	physics->_CheckCollisionShapeForDel(timeStep);
}

Physics::Physics()
	: mImpl(new Impl(this)){
}

Physics::~Physics(){
	Logger::Log(FB_DEFAULT_LOG_ARG, "Physics deleted.");
}

void Physics::Initilaize() {
	mImpl->Initilaize();
}

void Physics::Deinitilaize() {
	mImpl->Deinitialize();
}

void Physics::Update(float dt) {
	mImpl->Update(dt);
}

void Physics::EnablePhysics() {
	mImpl->EnablePhysics();
}

void Physics::DisablePhysics() {
	mImpl->DisablePhysics();
}

void Physics::SetPhysicsId(const char* id) {
	mImpl->SetPhysicsId(id);
}

const char* Physics::GetPhysicsId() const {
	return mImpl->GetPhysicsId();
}

btCollisionShape* Physics::CreateColShape(IPhysicsInterface* shapeProvider){
	return mImpl->CreateColShape(shapeProvider);
}

btCollisionShape* Physics::CreateColShapeForGroup(IPhysicsInterface* shapeProvider, const Vec3I& groupIdx){
	return mImpl->CreateColShapeForGroup(shapeProvider, groupIdx);
}

btCollisionShape* Physics::CreateColShape(CollisionShapePtr colShapes[], unsigned num, bool forceCompound){
	return mImpl->CreateColShape(colShapes, num, forceCompound);
}

RigidBodyPtr Physics::CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj) {
	return mImpl->CreateRigidBody(collisionFile, mass, obj);
}

RigidBodyPtr Physics::CreateRigidBody(IPhysicsInterface* obj) {
	return mImpl->CreateRigidBody(obj);
}

RigidBodyPtr Physics::CreateRigidBodyForGroup(IPhysicsInterface* colProvider, const Vec3I& groupIdx) {
	return mImpl->CreateRigidBodyForGroup(colProvider, groupIdx);
}

RigidBodyPtr Physics::CreateTempRigidBody(CollisionShapePtr colShape) {
	return mImpl->CreateTempRigidBody(colShape);
}

RigidBodyPtr Physics::CreateTempRigidBody(CollisionShapePtr  shapes[], unsigned num) {
	return mImpl->CreateTempRigidBody(shapes, num);
}

btCollisionShape* Physics::ParseCollisionFile(const char* collisionFile) {
	return mImpl->ParseCollisionFile(collisionFile);
}

void Physics::RemoveConstraint(btTypedConstraint* constraint) {
	mImpl->RemoveConstraint(constraint);
}

void Physics::AddRef(btCollisionShape* colShape) {
	mImpl->AddRef(colShape);
}

void Physics::Release(btCollisionShape* colShape) {
	mImpl->Release(colShape);
}

void Physics::SetDebugDrawer(IDebugDrawer* debugDrawer) {
	mImpl->SetDebugDrawer(debugDrawer);
}

void Physics::SetDebugMode(int debugMode) {
	mImpl->SetDebugMode(debugMode);
}

void Physics::AttachBodies(RigidBodyPtr bodies[], unsigned num) {
	mImpl->AttachBodies(bodies, num);
}

void Physics::AttachBodiesAlways(RigidBodyPtr bodies[], unsigned num) {
	mImpl->AttachBodiesAlways(bodies, num);
}

void Physics::SetRayCollisionGroup(int group) {
	mImpl->SetRayCollisionGroup(group);
}

bool Physics::RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int additionalRayGroup, int mask, RayResultClosest& result, void* excepts[], unsigned numExcepts) {
	return mImpl->RayTestClosest(fromWorld, toWorld, additionalRayGroup, mask, result, excepts, numExcepts);
}

bool Physics::RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, RayResultWithObj& result) {
	return mImpl->RayTestWithAnObj(fromWorld, toWorld, additionalGroupFlag, result);
}

RayResultAll* Physics::RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, int mask) {
	return mImpl->RayTestAll(fromWorld, toWorld, additionalGroupFlag, mask);
}

void Physics::Release(RayResultAll* r) {
	mImpl->Release(r);
}

unsigned Physics::GetAABBOverlaps(const AABB& aabb, unsigned colMask, 
	RigidBody* ret[], unsigned index[], unsigned limit, RigidBody* except) {
	return mImpl->GetAABBOverlaps(aabb, colMask, ret, index, limit, except);
}

float Physics::GetDistanceBetween(RigidBodyPtr a, RigidBodyPtr b, Vec3* outNormalOnB,
	Vec3* outDirToB) {
	return mImpl->GetDistanceBetween(a, b, outNormalOnB, outDirToB);
}

unsigned Physics::CreateBTSphereShape(float radius) {
	return mImpl->CreateBTSphereShape(radius);
}

void Physics::DeleteBTShape(unsigned id) {
	mImpl->DeleteBTShape(id);
}

void Physics::DrawDebugInfo() {
	mImpl->DrawDebugInfo();
}

void Physics::ResetConstraintsSolver(){
	mImpl->ResetConstraintsSolver();
}

bool Physics::NeedToCollides(RigidBodyPtr a, RigidBodyPtr b){
	return mImpl->NeedToCollides(a, b);
}

//BoxShapePtr Physics::CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr) {
//	return mImpl->CreateBoxShape(pos, rot, actorScale, extent, userPtr);
//}
//
//SphereShapePtr Physics::CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr) {
//	return mImpl->CreateSphereShape(pos, rot, actorScale, radius, userPtr);
//}
//
//CylinderShapePtr Physics::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr) {
//	return mImpl->CreateCylinderShape(pos, rot, actorScale, extent, userPtr);
//}
//
//CapsuleShapePtr Physics::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr) {
//	return mImpl->CreateCylinderShape(pos, rot, actorScale, radius, height, userPtr);
//}
//
//MeshShapePtr Physics::CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,	bool staticObj, void* userPtr) {
//	return mImpl->CreateMeshShape(pos, rot, vertices, numVertices, scale, staticObj, userPtr);
//}
//
//MeshShapePtr Physics::CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,	const Vec3& scale, void* userPtr) {
//	return mImpl->CreateConvexMeshShape(pos, rot, vertices, numVertices, scale, userPtr);
//}

void Physics::RegisterFilterCallback(IFilterCallback* callback, NeedCollisionForConvexCallback func) {
	mImpl->RegisterFilterCallback(callback, func);
}

void Physics::_ReportCollisions() {
	mImpl->_ReportCollisions();
}

void Physics::_CheckCollisionShapeForDel(float timeStep) {
	mImpl->_CheckCollisionShapeForDel(timeStep);
}

btDynamicsWorld* Physics::_GetDynamicWorld() const {
	return mImpl->_GetDynamicWorld();
}

