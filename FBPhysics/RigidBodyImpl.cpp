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
#include "RigidBodyImpl.h"
#include "Physics.h"
#include "IPhysicsInterface.h"
#include "RotationInfo.h"
#include "IPhysics.h"
using namespace fb;

class RigidBodyImpl::Impl{
public:
	RigidBodyImpl* mSelf;
	btDiscreteDynamicsWorld* mWorld;

	RotationInfo* mRotationInfo;
	IPhysicsInterface* mColProvider;


	// Game object. (class PhysicsComp for my game.)
	IPhysicsInterface* mPhysicsInterface;

	void* mGamePtr; // save actor.
	Vec3I mGroupIdx;
	bool mGroupedRigidBody;

	unsigned mGameFlag;

	bool mAddedToWorld;
	bool mDebug;

	//---------------------------------------------------------------------------
	Impl(RigidBodyImpl* self, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider)
		: mSelf(self)
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
		auto colShape = mSelf->getCollisionShape();
		if (colShape)
		{
			Physics::GetInstance().AddRef(colShape);
		}
		mRotationInfo = FB_NEW(RotationInfo);
		mSelf->setUserPointer(self);
	}
	~Impl(){
		UnregisterFromWorld();
		mSelf->setUserPointer(0);
		mGamePtr = 0;
		FB_DELETE(mRotationInfo);
		RemoveConstraints();

		auto colShape = mSelf->getCollisionShape();
		if (colShape){
			Physics::GetInstance().Release(colShape);
		}

		if (mWorld){
			mWorld->removeRigidBody(mSelf);
			mAddedToWorld = false;
		}
		FB_DELETE_ALIGNED(mSelf->getMotionState());
	}

	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1){
		return 1.f;
	}

	// Physics dll internal
	void RefreshColShape(IPhysicsInterface* colProvider){
		if (!colProvider)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No colProvider");			
			return;
		}
		mWorld->removeRigidBody(mSelf);
		mAddedToWorld = false;
		auto prevColShape = mSelf->getCollisionShape();
		if (prevColShape){
			Physics::GetInstance().Release(prevColShape);
		}

		btCollisionShape* colShape = 0;
		float mass = 1.0f;
		auto& physics = Physics::GetInstance();
		if (mGroupedRigidBody){
			if (colProvider->GetNumColShapes(mGroupIdx) == 0)
				return;

			colShape = physics.CreateColShapeForGroup(colProvider, mGroupIdx);
			mass = colProvider->GetMassForGroup(mGroupIdx);
		}
		else{
			if (colProvider->GetNumColShapes() == 0)
				return;

			colShape = physics.CreateColShape(colProvider);
			mass = colProvider->GetMass();
		}
		assert(colShape);
		mSelf->setCollisionShape(colShape);
		if (colShape)
			physics.AddRef(colShape);

		if (mass > 0 && colShape)
		{
			btVector3 inertia;
			colShape->calculateLocalInertia(mass, inertia);			
			mSelf->setMassProps(mass, inertia);
		}
		else{
			SetMass(0.f);
		}
		assert(!mAddedToWorld);
		mWorld->addRigidBody(mSelf, colProvider->GetCollisionGroup(), colProvider->GetCollisionMask());
		mAddedToWorld = true;
	}

	void ApplyForce(const Vec3& force, const Vec3& rel_pos){
		if (mDebug){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("ApplyForce = %.3f, %.3f, %.3f", force.x, force.y, force.z).c_str());			
			if (force.Length() > 5){
				int a = 0;
				a++;
			}
		}
		mSelf->applyForce(FBToBullet(force), FBToBullet(rel_pos));
		mSelf->activate();
	}

	void ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos){
		if (mDebug){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("ApplyImpulse = %.3f, %.3f, %.3f", impulse.x, impulse.y, impulse.z).c_str());			
		}
		mSelf->applyImpulse(FBToBullet(impulse), FBToBullet(rel_pos));
		mSelf->activate();
	}

	void ApplyCentralImpulse(const Vec3& impulse){
		if (mDebug){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("ApplyCentralImpulse = %.3f, %.3f, %.3f", impulse.x, impulse.y, impulse.z).c_str());			
		}

		mSelf->applyCentralImpulse(FBToBullet(impulse));
		mSelf->activate();
	}

	void ApplyTorqueImpulse(const Vec3& torque){
		if (mDebug){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("ApplyTorqueImpulse = %.3f, %.3f, %.3f", torque.x, torque.y, torque.z).c_str());			
		}
		mSelf->applyTorqueImpulse(FBToBullet(torque));
		mSelf->activate();
	}

	void ApplyTorque(const Vec3& torque){
		if (mDebug){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("ApplyTorque = %.3f, %.3f, %.3f", torque.x, torque.y, torque.z).c_str());			
		}
		mSelf->applyTorque(FBToBullet(torque));
		mSelf->activate();
	}

	Vec3 GetForce(){
		return BulletToFB(mSelf->getTotalForce());
	}

	void ClearForces(){
		mSelf->clearForces();
	}

	void Stop(){
		mSelf->setAngularVelocity(btVector3(0, 0, 0));
		mSelf->setLinearVelocity(btVector3(0, 0, 0));
		mSelf->clearForces();
	}

	float GetSpeed() const{
		return mSelf->getLinearVelocity().length();
	}

	Vec3 GetVelocity() const{
		return BulletToFB(mSelf->getLinearVelocity());
	}

	Vec3 GetAngularVelocity() const{
		return BulletToFB(mSelf->getAngularVelocity());
	}

	void SetAngularVelocity(const Vec3& angVel){
		mSelf->setAngularVelocity(FBToBullet(angVel));
	}

	Vec3 GetTorque() const{
		return BulletToFB(mSelf->getTotalTorque());
	}

	void SetVelocity(const Vec3& vel){
		mSelf->setLinearVelocity(FBToBullet(vel));
	}

	void Activate(){
		mSelf->activate();
	}

	void EnableDeactivation(bool enable){
		if (!enable)
		{
			if (mSelf->getActivationState() != DISABLE_DEACTIVATION)
				mSelf->setActivationState(DISABLE_DEACTIVATION);
		}
		else
		{
			if (mSelf->getActivationState() == DISABLE_DEACTIVATION)
				mSelf->forceActivationState(ACTIVE_TAG);
		}
	}

	Vec3 GetDestDir() const{
		return BulletToFB(mRotationInfo->mDestDir);
	}

	void SetMass(float mass){
		btVector3 inertia;
		mSelf->getCollisionShape()->calculateLocalInertia(mass, inertia);		
		mSelf->setMassProps(mass, inertia);
	}


	void SetPhysicsInterface(IPhysicsInterface* obj){
		mPhysicsInterface = obj;
		mSelf->setUserPointer(mSelf); // to avoid dynamic_cast

		// save actor;
		mGamePtr = obj->GetUserPtr();
	}

	void SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx){
		mPhysicsInterface = obj;
		mSelf->setUserPointer(mSelf); // to avoid dynamic_cast

		// save actor;
		mGamePtr = obj->GetUserPtr();

		mGroupIdx = groupIdx;
		mGroupedRigidBody = true;
	}

	IPhysicsInterface* GetPhysicsInterface() const{
		return mPhysicsInterface;
	}


	void* GetColShapeUserPtr(int idx = 0){
		auto colShape = mSelf->getCollisionShape();
		assert(colShape);
		if (colShape->isCompound())
		{
			btCompoundShape* compound = (btCompoundShape*)colShape;
			if (idx < compound->getNumChildShapes() && idx >= 0)
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

	void* GetGamePtr() const{
		return mGamePtr;
	}

	void SetRotationalForce(float force){
		mRotationInfo->mForce = force;
	}


	void SetCollisionFilterGroup(unsigned group){
		mWorld->removeRigidBody(mSelf);
		mWorld->addRigidBody(mSelf, group, mColProvider->GetCollisionMask());
	}

	void RemoveCollisionFilterGroup(unsigned flag){
		if (mSelf->getBroadphaseHandle())
			mSelf->getBroadphaseHandle()->m_collisionFilterGroup = mSelf->getBroadphaseHandle()->m_collisionFilterGroup & ~flag;
	}

	void AddCollisionFilter(unsigned flag){
		if (mSelf->getBroadphaseHandle())
			mSelf->getBroadphaseHandle()->m_collisionFilterGroup |= flag;
	}

	void SetColMask(unsigned mask){
		if (mSelf->getBroadphaseHandle())
			mSelf->getBroadphaseHandle()->m_collisionFilterMask = mask;
	}

	unsigned GetColMask() const{
		if (mSelf->getBroadphaseHandle())
			return mSelf->getBroadphaseHandle()->m_collisionFilterMask;
		else
			return -1;
	}


	void SetLinearDamping(float damping){
		float an = mSelf->getAngularDamping();
		mSelf->setDamping(damping, an);
	}

	void SetAngularDamping(float damping){
		float linear = mSelf->getLinearDamping();
		mSelf->setDamping(linear, damping);
	}

	void SetDamping(float linear, float angular){
		mSelf->setDamping(linear, angular);
	}


	unsigned HasContact(void* gamePtrs[], int limit){
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
		Callback callback(mSelf);
		auto& physics = Physics::GetInstance();		
		physics._GetDynamicWorld()->contactTest(mSelf, callback);
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


	void RemoveFromWorld(){
		mWorld->removeRigidBody(mSelf);
	}

	// make sure mColProvider is valid.
	void AddToWorld(){
		mWorld->removeRigidBody(mSelf);
		mWorld->addRigidBody(mSelf, mColProvider->GetCollisionGroup(), mColProvider->GetCollisionMask());		
	}

	void ReaddToWorld() {
		AddToWorld();
	}

	void ModifyCollisionFlag(int flag, bool enable){
		if (enable)
		{
			mSelf->setCollisionFlags(mSelf->getCollisionFlags() | flag);
		}
		else
		{
			mSelf->setCollisionFlags(mSelf->getCollisionFlags() & ~flag);
		}
	}

	void SetCCDMotionThreshold(float threshold){
		mSelf->setCcdMotionThreshold(threshold);
	}

	void SetCCDSweptSphereRadius(float radius){
		mSelf->setCcdSweptSphereRadius(radius);
	}

	void SetIgnoreCollisionCheck(RigidBodyPtr rigidBody, bool ignore){
		auto colObj = dynamic_cast<btCollisionObject*>(rigidBody.get());
		if (mSelf->checkCollideWithOverride(colObj) == ignore){
			mSelf->setIgnoreCollisionCheck(colObj, ignore);
		}
	}

	void SetTransform(const btTransform& aT, VectorMap<void*, int>& set){
		if (set.Find(mSelf) != set.end())
			return;
		mSelf->setWorldTransform(aT);
		mSelf->clearForces();
		set[mSelf] = 1;
		if (mGameFlag != 0)
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) Setting rigid body transform for %d", mGameFlag).c_str());
		auto numConstraints = mSelf->getNumConstraintRefs();
		for (int i = 0; i < numConstraints; ++i){
			auto con = mSelf->getConstraintRef(i);
			auto conType = con->getConstraintType();
			if (conType == FIXED_CONSTRAINT_TYPE || conType == D6_SPRING_2_CONSTRAINT_TYPE){
				btFixedConstraint* fixedCon = (btFixedConstraint*)con;
				auto a = &con->getRigidBodyA();
				auto b = &con->getRigidBodyB();
				auto trA = fixedCon->getFrameOffsetA();
				auto trB = fixedCon->getFrameOffsetB();
				if (b->getUserPointer() == mSelf){
					std::swap(a, b);
					std::swap(trA, trB);
				}
				auto bT = aT * trA * trB.inverse();
				auto rigidBodyImpl = (RigidBodyImpl*)b->getUserPointer();
				rigidBodyImpl->mImpl->SetTransform(bT, set);				
			}
		}
		mSelf->activate();
	}

	void SetTransform(const Transformation& t){		
		Mat44 mat4;
		t.GetHomogeneous(mat4);
		auto aT = FBToBullet(mat4);
		VectorMap<void*, int> mSet;
		SetTransform(aT, mSet);
	}

	Vec3 GetPos() const{
		return BulletToFB(mSelf->getWorldTransform().getOrigin());
	}


	void RegisterToWorld(){
		if (mAddedToWorld) {
			Logger::Log(FB_ERROR_LOG_ARG, "Already registered rigid body.");
			return;
		}

		if (mWorld && mColProvider){			
			mWorld->addRigidBody(mSelf, mColProvider->GetCollisionGroup(), mColProvider->GetCollisionMask());
			mAddedToWorld = true;
			auto num = mSelf->getNumConstraintRefs();
			for (int i = 0; i < num; ++i){
				auto constraint = mSelf->getConstraintRef(i);
				assert(constraint);
				if (constraint->getRigidBodyA().isInWorld() && constraint->getRigidBodyB().isInWorld())
					constraint->setEnabled(true);
			}
		}
		else
			Logger::Log(FB_ERROR_LOG_ARG, "No colprovier exists!");
	}

	void UnregisterFromWorld(){
		if (mWorld){
			auto num = mSelf->getNumConstraintRefs();
			for (int i = 0; i < num; ++i){
				auto constraint = mSelf->getConstraintRef(i);
				assert(constraint);
				constraint->setEnabled(false);
			}
			mWorld->removeRigidBody(mSelf);
			mAddedToWorld = false;
		}		
	}


	void SetKinematic(bool enable){
		if (enable)
		{
			int colFlag = mSelf->getCollisionFlags() | CF_KINEMATIC_OBJECT;
			mSelf->setCollisionFlags(colFlag);
		}
		else
		{
			int colFlag = mSelf->getCollisionFlags() & ~CF_KINEMATIC_OBJECT;
			mSelf->setCollisionFlags(colFlag);
		}
	}


	const Vec3I& GetGroupIdx() const {
		return mGroupIdx;
	}

	bool IsGrouped() const { 
		return mGroupedRigidBody;
	}

	void RemoveConstraints(){
		while (mSelf->getNumConstraintRefs()>0)
		{
			btTypedConstraint* pConstraint = mSelf->getConstraintRef(0);
			Physics::GetInstance().RemoveConstraint(pConstraint);
		}
	}

	void RemoveConstraint(void* constraintPtr){
		auto num = mSelf->getNumConstraintRefs();
		for (int i = 0; i < num; ++i){
			btTypedConstraint* pConstraint = mSelf->getConstraintRef(i);
			if (pConstraint == constraintPtr){
				Physics::GetInstance().RemoveConstraint(pConstraint);
				break;
			}
		}
	}

	void RemoveConstraintsFor(void* gamePtr){
		auto num = mSelf->getNumConstraintRefs();
		for (int i = 0; i < num; ){
			btTypedConstraint* pConstraint = mSelf->getConstraintRef(i);
			if (pConstraint->getRigidBodyA().getUserPointer() ==
				pConstraint->getRigidBodyB().getUserPointer()){
				Physics::GetInstance().RemoveConstraint(pConstraint);
			}
			else{
				++i;
			}
		}
	}

	void* GetLastConstraintsPtr(){
		auto num = mSelf->getNumConstraintRefs();
		if (num == 0)
			return 0;
		return mSelf->getConstraintRef(num - 1);
	}


	void SetGameFlag(unsigned flag){
		mGameFlag = flag;
	}

	unsigned GetGameFlag() const{
		return mGameFlag;
	}


	void SetDebug(bool debug){
		mDebug = debug;
		mSelf->btSetDebug(debug);
	}

	bool CheckCollideWith(RigidBodyPtr other){
		return mSelf->checkCollideWith((RigidBodyImpl*)other.get());
	}
};

//---------------------------------------------------------------------------
RigidBodyImplPtr RigidBodyImpl::Create(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider){
	return RigidBodyImplPtr(FB_NEW_ALIGNED(RigidBodyImpl, IPhysics::MemAlign)(cinfo, world, colProvider), [](RigidBodyImpl* obj){ FB_DELETE_ALIGNED(obj); });
}

RigidBodyImpl::RigidBodyImpl(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider)
: btRigidBody(cinfo)
, mImpl(new Impl(this, world, colProvider))
{
	
}

btScalar RigidBodyImpl::addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, 
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1){
	return mImpl->addSingleResult(cp, colObj0Wrap, partId0, index0,
		colObj1Wrap, partId1, index1);
}

