#include "stdafx.h"
#include "CollisionShapeFactory.h"
#include "ColShapes.h"

using namespace fb;

BoxShapePtr CollisionShapeFactory::CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr) {
	auto shape = BoxShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Box;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}
SphereShapePtr CollisionShapeFactory::CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr) {
	auto shape = SphereShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Sphere;
	shape->mUserPtr = userPtr;

	shape->mRadius = radius;

	return shape;
}
CylinderShapePtr CollisionShapeFactory::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr) {
	auto shape = CylinderShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Cylinder;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}
CapsuleShapePtr CollisionShapeFactory::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr) {
	auto shape = CapsuleShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Capsule;
	shape->mUserPtr = userPtr;

	shape->mRadius = radius;
	shape->mHeight = height;

	return shape;
}
MeshShapePtr CollisionShapeFactory::CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
	bool staticObj, void* userPtr) {

	auto shape = MeshShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mType = staticObj ? CollisionShapes::StaticMesh : CollisionShapes::DynamicMesh;
	shape->mUserPtr = userPtr;

	shape->mVertices = vertices;
	shape->mNumVertices = numVertices;
	shape->mScale = scale;
	shape->GetTriangleMesh(); // to generate

	return shape;
}
MeshShapePtr CollisionShapeFactory::CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
	const Vec3& scale, void* userPtr) {

	auto shape = MeshShape::Create();
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