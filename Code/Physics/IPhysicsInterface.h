#pragma once
#include <Physics/ColShapes.h>
namespace fastbird
{
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
		virtual unsigned GetNumColShapes() const = 0;
		virtual fastbird::CollisionShape* GetShape(unsigned i) = 0;
		virtual const std::vector<CollisionShape*>& GetShapes() const = 0;
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
			CollisionContactInfo(void* objB, const fastbird::Vec3& worldpos, const fastbird::Vec3& worldNormal, float impulse, int idxA, int idxB)
			:mObjB(objB), mWorldPos(worldpos), mWorldNormal(worldNormal), mImpulse(impulse), mIdxA(idxA), mIdxB(idxB)
			{

			}
			void* mObjB;
			Vec3 mWorldPos;
			const Vec3 mWorldNormal;
			float mImpulse;
			int mIdxA;
			int mIdxB;
		};
		virtual bool OnCollision(const CollisionContactInfo& contactInfo) = 0;
		virtual void AddCloseObjects(void* gamePtr) = 0;
		virtual void OnRigidBodyUpdated(const fastbird::RigidBodyEvents& data){}
	};
}