void RigidBodyImpl::RefreshColShape(IPhysicsInterface* colProvider) {
	mImpl->RefreshColShape(colProvider);
}

void RigidBodyImpl::ApplyForce(const Vec3& force, const Vec3& rel_pos) {
	mImpl->ApplyForce(force, rel_pos);
}

void RigidBodyImpl::ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos) {
	mImpl->ApplyImpulse(impulse, rel_pos);
}

void RigidBodyImpl::ApplyCentralImpulse(const Vec3& impulse) {
	mImpl->ApplyCentralImpulse(impulse);
}

void RigidBodyImpl::ApplyTorqueImpulse(const Vec3& torque) {
	mImpl->ApplyTorqueImpulse(torque);
}

void RigidBodyImpl::ApplyTorque(const Vec3& torque) {
	mImpl->ApplyTorque(torque);
}

Vec3 RigidBodyImpl::GetForce() {
	return mImpl->GetForce();
}

void RigidBodyImpl::ClearForces() {
	mImpl->ClearForces();
}

void RigidBodyImpl::Stop() {
	mImpl->Stop();
}

float RigidBodyImpl::GetSpeed() const {
	return mImpl->GetSpeed();
}

Vec3 RigidBodyImpl::GetVelocity() const {
	return mImpl->GetVelocity();
}

