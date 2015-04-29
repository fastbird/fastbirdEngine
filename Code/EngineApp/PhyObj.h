#pragma once

namespace fastbird
{
	class RigidBody;
}
#include <Physics/IPhysicsInterface.h>
class PhyObj : public fastbird::IPhysicsInterface
{
	fastbird::Transformation mTransform;
	fastbird::BoxShape mColShape;
	std::vector<fastbird::CollisionShape*> mDummy;
	float mMass;
	unsigned mColGroup;
	unsigned mColMask;
	float mLinearDamping;
	float mAngularDamping;

	fastbird::RigidBody* mRigidBody;
	fastbird::IMeshObject* mMesh;

public:
	PhyObj(const fastbird::Transformation& transform, float mass, unsigned colGroup, unsigned colMask,
		float linearDamping, float angularDamping);
	~PhyObj();

	//-------------------------------------------------------------------------
	void CreateRigidBody();
	fastbird::RigidBody* GetRigidBody() const { return mRigidBody; }
	// transferred ownership
	void SetMeshObj(fastbird::IMeshObject* mesh);

	//-------------------------------------------------------------------------
	// IPhysicsInterface Implementation
	//-------------------------------------------------------------------------
	virtual void* GetUserPtr() const { return (void*)this; }
	// col shape provider
	virtual unsigned GetNumColShapes() const { return 1; }
	virtual fastbird::CollisionShape* GetShape(unsigned i) { return &mColShape; }
	virtual const std::vector<fastbird::CollisionShape*>& GetShapes() const { 
		return  mDummy;
	}
	virtual float GetMass() const { return mMass; }
	virtual int GetCollisionGroup() const { return mColGroup; }
	virtual int GetCollisionMask() const { return mColMask; }

	virtual float GetLinearDamping() const { return mLinearDamping; }
	virtual float GetAngularDamping() const { return mAngularDamping; }

	// Transform exchanger
	virtual const fastbird::Vec3& GetPos();
	virtual const fastbird::Quat& GetRot();
	virtual void SetPosRot(const fastbird::Vec3& pos, const fastbird::Quat& rot);

	virtual bool OnCollision(const CollisionContactInfo& contactInfo);
	virtual void AddCloseObjects(void* gamePtr);
	virtual void OnRigidBodyUpdated(const fastbird::RigidBodyEvents& data){}

};