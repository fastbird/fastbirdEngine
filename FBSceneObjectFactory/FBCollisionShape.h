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
#include "FBMathLib/Transformation.h"
#include "CollisionShapeType.h"
namespace fb
{
	typedef std::pair<ColisionShapeType::Enum, Transformation> COL_SHAPE;

	FB_DECLARE_SMART_PTR(MeshObject);
	FB_DECLARE_SMART_PTR(FBCollisionShape);
	class FB_DLL_SCENEOBJECTFACTORY FBCollisionShape
	{
		FB_DECLARE_PIMPL(FBCollisionShape);
		FBCollisionShape(ColisionShapeType::Enum e, const Transformation& t, MeshObjectPtr colMesh);
		~FBCollisionShape();

	public:
		static FBCollisionShapePtr Create(ColisionShapeType::Enum e, const Transformation& t, MeshObjectPtr colMesh);
		FBCollisionShape(const FBCollisionShape& other);
		FBCollisionShape& operator=(const FBCollisionShape& other);

		void SetCollisionMesh(MeshObjectPtr colMesh);
		MeshObjectPtr GetCollisionMesh() const;
		BoundingVolumePtr GetBV() const;
		ColisionShapeType::Enum GetColShape() const;
		Vec3 GetOffset() const;
		Quat GetRot() const;
		Vec3 GetScale() const;
		typedef std::pair<bool, Real> IResult;
		IResult Intersects(const Ray3& ray, const Transformation& objT) const;
		bool TestCollision(const BoundingVolume* pBV, const Transformation& objT) const;
		Vec3 GetRandomPosInVolume(const Vec3* nearWorld, const Transformation& objT) const;

	private:
		


	};
}