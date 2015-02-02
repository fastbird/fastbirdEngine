#pragma once
#include <Physics/RigidBody.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
namespace fastbird
{
	class RigidBodyImpl : public RigidBody, public btRigidBody,
		public btCollisionWorld::ContactResultCallback
	{
		btDiscreteDynamicsWorld* mWorld;
		
		RotationInfo* mRotationInfo;

		

		// Game object. (class PhysicsComp for my game.)
		IPhysicsInterface* mPhysicsInterface;

		void* mGamePtr; // save actor.

	public:
		RigidBodyImpl(btRigidBodyConstructionInfo& cinfo, btDiscreteDynamicsWorld* world, IPhysicsInterface* colProvider);
		~RigidBodyImpl();

		//-------------------------------------------------------------------
		// btCollisionWorld::ContactResultCallback
		//-------------------------------------------------------------------
		virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);

		// Physics dll internal
		virtual void RefreshColShape(IPhysicsInterface* colProvider);
		virtual void ApplyForce(const Vec3& force, const Vec3& rel_pos);
		virtual void ApplyImpulse(const Vec3& impulse, const Vec3& rel_pos);
		virtual void ApplyCentralImpulse(const Vec3& impulse);
		virtual void ApplyTorqueImpulse(const Vec3& torque);
		virtual void ApplyTorque(const Vec3& torque);
		virtual void ClearForces();
		virtual float GetSpeed() const;
		virtual Vec3 GetVelocity() const;
		virtual Vec3 GetAngularVelocity() const;
		virtual Vec3 GetTorque() const;
		virtual void SetVelocity(const Vec3& vel);
		virtual void Activate();
		virtual void EnableDeactivation(bool enable);
		virtual Vec3 GetDestDir() const;

		virtual void Update(float dt);

		virtual void SetMass(float mass);

		virtual void SetPhysicsInterface(IPhysicsInterface* obj);
		virtual IPhysicsInterface* GetPhysicsInterface() const;

		virtual void* GetColShapeUserPtr(int idx = 0);
		virtual void* GetGamePtr() const;
		virtual void SetRotationalForce(float force);

		virtual void RemoveColFlag(unsigned flag);
		virtual void AddColFlag(unsigned flag);
		virtual void SetColMask(unsigned mask);
		virtual unsigned GetColMask() const;

		virtual void SetLinearDamping(float damping);

		virtual bool HasContact();
	};
}