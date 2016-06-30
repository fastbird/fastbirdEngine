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
#include "ISpatialObject.h"
#include "FBMathLib/BoundingVolume.h"// convinient include
#include "FBCommonHeaders/Types.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBMathLib/Transformation.h"
namespace fb{	
	class ICamera;
	FB_DECLARE_SMART_PTR(AnimationData);
	FB_DECLARE_SMART_PTR(Animation);
	FB_DECLARE_SMART_PTR(BoundingVolume);
	FB_DECLARE_SMART_PTR(SpatialObject);	
	class FB_DLL_SCENEMANAGER SpatialObject : public ISpatialObject {
		Transformation mLocation;
		TransformationPtr mAnimatedLocation;
		BoundingVolumePtr mBoundingVolume;
		BoundingVolumePtr mBoundingVolumeWorld;
		VectorMap<ICamera*, Real> mDistToCam;
		AnimationPtr mAnim;
		Vec3 mPreviousPosition;
		bool mTransformChanged;

	protected:
		SpatialObject();
		SpatialObject(const SpatialObject& other);
		~SpatialObject();

	public:		
		void SetRadius(Real r);
		Real GetRadius() const;
		void SetDistToCam(ICamera* cam, Real dist);
		Real GetDistToCam(ICamera* cam) const;
		const Vec3& GetPosition() const;
		const Vec3& GetPreviousPosition() const;
		const Vec3& GetScale() const;
		Vec3 GetDirection() const;
		const Quat& GetRotation() const;
		void SetPosition(const Vec3& pos);
		void SetRotation(const Quat& rot);
		void SetScale(const Vec3& scale); // use uniform only.
		void SetDirection(const Vec3& dir);
		void SetDirectionAndRight(const Vec3& dir, const Vec3& right);
		void UseAABBBoundingVolume();
		BoundingVolumePtr GetBoundingVolume();
		BoundingVolumePtr GetBoundingVolumeWorld();
		const Transformation& GetLocation() const;
		const Transformation& GetAnimatedLocation() const;
		AnimationPtr GetAnimation() const;
		void SetLocation(const Transformation& t);
		bool GetTransformChanged() const;
		void ClearTransformChanged();
		virtual void Update(TIME_PRECISION dt);
		void SetAnimation(AnimationPtr anim);
		AnimationDataPtr GetAnimationData() const;
		void PlayAction(const char* name, bool immediate, bool reverse);			
		bool IsPlayingAction() const;		
		bool IsActionDone(const char* action) const;
		void StopAnimation();
		void NotifyTransformChanged();		

	protected:
		void SetBoundingVolume(const BoundingVolume& src);
		void MergeBoundingVolume(const BoundingVolumePtr src);
		void UpdateAnimation(TIME_PRECISION dt);
	};
}