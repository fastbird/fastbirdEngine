#include <Physics/stdafx.h>
#include <Physics/Physics.h>
#include <Physics/RigidBodyImpl.h>
#include <Physics/IPhysicsInterface.h>
#include <Physics/RotationInfo.h>

using namespace fastbird;

RigidBodyImpl::RigidBodyImpl(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider)
: btRigidBody(cinfo)
, mWorld(world)
, mPhysicsInterface(0)
, mGamePtr(0)
, mColProvider(colProvider)
, mGroupedRigidBody(false)
, mGroupIdx(Vec3I::MAX)
, mGameFlag(0)
, mAddedToWorld(false)
, mDebug(false)
{
	auto colShape = getCollisionShape();
	if (colShape)
		gFBPhysics->AddRef(colShape);
	mRotationInfo = FB_NEW(RotationInfo);
	setUserPointer(this);
}

RigidBodyImpl::~RigidBodyImpl()
{
	setUserPointer(0);
	mGamePtr = 0;
	FB_DELETE(mRotationInfo);
	RemoveConstraints();

	auto colShape = getCollisionShape();
	if (colShape)
		gFBPhysics->Release(colShape);

	if (mWorld){
		mWorld->removeRigidBody(this);
		mAddedToWorld = false;
	}
	FB_DEL_ALIGNED(getMotionState());
}

void RigidBodyImpl::RefreshColShape(IPhysicsInterface* colProvider)
{
	if (!colProvider)
	{
		Error("No colProvider!");
		return;
	}
	mWorld->removeRigidBody(this);
	mAddedToWorld = false;
	auto prevColShape = getCollisionShape();
	if (prevColShape)
		gFBPhysics->Release(prevColShape);

	btCollisionShape* colShape = 0;
	float mass = 1.0f;
	auto physics = (Physics*)gFBPhysics;
	if (mGroupedRigidBody){
		if (colProvider->GetNumColShapes(mGroupIdx) == 0)
			return;

		colShape = physics->CreateColShapeForGroup(colProvider, mGroupIdx);
		mass = colProvider->GetMassForGroup(mGroupIdx);
	}
	else{

		if (colProvider->GetNumColShapes() == 0)
			return;

		colShape = physics->CreateColShape(colProvider);
		mass = colProvider->GetMass();
	}
	assert(colShape);
	setCollisionShape(colShape);
	if (colShape)
		gFBPhysics->AddRef(colShape);	
	
	if (mass > 0 && colShape)
	{
		btVector3 inertia;
		colShape->calculateLocalInertia(mass, inertia);
		setMassProps(mass, inertia);
	}
	assert(!mAddedToWorld);
	mWorld->addRigidBody(this, colProvider->GetCollisionGroup(), colProvider->GetCollisionMask());
	mAddedToWorld = true;

}

void RigidBodyImpl::ApplyForce(const Vec3& force, const Vec3& rel_pos)
{
	if (mDebug){
		Log("ApplyForce = %.3f, %.3f, %.3f", force.x, force.y, force.z);
		if (force.Length() > 5){
			int a = 0;
			a++;
		}
	}
	applyForce(FBToBullet(force), FBToBullet(rel_pos));
	activate();
}

void RigidBodyImpl::ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos)
{
	if (mDebug){
		Log("ApplyImpulse = %.3f, %.3f, %.3f", impulse.x, impulse.y, impulse.z);
	}
	applyImpulse(FBToBullet(impulse), FBToBullet(rel_pos));
	activate();
}

void RigidBodyImpl::ApplyCentralImpulse(const Vec3& impulse)
{
	if (mDebug){
		Log("ApplyCentralImpulse = %.3f, %.3f, %.3f", impulse.x, impulse.y, impulse.z);
	}

	applyCentralImpulse(FBToBullet(impulse));
	activate();
}

