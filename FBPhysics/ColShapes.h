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

class btTriangleMesh;
namespace fb
{
	class CollisionShapes{
	public:
		enum Enum {
			Box,
			Sphere,
			Cylinder,
			Capsule,
			StaticMesh,
			DynamicMesh,
			Convex,

			Num,
		};

		static const char* strings[];
		static const char* ConvertToString(Enum a);
		static Enum ConvertToEnum(const char* str);
	};

	FB_DECLARE_SMART_PTR_STRUCT(CollisionShape);
	struct CollisionShape
	{
	protected:
		CollisionShape();
		~CollisionShape();
	public:
		virtual void ChangeScale(const Vec3& scale);

		CollisionShapes::Enum mType;
		Vec3 mPos;
		Quat mRot;
		Vec3 mScale;
		void* mUserPtr;
	};

	FB_DECLARE_SMART_PTR_STRUCT(BoxShape);
	struct BoxShape : public CollisionShape
	{
	private:
		BoxShape();
		~BoxShape();

	public:
		static BoxShapePtr Create();
		void ChangeScale(const Vec3& scale);
		Vec3 mExtent;		
	};

	FB_DECLARE_SMART_PTR_STRUCT(SphereShape);
	struct SphereShape : public CollisionShape
	{
	private:
		SphereShape();
		~SphereShape();

	public:
		static SphereShapePtr Create();
		void ChangeScale(const Vec3& scale);
		float mRadius;
	};

	FB_DECLARE_SMART_PTR_STRUCT(CylinderShape);
	struct CylinderShape : public CollisionShape
	{
	private:
		CylinderShape();
		~CylinderShape();

	public:
		static CylinderShapePtr Create();
		void ChangeScale(const Vec3& scale);
		Vec3 mExtent;
	};

	FB_DECLARE_SMART_PTR_STRUCT(CapsuleShape);
	struct CapsuleShape : public CollisionShape
	{
	private:
		CapsuleShape();
		~CapsuleShape();

	public:
		static CapsuleShapePtr Create();
		void ChangeScale(const Vec3& scale);
		float mRadius;
		float mHeight;
	};

	FB_DECLARE_SMART_PTR_STRUCT(MeshShape);
	struct MeshShape : public CollisionShape
	{
	private:
		MeshShape();
		~MeshShape();

	public:
		static MeshShapePtr Create();
		void ChangeScale(const Vec3& scale);

		Vec3* mVertices;
		unsigned mNumVertices;
		btTriangleMesh* GetTriangleMesh();

	private:
		void CreateTriangleMesh();
		btTriangleMesh* mTriangleMesh;
	};

	class CollisionShapeMan
	{
	public:
		static BoxShapePtr CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale,const Vec3& extent, void* userPtr = 0);
		static SphereShapePtr CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr = 0);
		static CylinderShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		static CapsuleShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr = 0);
		static MeshShapePtr CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
			bool staticObj, void* userPtr = 0);
		static MeshShapePtr CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, 
			const Vec3& scale, void* userPtr = 0);		
	};
}