Vec3 RigidBodyImpl::GetAngularVelocity() const {
	return mImpl->GetAngularVelocity();
}

void RigidBodyImpl::SetAngularVelocity(const Vec3& angVel) {
	mImpl->SetAngularVelocity(angVel);
}

Vec3 RigidBodyImpl::GetTorque() const {
	return mImpl->GetTorque();
}

void RigidBodyImpl::SetVelocity(const Vec3& vel) {
	mImpl->SetVelocity(vel);
}

void RigidBodyImpl::Activate() {
	mImpl->Activate();
}

void RigidBodyImpl::EnableDeactivation(bool enable) {
	mImpl->EnableDeactivation(enable);
}

Vec3 RigidBodyImpl::GetDestDir() const {
	return mImpl->GetDestDir();
}

void RigidBodyImpl::SetMass(float mass) {
	mImpl->SetMass(mass);
}

void RigidBodyImpl::SetPhysicsInterface(IPhysicsInterface* obj) {
	mImpl->SetPhysicsInterface(obj);
}

void RigidBodyImpl::SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx) {
	mImpl->SetPhysicsInterface(obj, groupIdx);
}

IPhysicsInterface* RigidBodyImpl::GetPhysicsInterface() const {
	return mImpl->GetPhysicsInterface();
}

