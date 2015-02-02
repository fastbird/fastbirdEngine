#include <Physics/stdafx.h>
#include <Physics/fbMotionState.h>
#include <Physics/IPhysicsInterface.h>
#include <Physics/mathConv.h>

using namespace fastbird;

fbMotionState::fbMotionState(IPhysicsInterface* obj)
: mVisualObj(obj)
{
}

fbMotionState::~fbMotionState()
{
}

void	fbMotionState::getWorldTransform(btTransform& worldTrans) const
{
	assert(mVisualObj);
	worldTrans.setIdentity();
	worldTrans.setRotation(FBToBullet(mVisualObj->GetRot()));
	worldTrans.setOrigin(FBToBullet(mVisualObj->GetPos()));	
}

//Bullet only calls the update of worldtransform for active objects
void	fbMotionState::setWorldTransform(const btTransform& worldTrans)
{
	assert(mVisualObj);
	mVisualObj->SetPosRot(BulletToFB(worldTrans.getOrigin()), BulletToFB(worldTrans.getRotation()));
}