void RigidBodyImpl::ApplyTorqueImpulse(const Vec3& torque)
{
	if (mDebug){
		Log("ApplyTorqueImpulse = %.3f, %.3f, %.3f", torque.x, torque.y, torque.z);
	}
	applyTorqueImpulse(FBToBullet(torque));
	activate();
}

void RigidBodyImpl::ApplyTorque(const Vec3& torque)
{
	if (mDebug){
		Log("ApplyTorque = %.3f, %.3f, %.3f", torque.x, torque.y, torque.z);
	}
	applyTorque(FBToBullet(torque));
	activate();
}

Vec3 RigidBodyImpl::GetForce()
{
	return BulletToFB(getTotalForce());
}

void RigidBodyImpl::ClearForces()
{
	clearForces();
}

float RigidBodyImpl::GetSpeed() const
{
	return getLinearVelocity().length();
}

Vec3 RigidBodyImpl::GetVelocity() const
{
	return BulletToFB(getLinearVelocity());
}

Vec3 RigidBodyImpl::GetAngularVelocity() const
{
	return BulletToFB(getAngularVelocity());
}

void RigidBodyImpl::SetAngularVelocity(const Vec3& angVel)
{
	setAngularVelocity(FBToBullet(angVel));
}

Vec3 RigidBodyImpl::GetTorque() const
{
	return BulletToFB(getTotalTorque());
}

void RigidBodyImpl::SetVelocity(const Vec3& vel)
{
	setLinearVelocity(FBToBullet(vel));
}

void RigidBodyImpl::Activate()
{
	activate();
}

void RigidBodyImpl::EnableDeactivation(bool enable)
{
	if (!enable)
	{
		if (getActivationState() != DISABLE_DEACTIVATION)
			setActivationState(DISABLE_DEACTIVATION);
	}
	else
	{
		if (getActivationState() == DISABLE_DEACTIVATION)
			forceActivationState(ACTIVE_TAG);
	}
}

Vec3 RigidBodyImpl::GetDestDir() const
{
	return BulletToFB(mRotationInfo->mDestDir);
}

void RigidBodyImpl::Update(float dt)
{
}


void RigidBodyImpl::SetMass(float mass)
{
	btVector3 inertia;
	getCollisionShape()->calculateLocalInertia(mass, inertia);
	setMassProps(mass, inertia);
}

btScalar RigidBodyImpl::addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
{
	

	return 1.f;
}

void RigidBodyImpl::SetPhysicsInterface(IPhysicsInterface* obj)
{
	mPhysicsInterface = obj;
	setUserPointer(this); // to avoid dynamic_cast

	// save actor;
	mGamePtr = obj->GetUserPtr();
}

void RigidBodyImpl::SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx){
	mPhysicsInterface = obj;
	setUserPointer(this); // to avoid dynamic_cast

	// save actor;
	mGamePtr = obj->GetUserPtr();

	mGroupIdx = groupIdx;
	mGroupedRigidBody = true;
}

IPhysicsInterface* RigidBodyImpl::GetPhysicsInterface() const
{
	return mPhysicsInterface;
}

void* RigidBodyImpl::GetColShapeUserPtr(int idx)
{
	auto colShape = getCollisionShape();
	assert(colShape);
	if (colShape->isCompound())
	{
		btCompoundShape* compound = (btCompoundShape*)colShape;
		if(idx < compound->getNumChildShapes() && idx>=0)
		{
			auto child = compound->getChildShape(idx);
			if (child)
			{
				return child->getUserPointer();
			}
		}
	}
	else
	{
		return colShape->getUserPointer();
	}
	return 0;
}

void* RigidBodyImpl::GetGamePtr() const
{
	return mGamePtr;
}

void RigidBodyImpl::SetRotationalForce(float force)
{
	mRotationInfo->mForce = force;
}


void RigidBodyImpl::SetCollisionFilter(unsigned group)
{
	if (getBroadphaseHandle())
		getBroadphaseHandle()->m_collisionFilterGroup = group;
}