void* RigidBodyImpl::GetColShapeUserPtr(int idx) {
	return mImpl->GetColShapeUserPtr(idx);
}

void* RigidBodyImpl::GetGamePtr() const {
	return mImpl->GetGamePtr();
}

void RigidBodyImpl::SetRotationalForce(float force) {
	mImpl->SetRotationalForce(force);
}

void RigidBodyImpl::SetCollisionFilterGroup(unsigned group) {
	mImpl->SetCollisionFilterGroup(group);
}

void RigidBodyImpl::RemoveCollisionFilterGroup(unsigned flag) {
	mImpl->RemoveCollisionFilterGroup(flag);
}

void RigidBodyImpl::AddCollisionFilter(unsigned flag) {
	mImpl->AddCollisionFilter(flag);
}

void RigidBodyImpl::SetColMask(unsigned mask) {
	mImpl->SetColMask(mask);
}

unsigned RigidBodyImpl::GetColMask() const {
	return mImpl->GetColMask();
}

void RigidBodyImpl::SetLinearDamping(float damping) {
	mImpl->SetLinearDamping(damping);
}

void RigidBodyImpl::SetAngularDamping(float damping) {
	mImpl->SetAngularDamping(damping);
}

void RigidBodyImpl::SetDamping(float linear, float angular) {
	mImpl->SetDamping(linear, angular);
}

