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

#include "stdafx.h"
#include "ColShapes.h"

using namespace fb;
const char* CollisionShapes::strings[] = {
	"Box",
	"Sphere",
	"Cylinder",
	"Capsule",
	"StaticMesh",
	"DynamicMesh",
	"Convex",

	"Num",
};

const char* CollisionShapes::ConvertToString(Enum a)
{
	assert(a >= 0 && a <= Num);
	return strings[a];
}

CollisionShapes::Enum CollisionShapes::ConvertToEnum(const char* str)
{
	for (int i = 0; i < Num; i++)
	{
		if (_stricmp(str, strings[i]) == 0)
			return Enum(i);
	}

	return Num;
}
//---------------------------------------------------------------------------
CollisionShape::CollisionShape()
	: mPos(0, 0, 0), mUserPtr(0)
	, mScale(1, 1, 1)
{
}

void CollisionShape::ChangeScale(const Vec3& scale){
	mScale = scale;
}

CollisionShape::~CollisionShape(){
}

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(BoxShape);
BoxShape::BoxShape(){

}
BoxShape::~BoxShape(){

}

void BoxShape::ChangeScale(const Vec3& scale){
	mScale = scale;
}

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(SphereShape);
SphereShape::SphereShape(){

}

SphereShape::~SphereShape(){

}
void SphereShape::ChangeScale(const Vec3& scale)
{
	mScale = scale;
}

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(CylinderShape);
CylinderShape::CylinderShape(){

}
CylinderShape::~CylinderShape(){

}
void CylinderShape::ChangeScale(const Vec3& scale){
	mScale = scale;
}

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(CapsuleShape);
CapsuleShape::CapsuleShape(){

}

CapsuleShape::~CapsuleShape(){

}
void CapsuleShape::ChangeScale(const Vec3& scale){
	mRadius *= scale.x;
	mHeight *= scale.y;
}

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(MeshShape);
MeshShape::MeshShape() :mTriangleMesh(0), mNumVertices(0)
{
}
MeshShape::~MeshShape()
{
	FB_DELETE_ALIGNED(mTriangleMesh);
}

//---------------------------------------------------------------------------
BoxShapePtr CollisionShapeMan::CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr)
{
	auto shape = BoxShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Box;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}

//---------------------------------------------------------------------------
SphereShapePtr CollisionShapeMan::CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr)
{
	auto shape = SphereShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Sphere;
	shape->mUserPtr = userPtr;

	shape->mRadius = radius;

	return shape;
}

//---------------------------------------------------------------------------
CylinderShapePtr CollisionShapeMan::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr)
{
	auto shape = CylinderShape::Create();
	shape->mPos = pos;
	shape->mRot = rot;
	shape->mScale = actorScale;
	shape->mType = CollisionShapes::Cylinder;
	shape->mUserPtr = userPtr;

	shape->mExtent = extent;

	return shape;
}

//---------------------------------------------------------------------------
CapsuleShapePtr CollisionShapeMan::CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr)
{
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

//---------------------------------------------------------------------------
MeshShapePtr CollisionShapeMan::CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
	bool staticObj, void* userPtr)
{
	auto shape = MeshShape::Create();
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

MeshShapePtr CollisionShapeMan::CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
	const Vec3& scale, void* userPtr)
{
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

void MeshShape::CreateTriangleMesh()
{
	assert(mVertices);
	mTriangleMesh = FB_NEW_ALIGNED(btTriangleMesh, 16)();
	assert(mNumVertices % 3 == 0);
	for (unsigned i = 0; i < mNumVertices; i += 3)
	{
		mTriangleMesh->addTriangle(FBToBullet(mVertices[i]), FBToBullet(mVertices[i + 2]), FBToBullet(mVertices[i + 1]));
	}
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

void MeshShape::ChangeScale(const Vec3& scale)
{
	mScale = scale;
	mTriangleMesh->setScaling(FBToBullet(mScale));
	//FB_DEL_ALIGNED(mTriangleMesh);
	//mTriangleMesh = 0;
}