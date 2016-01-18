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
namespace fb
{
	FB_DECLARE_SMART_PTR_STRUCT(CollisionShape);
	class RigidBody;
	struct RigidBodyEvents;
	class IPhysicsInterface
	{
	public:

		virtual void* GetUserPtr() const = 0;

		// col shape provider
		virtual bool IsGroupedBody() const { return false; }
		virtual unsigned GetShapesForGroup(const Vec3I& groupIdx, CollisionShapePtr shapes[], unsigned maxNum) const { return 0; }
		virtual unsigned GetNumGroups() const { return 0; }
		virtual unsigned GetNumColShapes(const Vec3I& groupIdx) { return 0; }
		virtual float GetMassForGroup(const Vec3I& group) const { return 0; }

		virtual unsigned GetNumColShapes() const = 0;		
		virtual fb::CollisionShapePtr GetShape(unsigned i) = 0;
		virtual unsigned GetShapes(CollisionShapePtr shapes[], unsigned maxNum) const = 0;
		virtual float GetMass() const = 0;

		virtual int GetCollisionGroup() const = 0;
		virtual int GetCollisionMask() const = 0;

		virtual float GetLinearDamping() const = 0;
		virtual float GetAngularDamping() const = 0;

		// Transform exchanger
		virtual const fb::Vec3& GetPos() = 0;
		virtual const fb::Quat& GetRot() = 0;
		virtual void SetPosRot(const Vec3& pos, const Quat& rot) = 0;

		// Events Handler
		struct CollisionContactInfo
		{
			CollisionContactInfo(RigidBody* a,RigidBody* b, const fb::Vec3& worldpos, const fb::Vec3& worldNormal, float impulse, int idxA, int idxB)
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
		virtual void OnRigidBodyUpdated(const fb::RigidBodyEvents& data){}

		virtual bool ForceCompound() const { return false; }
		/// not using
		virtual bool UseSymmetricInertia() const { return false; }
	};
}