unsigned RigidBodyImpl::HasContact(void* gamePtrs[], int limit) {
	return mImpl->HasContact(gamePtrs, limit);
}

void RigidBodyImpl::RemoveFromWorld() {
	mImpl->RemoveFromWorld();
}

void RigidBodyImpl::AddToWorld() {
	mImpl->AddToWorld();
}

void RigidBodyImpl::ReaddToWorld() {
	mImpl->ReaddToWorld();
}

void RigidBodyImpl::ModifyCollisionFlag(int flag, bool enable) {
	mImpl->ModifyCollisionFlag(flag, enable);
}

void RigidBodyImpl::SetCCDMotionThreshold(float threshold) {
	mImpl->SetCCDMotionThreshold(threshold);
}

void RigidBodyImpl::SetCCDSweptSphereRadius(float radius) {
	mImpl->SetCCDSweptSphereRadius(radius);
}

void RigidBodyImpl::SetIgnoreCollisionCheck(RigidBodyPtr rigidBody, bool ignore) {
	mImpl->SetIgnoreCollisionCheck(rigidBody, ignore);
}

void RigidBodyImpl::SetTransform(const Transformation& t) {
	mImpl->SetTransform(t);
}

Vec3 RigidBodyImpl::GetPos() const {
	return mImpl->GetPos();
}

