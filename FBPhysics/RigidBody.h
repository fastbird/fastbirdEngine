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

class btRigidBody;
class btDiscreteDynamicsWorld;
namespace fb
{
	class IPhysicsInterface;
	struct RotationInfo;

	FB_DECLARE_SMART_PTR(RigidBody);
	class RigidBody
	{
	public:
		RigidBody() {}
		virtual ~RigidBody(){}

		// Physics dll internal
		virtual void RefreshColShape(IPhysicsInterface* colProvider) = 0;
		virtual void ApplyForce(const Vec3& force, const Vec3& rel_pos) = 0;
		virtual void ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos) = 0;
		virtual void ApplyCentralImpulse(const Vec3& impulse) = 0;
		virtual void ApplyTorqueImpulse(const Vec3& torque) = 0;
		virtual void ApplyTorque(const Vec3& torque) = 0;
		virtual void ClearForces() = 0;
		virtual void Stop() = 0;
		virtual float GetSpeed() const = 0;
		virtual Vec3 GetVelocity() const = 0;
		virtual Vec3 GetAngularVelocity() const = 0;
		virtual void SetAngularVelocity(const Vec3& angVel) = 0;
		virtual Vec3 GetTorque() const = 0;
		virtual void SetVelocity(const Vec3& vel) = 0;
		virtual void Activate() = 0;
		virtual void EnableDeactivation(bool enable) = 0;

		virtual Vec3 GetDestDir() const = 0;

		virtual void SetMass(float mass) = 0;

		virtual void SetPhysicsInterface(IPhysicsInterface* obj) = 0;
		virtual void SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx) = 0;
		virtual IPhysicsInterface* GetPhysicsInterface() const = 0;

		// idx is for compound shape
		virtual void* GetColShapeUserPtr(int idx = 0) = 0;
		// rigid body user ptr
		virtual void* GetGamePtr() const = 0;

		virtual void SetRotationalForce(float force) = 0;

		virtual void SetCollisionFilterGroup(unsigned group) = 0;
		virtual void RemoveCollisionFilterGroup(unsigned flag) = 0;
		virtual void AddCollisionFilter(unsigned flag)=0;
		virtual void SetColMask(unsigned mask) = 0;
		virtual unsigned GetColMask() const = 0;

		virtual void SetLinearDamping(float damping) = 0;
		virtual void SetAngularDamping(float damping) = 0;
		virtual void SetDamping(float linear, float angular) = 0;
		virtual unsigned HasContact(void* gamePtrs[], int limit) = 0;

		virtual void RemoveFromWorld() = 0;
		// make sure mColProvider is valid.
		virtual void AddToWorld() = 0;
		virtual void ReaddToWorld() = 0;
		virtual void ModifyCollisionFlag(int flag, bool enable) = 0;

		virtual void SetCCDMotionThreshold(float threshold) = 0;
		virtual void SetCCDSweptSphereRadius(float radius) = 0;

		virtual void SetIgnoreCollisionCheck(RigidBodyPtr rigidBody, bool ignore) = 0;

		virtual void SetTransform(const Transformation& t) = 0;
		virtual Vec3 GetPos() const = 0;

		virtual void RegisterToWorld() = 0;
		virtual void UnregisterFromWorld() = 0;
		virtual void SetKinematic(bool enable) = 0;
		virtual const Vec3I& GetGroupIdx() const = 0;
		virtual bool IsGrouped() const = 0;
		virtual void RemoveConstraints() = 0;
		virtual void RemoveConstraint(void* constraintPtr)  = 0;
		virtual void RemoveConstraintsFor(void* gamePtr) = 0;
		virtual void* GetLastConstraintsPtr() = 0;

		virtual void SetGameFlag(unsigned flag) = 0;
		virtual unsigned GetGameFlag() const = 0;
		virtual void SetDebug(bool debug) = 0;		
		virtual bool CheckCollideWith(RigidBodyPtr other) = 0;
		virtual float GetTimeToStopRotation(
			const Vec3& torque, float& currentAngularSpeed) const = 0;
		virtual Mat33 GetInertiaTensor() const = 0;

		virtual void SetGravity(const Vec3& gravity) = 0;
		virtual AABB GetAABB() const = 0;		
	};
}