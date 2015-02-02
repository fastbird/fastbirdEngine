#include <Physics/stdafx.h>
#include <Physics/ColShapes.h>

using namespace fastbird;

BoxShape* CollisionShapeMan::CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& extent, void* userPtr)
{
	auto shape = FB_NEW(BoxShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = CollisionShapes::Box;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}
SphereShape* CollisionShapeMan::CreateSphereShape(const Vec3& pos, const Quat& rot, float radius, void* userPtr)
{
	auto shape = FB_NEW(SphereShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = CollisionShapes::Sphere;
	shape->mUserPtr = userPtr;

	shape->mRadius = radius;

	return shape;
}
CylinderShape* CollisionShapeMan::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& extent, void* userPtr)
{
	auto shape = FB_NEW(CylinderShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = CollisionShapes::Cylinder;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}
CapsuleShape* CollisionShapeMan::CreateCylinderShape(const Vec3& pos, const Quat& rot, float radius, float height, void* userPtr)
{
	auto shape = FB_NEW(CapsuleShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = CollisionShapes::Capsule;
	shape->mUserPtr = userPtr;

	shape->mRadius = radius;
	shape->mHeight = height;

	return shape;
}
MeshShape* CollisionShapeMan::CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
	bool staticObj, void* userPtr)
{
	auto shape = FB_NEW(MeshShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = staticObj ? CollisionShapes::StaticMesh : CollisionShapes::DynamicMesh;
	shape->mUserPtr = userPtr;

	shape->mVertices = vertices;
	shape->mNumVertices= numVertices;
	shape->mScale = scale;
	shape->GetTriangleMesh(); // to generate

	return shape;
}

MeshShape* CollisionShapeMan::CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
	const Vec3& scale, void* userPtr)
{
	auto shape = FB_NEW(MeshShape);
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = CollisionShapes::Convex;
	shape->mUserPtr = userPtr;

	shape->mVertices = vertices;
	shape->mNumVertices = numVertices;
	shape->mScale = scale;
	shape->GetTriangleMesh(); // to generate

	return shape;
}

void MeshShape::CreateTriangleMesh()
{
	assert(mVertices);
	mTriangleMesh = FB_NEW_ALIGNED(btTriangleMesh, 16)();
	assert(mNumVertices % 3 == 0);
	for (unsigned i = 0; i < mNumVertices; i += 3)
	{
		mTriangleMesh->addTriangle(FBToBullet(mVertices[i]), FBToBullet(mVertices[i + 1]), FBToBullet(mVertices[i + 2]));
	}
}

void CollisionShapeMan::DestroyShape(CollisionShape* shape)
{
	FB_DELETE(shape);
}


//---------------------------------------------------------------------------
btTriangleMesh* MeshShape::GetTriangleMesh()
{
	if (!mTriangleMesh)
	{
		assert(mVertices);
		CreateTriangleMesh();
		mTriangleMesh->setScaling(FBToBullet(mScale));
	}
	return mTriangleMesh;
}

MeshShape::~MeshShape()
{
	FB_DEL_ALIGNED(mTriangleMesh);
}