void RigidBodyImpl::RemoveCollisionFilter(unsigned flag)
{
	if (getBroadphaseHandle())
		getBroadphaseHandle()->m_collisionFilterGroup = getBroadphaseHandle()->m_collisionFilterGroup & ~flag;
}

void RigidBodyImpl::AddCollisionFilter(unsigned flag)
{
	if (getBroadphaseHandle())
		getBroadphaseHandle()->m_collisionFilterGroup |= flag;
}

void RigidBodyImpl::SetColMask(unsigned mask)
{
	if (getBroadphaseHandle())
		getBroadphaseHandle()->m_collisionFilterMask = mask;
}

unsigned RigidBodyImpl::GetColMask()  const
{
	if (getBroadphaseHandle())
		return getBroadphaseHandle()->m_collisionFilterMask;
	else
		return -1;
}

void RigidBodyImpl::SetLinearDamping(float damping)
{
	float an = getAngularDamping();
	setDamping(damping, an);
}

void RigidBodyImpl::SetAngularDamping(float damping)
{
	float linear = getLinearDamping();
	setDamping(linear, damping);
}

void RigidBodyImpl::SetDamping(float linear, float angular)
{
	setDamping(linear, angular);
}

unsigned RigidBodyImpl::HasContact(void* gamePtrs[], int limit)
{
	struct Callback : public btCollisionWorld::ContactResultCallback
	{
		std::vector<RigidBody*> mCollided;
		RigidBodyImpl* mMe;

		Callback(RigidBodyImpl* me)
			:mHasCollision(false), mMe(me)
		{

		}

		virtual bool needsCollision(btBroadphaseProxy* proxy) const {
			// superclass will check m_collisionFilterGroup and m_collisionFilterMask
			if (!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
				return false;
			// if passed filters, may also want to avoid contacts between constraints
			return mMe->checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
		}

		bool mHasCollision;
		virtual	btScalar	addSingleResult(btManifoldPoint& cp, 
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, 
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
		{
			//if (!colObj0Wrap->m_collisionObject->checkCollideWith(colObj1Wrap->m_collisionObject) || 
			//	!colObj1Wrap->m_collisionObject->checkCollideWith(colObj0Wrap->m_collisionObject))
			//	return 1.0f;
			if (cp.getDistance() < 0.05f)
			{
				mHasCollision = true;
				if (colObj0Wrap->m_collisionObject->getUserPointer() == mMe)
				{
					mCollided.push_back((RigidBody*)colObj1Wrap->m_collisionObject->getUserPointer());
				}
				else if (colObj1Wrap->m_collisionObject->getUserPointer() == mMe)
				{
					mCollided.push_back((RigidBody*)colObj0Wrap->m_collisionObject->getUserPointer());
				}
				
			}
			return 1.0f;
		}
	};
	Callback callback(this);
	Physics* physics = (Physics*)gFBPhysics;
	physics->_GetDynamicWorld()->contactTest(this, callback);
	unsigned num = 0;
	if (gamePtrs && !callback.mCollided.empty())
	{
		for (auto& rigidBody : callback.mCollided){
			gamePtrs[num++] = rigidBody->GetGamePtr();
			if (num >= (unsigned)limit)
				break;
		}
	}
	else{
		num = callback.mCollided.size();
	}
	return num;
}

void RigidBodyImpl::RemoveRigidBodyFromWorld()
{
	this->forceActivationState(WANTS_DEACTIVATION);
}

void RigidBodyImpl::ReAddRigidBodyFromWorld()
{
	this->forceActivationState(ACTIVE_TAG);
}

void RigidBodyImpl::ModifyCollisionFlag(int flag, bool enable)
{
	if (enable)
	{ 
		setCollisionFlags(getCollisionFlags() | flag);
	}
	else
	{
		setCollisionFlags(getCollisionFlags() & ~flag);
	}	
}

void RigidBodyImpl::SetCCDMotionThreshold(float threshold)
{
	setCcdMotionThreshold(threshold);
}

void RigidBodyImpl::SetCCDSweptSphereRadius(float radius)
{
	setCcdSweptSphereRadius(radius);
}

void RigidBodyImpl::SetIgnoreCollisionCheck(RigidBody* rigidBody, bool ignore)
{
	auto colObj = dynamic_cast<btCollisionObject*>(rigidBody);
	assert(colObj);
	if (ignore){
		int i = m_objectsWithoutCollisionCheck.findLinearSearch(colObj);
		if (i == m_objectsWithoutCollisionCheck.size()){
			setIgnoreCollisionCheck(colObj, ignore);
		}
	}
	else{
		setIgnoreCollisionCheck(colObj, ignore);
	}
}

void RigidBodyImpl::SetTransform(const Transformation& t)
{
	Mat44 mat4;
	t.GetHomogeneous(mat4);
	auto btT = FBToBullet(mat4);
	setWorldTransform(btT);
	activate();

}

Vec3 RigidBodyImpl::GetPos() const{
	return BulletToFB(getWorldTransform().getOrigin());
}

void RigidBodyImpl::RegisterToWorld()
{
	if (mWorld && mColProvider){
		assert(!mAddedToWorld);
		mWorld->addRigidBody(this, mColProvider->GetCollisionGroup(), mColProvider->GetCollisionMask());
		mAddedToWorld = true;
		auto num = getNumConstraintRefs();
		for (int i = 0; i < num; ++i){
			auto constraint = getConstraintRef(i);
			assert(constraint);
			if (constraint->getRigidBodyA().isInWorld() && constraint->getRigidBodyB().isInWorld())
				constraint->setEnabled(true);			
		}
	}
	else
		Error(DEFAULT_DEBUG_ARG, "No colprovier exists!");
}

void RigidBodyImpl::UnregisterFromWorld(){
	if (mWorld){
		auto num = getNumConstraintRefs();
		for (int i = 0; i < num; ++i){
			auto constraint = getConstraintRef(i);
			assert(constraint);
			constraint->setEnabled(false);
		}
		mWorld->removeRigidBody(this);
		mAddedToWorld = false;
	}
	else
		Error(DEFAULT_DEBUG_ARG, "No colprovier exists!");
}

void RigidBodyImpl::SetKinematic(bool enable)
{
	if (enable)
	{
		int colFlag = getCollisionFlags() | CF_KINEMATIC_OBJECT;
		setCollisionFlags(colFlag);
	}
	else
	{
		int colFlag = getCollisionFlags() & ~CF_KINEMATIC_OBJECT;
		setCollisionFlags(colFlag);
	}
}

void RigidBodyImpl::RemoveConstraints(){
	while (getNumConstraintRefs()>0)
	{
		btTypedConstraint* pConstraint = getConstraintRef(0);
		gFBPhysics->RemoveConstraint(pConstraint);

	}
}

void RigidBodyImpl::RemoveConstraint(void* constraintPtr){
	auto num = getNumConstraintRefs();
	for (int i = 0; i < num; ++i){
		btTypedConstraint* pConstraint = getConstraintRef(i);
		if (pConstraint == constraintPtr){
			gFBPhysics->RemoveConstraint(pConstraint);
			break;
		}
	}
}

void* RigidBodyImpl::GetLastConstraintsPtr(){
	auto num = getNumConstraintRefs();
	if (num == 0)
		return 0;
	return getConstraintRef(num - 1);
}

void RigidBodyImpl::SetGameFlag(unsigned flag){
	mGameFlag = flag;
}

unsigned RigidBodyImpl::GetGameFlag() const{
	return mGameFlag;
}

void RigidBodyImpl::SetDebug(bool debug){
	mDebug = debug;
	btSetDebug();
}