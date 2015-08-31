#pragma once
namespace fastbird{
	class RigidBody;
	struct IFilterCallback{
		virtual bool needCollision(RigidBody* a, RigidBody* b) = 0;
	};
}