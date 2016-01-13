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
#include "IPhysics.h"
#include "FBCommonHeaders/Types.h"
namespace fb
{
	struct FBFilterCallback;
	class BulletDebugDraw;
	struct BulletFilterCallback;
	FB_DECLARE_SMART_PTR(Physics);
	class Physics : public IPhysics
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(Physics);		
		Physics();
		~Physics();

	public:
		/**Internal only.
		Use IPhysics::Create() instead. */
		static PhysicsPtr Create();
		/**Internal only.
		Use IPhysics::GetInstance() instead. */
		static Physics& GetInstance();

		static NeedCollisionForConvexCallback sNeedCollisionForConvexCallback;

		void Initilaize();
		void Deinitilaize();
		void Update(float dt);
		void EnablePhysics();
		void DisablePhysics();

		btCollisionShape* CreateColShape(IPhysicsInterface* shapeProvider);
		btCollisionShape* CreateColShapeForGroup(IPhysicsInterface* shapeProvider, const Vec3I& groupIdx);
		btCollisionShape* CreateColShape(CollisionShapePtr colShapes[], unsigned num, bool forceCompound);

		RigidBodyPtr CreateRigidBody(const char* collisionFile, float mass, IPhysicsInterface* obj);
		RigidBodyPtr CreateRigidBody(IPhysicsInterface* obj);
		RigidBodyPtr CreateRigidBodyForGroup(IPhysicsInterface* colProvider, const Vec3I& groupIdx);
		RigidBodyPtr CreateTempRigidBody(CollisionShapePtr colShape);
		RigidBodyPtr CreateTempRigidBody(CollisionShapePtr  shapes[], unsigned num);
		
		btCollisionShape* ParseCollisionFile(const char* collisionFile);
		
		void RemoveConstraint(btTypedConstraint* constraint);

		void AddRef(btCollisionShape* colShape);
		void Release(btCollisionShape* colShape);
		void SetDebugDrawer(IDebugDrawer* debugDrawer);
		void SetDebugMode(int debugMode);

		
		void AttachBodies(RigidBodyPtr bodies[], unsigned num);
		void AttachBodiesAlways(RigidBodyPtr bodies[], unsigned num);

		void SetRayCollisionGroup(int group);
		bool RayTestClosest(const Vec3& fromWorld, const Vec3& toWorld, int additionalRayGroup, int mask, RayResultClosest& result, void* excepts[] = 0, unsigned numExcepts = 0);
		bool RayTestWithAnObj(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, RayResultWithObj& result);
		RayResultAll* RayTestAll(const Vec3& fromWorld, const Vec3& toWorld, int additionalGroupFlag, int mask);
		void Release(RayResultAll* r);
		unsigned GetAABBOverlaps(const AABB& aabb, unsigned colMask, RigidBody* ret[], unsigned limit, RigidBody* except);
		float GetDistanceBetween(RigidBodyPtr a, RigidBodyPtr b);

		unsigned CreateBTSphereShape(float radius);
		void DeleteBTShape(unsigned id);
		void DrawDebugInfo();
		void ResetConstraintsSolver();
		bool NeedToCollides(RigidBodyPtr a, RigidBodyPtr b);

		//-------------------------------------------------------------------
		// collision shape manager
		//-------------------------------------------------------------------
		BoxShapePtr CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		SphereShapePtr CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr = 0);
		CylinderShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		CapsuleShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr = 0);
		MeshShapePtr CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
			bool staticObj, void* userPtr = 0);
		MeshShapePtr CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
			const Vec3& scale, void* userPtr = 0);

		void RegisterFilterCallback(IFilterCallback* callback, NeedCollisionForConvexCallback func);		


		// internal only.
		void _ReportCollisions();
		void _CheckCollisionShapeForDel(float timeStep);
		btDynamicsWorld* _GetDynamicWorld() const;
	};
}