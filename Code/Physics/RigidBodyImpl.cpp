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
{
	assert(world);
	auto colShape = getCollisionShape();
	if (colShape)
		IPhysics::GetPhysics()->AddRef(colShape);

	mWorld->addRigidBody(this, colProvider->GetCollisionGroup(), colProvider->GetCollisionMask());
	mRotationInfo = FB_NEW(RotationInfo);
	setUserPointer(0);
}

RigidBodyImpl::~RigidBodyImpl()
{
	setUserPointer(0);
	mGamePtr = 0;
	FB_DELETE(mRotationInfo);
	while(getNumConstraintRefs()>0)
	{
		btTypedConstraint* pConstraint = getConstraintRef(0);
		IPhysics::GetPhysics()->RemoveConstraint(pConstraint);
		
	}

	auto colShape = getCollisionShape();
	if (colShape)
		IPhysics::GetPhysics()->Release(colShape);

	mWorld->removeRigidBody(this);
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
	auto prevColShape = getCollisionShape();
	if (prevColShape)
		IPhysics::GetPhysics()->Release(prevColShape);

	auto physics = (Physics*)IPhysics::GetPhysics();
	auto numColShapes = colProvider->GetNumColShapes();
	if (numColShapes == 0)
	{
		return;
	}
	auto colShape = physics->CreateColShape(colProvider);
	assert(colShape);
	setCollisionShape(colShape);
	if (colShape)
		IPhysics::GetPhysics()->AddRef(colShape);


	float mass = colProvider->GetMass();
	if (mass > 0 && colShape)
	{
		btVector3 inertia;
		colShape->calculateLocalInertia(mass, inertia);
		setMassProps(mass, inertia);
	}
	mWorld->addRigidBody(this, colProvider->GetCollisionGroup(), colProvider->GetCollisionMask());
}

void RigidBodyImpl::ApplyForce(const Vec3& force, const Vec3& rel_pos)
{
	activate();
	applyForce(FBToBullet(force), FBToBullet(rel_pos));
}

void RigidBodyImpl::ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos)
{
	activate();
	applyImpulse(FBToBullet(impulse), FBToBullet(rel_pos));
}

void RigidBodyImpl::ApplyCentralImpulse(const Vec3& impulse)
{
	activate();
	applyCentralImpulse(FBToBullet(impulse));
}

void RigidBodyImpl::ApplyTorqueImpulse(const Vec3& torque)
{
	activate();
	applyTorqueImpulse(FBToBullet(torque));
}

void RigidBodyImpl::ApplyTorque(const Vec3& torque)
{
	activate();
	applyTorque(FBToBullet(torque));
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
		if_assert_pass(idx < compound->getNumChildShapes() && idx>=0)
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

	assert(0);
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

void RigidBodyImpl::RemoveCollisionFilter(unsigned flag)
{
	getBroadphaseHandle()->m_collisionFilterGroup = getBroadphaseHandle()->m_collisionFilterGroup & ~flag;
}

void RigidBodyImpl::AddCollisionFilter(unsigned flag)
{
	getBroadphaseHandle()->m_collisionFilterGroup |= flag;
}

void RigidBodyImpl::SetColMask(unsigned mask)
{
	getBroadphaseHandle()->m_collisionFilterMask = mask;
}

unsigned RigidBodyImpl::GetColMask()  const
{
	return getBroadphaseHandle()->m_collisionFilterMask;
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

bool RigidBodyImpl::HasContact(void** gamePtr)
{
	struct Callback : public btCollisionWorld::ContactResultCallback
	{
		RigidBody* mCollided;
		RigidBodyImpl* mMe;

		Callback(RigidBodyImpl* me)
			:mHasCollision(false), mCollided(0), mMe(me)
		{

		}

		bool mHasCollision;
		virtual	btScalar	addSingleResult(btManifoldPoint& cp, 
			const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, 
			const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
		{
			if (!colObj0Wrap->m_collisionObject->checkCollideWith(colObj1Wrap->m_collisionObject) || 
				!colObj1Wrap->m_collisionObject->checkCollideWith(colObj0Wrap->m_collisionObject))
				return 1.0f;
			if (cp.getDistance() < 0.05f)
			{
				mHasCollision = true;
				if (colObj0Wrap->m_collisionObject->getUserPointer() == mMe)
				{
					mCollided = (RigidBody*)colObj1Wrap->m_collisionObject->getUserPointer();
				}
				else if (colObj1Wrap->m_collisionObject->getUserPointer() == mMe)
				{
					mCollided = (RigidBody*)colObj0Wrap->m_collisionObject->getUserPointer();
				}
				
			}
			return 1.0f;
		}
	};
	Callback callback(this);
	mWorld->contactTest(this, callback);
	if (gamePtr && callback.mCollided)
	{
		*gamePtr = callback.mCollided->GetGamePtr();
	}
	return callback.mHasCollision;
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
	RigidBodyImpl* rigid = (RigidBodyImpl*)rigidBody;
	setIgnoreCollisionCheck(rigid, ignore);
}