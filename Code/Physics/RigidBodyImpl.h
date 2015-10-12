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
		IPhysicsInterface* mColProvider;
		

		// Game object. (class PhysicsComp for my game.)
		IPhysicsInterface* mPhysicsInterface;

		void* mGamePtr; // save actor.
		Vec3I mGroupIdx;
		bool mGroupedRigidBody;

		unsigned mGameFlag;

		bool mAddedToWorld;
		bool mDebug;
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
		virtual Vec3 GetForce();
		virtual void ClearForces();
		virtual float GetSpeed() const;
		virtual Vec3 GetVelocity() const;
		virtual Vec3 GetAngularVelocity() const;
		virtual void SetAngularVelocity(const Vec3& angVel);
		virtual Vec3 GetTorque() const;
		virtual void SetVelocity(const Vec3& vel);
		virtual void Activate();
		virtual void EnableDeactivation(bool enable);
		virtual Vec3 GetDestDir() const;

		virtual void Update(float dt);

		virtual void SetMass(float mass);

		virtual void SetPhysicsInterface(IPhysicsInterface* obj);
		virtual void SetPhysicsInterface(IPhysicsInterface* obj, const Vec3I& groupIdx);
		virtual IPhysicsInterface* GetPhysicsInterface() const;

		virtual void* GetColShapeUserPtr(int idx = 0);
		virtual void* GetGamePtr() const;
		virtual void SetRotationalForce(float force);

		virtual void SetCollisionFilter(unsigned group);
		virtual void RemoveCollisionFilter(unsigned flag);
		virtual void AddCollisionFilter(unsigned flag);
		virtual void SetColMask(unsigned mask);
		virtual unsigned GetColMask() const;

		virtual void SetLinearDamping(float damping);
		virtual void SetAngularDamping(float damping);
		virtual void SetDamping(float linear, float angular);

		virtual unsigned HasContact(void* gamePtrs[], int limit);

		virtual void RemoveRigidBodyFromWorld();
		// make sure mColProvider is valid.
		virtual void ReAddRigidBodyFromWorld();
		virtual void ModifyCollisionFlag(int flag, bool enable);
		virtual void SetCCDMotionThreshold(float threshold);
		virtual void SetCCDSweptSphereRadius(float radius);
		virtual void SetIgnoreCollisionCheck(RigidBody* rigidBody, bool ignore);

		virtual void SetTransform(const Transformation& t);
		virtual Vec3 GetPos() const;

		virtual void RegisterToWorld();
		virtual void UnregisterFromWorld();

		virtual void SetKinematic(bool enable);

		virtual const Vec3I& GetGroupIdx() const { return mGroupIdx; }
		virtual bool IsGrouped() const { return mGroupedRigidBody; }

		virtual void RemoveConstraints();
		virtual void RemoveConstraint(void* constraintPtr);
		virtual void* GetLastConstraintsPtr();

		virtual void SetGameFlag(unsigned flag);
		virtual unsigned GetGameFlag() const;

		virtual void SetDebug(bool debug);

		virtual void ClearAngularVelocity();
	};
}