#pragma once
#include <LinearMath/btMotionState.h>
namespace fastbird
{
	class IPhysicsInterface;
	class RigidBody;
	class fbMotionState : public btMotionState
	{
		IPhysicsInterface* mVisualObj;

	public:
		fbMotionState(IPhysicsInterface* obj);
		virtual ~fbMotionState();

		virtual void	getWorldTransform(btTransform& worldTrans) const;

		//Bullet only calls the update of worldtransform for active objects
		virtual void	setWorldTransform(const btTransform& worldTrans);
	};
}