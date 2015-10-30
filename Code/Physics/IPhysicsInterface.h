#pragma once
#include <Physics/ColShapes.h>
namespace fastbird
{
	class RigidBody;
	struct RigidBodyEvents
	{
		RigidBodyEvents()
		: mForward(0), mBackward(0), mLeft(0), mRight(0), mUp(0), mDown(0), mHori(0), mVert(0)
		, mRollRight(0), mRollLeft(0)
		{

		}
		float mForward;
		float mBackward;
		float mLeft;
		float mRight;
		float mUp;
		float mDown;

		float mRollRight; // clockWise
		float mRollLeft;

		float mHori;
		float mVert;
	};

	class IPhysicsInterface
	{
	public:

		virtual void* GetUserPtr() const = 0;

		// col shape provider
		virtual bool IsGroupedBody() const { return false; }
		virtual unsigned GetShapesForGroup(const Vec3I& groupIdx, CollisionShape* shapes[]) const { assert(0); return 0; }
		virtual unsigned GetNumGroups() const { assert(0); return 0; }
		virtual unsigned GetNumColShapes(const Vec3& groupIdx) { assert(0); return 0; }
		virtual float GetMassForGroup(const Vec3I& group) const { assert(0); return 0; }

		virtual unsigned GetNumColShapes() const = 0;		
		virtual fastbird::CollisionShape* GetShape(unsigned i) = 0;
		virtual unsigned GetShapes(CollisionShape* shapes[]) const = 0;
		virtual float GetMass() const = 0;

		virtual int GetCollisionGroup() const = 0;
		virtual int GetCollisionMask() const = 0;

		virtual float GetLinearDamping() const = 0;
		virtual float GetAngularDamping() const = 0;

		// Transform exchanger
		virtual const fastbird::Vec3& GetPos() = 0;
		virtual const fastbird::Quat& GetRot() = 0;
		virtual void SetPosRot(const Vec3& pos, const Quat& rot) = 0;

		// Events Handler
		struct CollisionContactInfo
		{
			CollisionContactInfo(RigidBody* a,RigidBody* b, const fastbird::Vec3& worldpos, const fastbird::Vec3& worldNormal, float impulse, int idxA, int idxB)
			:mA(a), mB(b), mWorldPos(worldpos), mWorldNormal(worldNormal), mImpulse(impulse), mIdxA(idxA), mIdxB(idxB)
			{

			}
			RigidBody* mA;
			RigidBody* mB;
			Vec3 mWorldPos;
			const Vec3 mWorldNormal;
			float mImpulse;
			int mIdxA;
			int mIdxB;
		};
		virtual bool OnCollision(const CollisionContactInfo& contactInfo) = 0;
		virtual void AddCloseObjects(RigidBody* gamePtr) {}
		virtual void OnRigidBodyUpdated(const fastbird::RigidBodyEvents& data){}

		virtual bool ForceCompound() const { return false; }
	};
}