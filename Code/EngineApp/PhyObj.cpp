#include "StdAfx.h"
#include "PhyObj.h"
#include <Physics/IPhysics.h>
#include <Engine/IMeshObject.h>

using namespace fastbird;

PhyObj::PhyObj(const Transformation& transform, float mass, unsigned colGroup, unsigned colMask, 
	float linearDamping, float angularDamping)
: mTransform(transform)
, mMass(mass)
, mColGroup(colGroup)
, mColMask(colMask)
, mLinearDamping(linearDamping)
, mAngularDamping(angularDamping)
, mRigidBody(0)
, mMesh(0)
{
	mColShape.mPos = Vec3::ZERO;
	mColShape.mRot = Quat::IDENTITY;
	mColShape.mType = CollisionShapes::Box;
	mColShape.mUserPtr = this;
	mColShape.mExtent = Vec3(1, 1, 1);
}

PhyObj::~PhyObj()
{
	IPhysics::GetPhysics()->DeleteRigidBody(mRigidBody);
	mRigidBody = 0;
	if (mMesh)
	{
		gEnv->pEngine->ReleaseMeshObject(mMesh);
	}
}

void PhyObj::CreateRigidBody()
{
	assert(!mRigidBody);
	mRigidBody = IPhysics::GetPhysics()->CreateRigidBody(this, mMass);
	assert(mRigidBody);
}

const fastbird::Vec3& PhyObj::GetPos()
{
	return mTransform.GetTranslation();
}
const fastbird::Quat& PhyObj::GetRot()
{
	return mTransform.GetRotation();
}
void PhyObj::SetPosRot(const fastbird::Vec3& pos, const fastbird::Quat& rot)
{
	mTransform.SetTranslation(pos);
	mTransform.SetRotation(rot);
	if (mMesh)
		mMesh->SetTransform(mTransform);
}

bool PhyObj::OnCollision(const CollisionContactInfo& contactInfo)
{
	return false;
}
void PhyObj::AddCloseObjects(void* gamePtr)
{

}

void PhyObj::SetMeshObj(IMeshObject* mesh)
{
	if (mMesh)
	{
		gEnv->pEngine->ReleaseMeshObject(mMesh);
	}

	mMesh = mesh;
	if (mMesh)
	{
		mMesh->SetTransform(mTransform);
		mMesh->AttachToScene();
	}
	
}