void RigidBodyImpl::RegisterToWorld() {
	mImpl->RegisterToWorld();
}

void RigidBodyImpl::UnregisterFromWorld() {
	mImpl->UnregisterFromWorld();
}

void RigidBodyImpl::SetKinematic(bool enable) {
	mImpl->SetKinematic(enable);
}

const Vec3I& RigidBodyImpl::GetGroupIdx() const {
	return mImpl->GetGroupIdx();
}

bool RigidBodyImpl::IsGrouped() const {
	return mImpl->IsGrouped();
}

void RigidBodyImpl::RemoveConstraints() {
	mImpl->RemoveConstraints();
}

void RigidBodyImpl::RemoveConstraint(void* constraintPtr) {
	mImpl->RemoveConstraint(constraintPtr);
}

void RigidBodyImpl::RemoveConstraintsFor(void* gamePtr){
	mImpl->RemoveConstraintsFor(gamePtr);
}

void* RigidBodyImpl::GetLastConstraintsPtr() {
	return mImpl->GetLastConstraintsPtr();
}

void RigidBodyImpl::SetGameFlag(unsigned flag) {
	mImpl->SetGameFlag(flag);
}

unsigned RigidBodyImpl::GetGameFlag() const {
	return mImpl->GetGameFlag();
}

void RigidBodyImpl::SetDebug(bool debug) {
	mImpl->SetDebug(debug);
}

bool RigidBodyImpl::CheckCollideWith(RigidBodyPtr other){
	return mImpl->CheckCollideWith(other);
}

float RigidBodyImpl::GetTimeToStopRotation(const Vec3& torque, float& currentAngularSpeed) const{
	auto btTorque = FBToBullet(torque);
	auto& angularVelV = getAngularVelocity();
	currentAngularSpeed = angularVelV.length();
	// dw : angular acceleration
	auto& invInertiaTensor = getInvInertiaTensorWorld();
	auto dwV = invInertiaTensor * btTorque;
	float dw = dwV.length();
	return currentAngularSpeed / dw;
}

Mat33 RigidBodyImpl::GetInertiaTensor() const{
	return Mat33();
}

void RigidBodyImpl::SetGravity(const Vec3& gravity) {
	auto btGravity = FBToBullet(gravity);
	setGravity(btGravity);
}

void RigidBodyImpl::OnAABBOverflow() {
	__super::OnAABBOverflow();

	if (mImpl->mColProvider) {
		mImpl->mColProvider->OnAABBOverflow();
	}
}