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

#pragma once
#include "FBCommonHeaders/Types.h"
#include "RigidBody.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
namespace fb
{
	class Vec3;
	class Vec3I;
	class Transformation;
	FB_DECLARE_SMART_PTR(RigidBodyImpl);
	class RigidBodyImpl : public RigidBody, public btRigidBody, public btCollisionWorld::ContactResultCallback
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(RigidBodyImpl);

		RigidBodyImpl(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider);
		
	public:	
		static RigidBodyImplPtr Create(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider);
		//-------------------------------------------------------------------
		// btCollisionWorld::ContactResultCallback
		//-------------------------------------------------------------------
		btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);

		// Physics dll internal
		void RefreshColShape(IPhysicsInterface* colProvider);
		void ApplyForce(const Vec3& force, const Vec3& rel_pos);
		void ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos);
		void ApplyCentralImpulse(const Vec3& impulse);
		void ApplyTorqueImpulse(const Vec3& torque);
		void ApplyTorque(const Vec3& torque);
		Vec3 GetForce();
		void ClearForces();
		float GetSpeed() const;
		Vec3 GetVelocity() const;
		Vec3 GetAngularVelocity() const;
		void SetAngularVelocity(const Vec3& angVel);
		Vec3 GetTorque() const;
		void SetVelocity(const Vec3& vel);
		void Activate();
		void EnableDeactivation(bool enable);
		Vec3 GetDestDir() const;

		void SetMass(float mass);

		void SetPhysicsInterface(IPhysicsInterface* obj);
		void SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx);
		IPhysicsInterface* GetPhysicsInterface() const;

		void* GetColShapeUserPtr(int idx = 0);
		void* GetGamePtr() const;
		void SetRotationalForce(float force);

		void SetCollisionFilter(unsigned group);
		void RemoveCollisionFilter(unsigned flag);
		void AddCollisionFilter(unsigned flag);
		void SetColMask(unsigned mask);
		unsigned GetColMask() const;

		void SetLinearDamping(float damping);
		void SetAngularDamping(float damping);
		void SetDamping(float linear, float angular);

		unsigned HasContact(void* gamePtrs[], int limit);

		void RemoveRigidBodyFromWorld();
		// make sure mColProvider is valid.
		void ReAddRigidBodyFromWorld();
		void ModifyCollisionFlag(int flag, bool enable);
		void SetCCDMotionThreshold(float threshold);
		void SetCCDSweptSphereRadius(float radius);
		void SetIgnoreCollisionCheck(RigidBodyPtr rigidBody, bool ignore);

		void SetTransform(const Transformation& t);
		Vec3 GetPos() const;

		void RegisterToWorld();
		void UnregisterFromWorld();

		void SetKinematic(bool enable);

		const Vec3I& GetGroupIdx() const;
		bool IsGrouped() const;

		void RemoveConstraints();
		void RemoveConstraint(void* constraintPtr);
		void* GetLastConstraintsPtr();

		void SetGameFlag(unsigned flag);
		unsigned GetGameFlag() const;

		void SetDebug(bool debug);

		void ClearAngularVelocity();
	};
}