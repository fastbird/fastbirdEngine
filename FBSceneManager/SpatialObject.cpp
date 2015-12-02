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
#include "SpatialObject.h"
#include "Scene.h"
#include "FBMathLib/BoundingVolume.h"
#include "FBAnimation/Animation.h"
#include "FBAnimation/AnimationData.h"
#include "FBMathLib/Math.h"
#include "FBMathLib/BoundingVolume.h"
using namespace fb;

//---------------------------------------------------------------------------
SpatialObject::SpatialObject()
	: mBoundingVolume(BoundingVolume::Create())
	, mBoundingVolumeWorld(BoundingVolume::Create())
	, mPreviousPosition(0, 0, 0)
	, mTransformChanged(true)
{
}

SpatialObject::SpatialObject(const SpatialObject& other)
	: SpatialObject()
{
	mLocation = other.mLocation;
	if (other.mAnimatedLocation){
		mAnimatedLocation = Transformation::Create();
		*mAnimatedLocation = *other.mAnimatedLocation;
	}
	*mBoundingVolume = *other.mBoundingVolume;
	*mBoundingVolumeWorld = *other.mBoundingVolumeWorld;
	mDistToCam = other.mDistToCam;
	if (other.mAnim){
		mAnim = other.mAnim->Clone();
	}
	mTransformChanged = other.mTransformChanged;
}

SpatialObject::~SpatialObject(){

}

void SpatialObject::SetRadius(Real r){
	mBoundingVolume->SetRadius(r);
	mBoundingVolumeWorld->SetRadius(r);
}

Real SpatialObject::GetRadius() const{
	return mBoundingVolumeWorld->GetRadius();
}

void SpatialObject::SetDistToCam(Real dist){
	mDistToCam = dist;
}

Real SpatialObject::GetDistToCam() const{
	return mDistToCam;
}

const Vec3& SpatialObject::GetPosition() const{
	return mLocation.GetTranslation();
}

const Vec3& SpatialObject::GetPreviousPosition() const{
	return mPreviousPosition;
}

const Vec3& SpatialObject::GetScale() const{
	return mLocation.GetScale();
}

Vec3 SpatialObject::GetDirection() const{
	return mLocation.GetForward();
}

const Quat& SpatialObject::GetRotation() const{
	return mLocation.GetRotation();
}

void SpatialObject::SetPosition(const Vec3& pos){
	mPreviousPosition = mLocation.GetTranslation();
	mLocation.SetTranslation(pos);
	mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + pos);
	mTransformChanged = true;
}

void SpatialObject::SetRotation(const Quat& rot){
	mLocation.SetRotation(rot);
	mTransformChanged = true;
}

void SpatialObject::SetScale(const Vec3& scale){
	mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius() * std::max(scale.x, std::max(scale.y, scale.z)));
	mLocation.SetScale(scale);
	mTransformChanged = true;
}

void SpatialObject::SetDirection(const Vec3& dir){
	mLocation.SetDirection(dir);
	mTransformChanged = true;
}

void SpatialObject::SetDirectionAndRight(const Vec3& dir, const Vec3& right){
	mLocation.SetDirectionAndRight(dir, right);
	mTransformChanged = true;
}

BoundingVolumePtr SpatialObject::GetBoundingVolume(){
	return mBoundingVolume;
}

BoundingVolumePtr SpatialObject::GetBoundingVolumeWorld(){
	return mBoundingVolumeWorld;
}

const Transformation& SpatialObject::GetLocation() const{
	return mLocation;
}

const Transformation& SpatialObject::GetAnimatedLocation() const{
	return mAnim ? *mAnimatedLocation : mLocation;
}

AnimationPtr SpatialObject::GetAnimation() const{
	return mAnim;
}

void SpatialObject::SetLocation(const Transformation& t){
	mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + t.GetTranslation());
	mLocation = t;
}

bool SpatialObject::GetTransformChanged() const{
	return mTransformChanged;
}

void SpatialObject::ClearTransformChanged(){
	mTransformChanged = false;
}

void SpatialObject::SetAnimation(AnimationPtr anim){
	mAnim = anim;
	if (!mAnimatedLocation){
		mAnimatedLocation = Transformation::Create();
	}
	mAnimatedLocation->MakeIdentity();
}

void SpatialObject::UpdateAnimation(TIME_PRECISION dt){
	if (mAnim){
		mAnim->Update(dt);
		if (mAnim->Changed())
			*mAnimatedLocation = mLocation * mAnim->GetResult();
	}
}

void SpatialObject::PlayAction(const char* name, bool immediate, bool reverse){
	if (mAnim){
		mAnim->PlayAction(name, immediate, reverse);
	}
}

bool SpatialObject::IsPlayingAction() const{
	return mAnim && mAnim->IsPlaying();
}

bool SpatialObject::IsActionDone(const char* action) const{
	if (!mAnim)
		return true;
	return mAnim->IsActionDone(action);
}

void SpatialObject::NotifyTransformChanged(){
	mTransformChanged = true;
}

void SpatialObject::SetBoundingVolume(const BoundingVolume& src){
	*mBoundingVolume = src;
	*mBoundingVolumeWorld = *mBoundingVolume;
	mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + mLocation.GetTranslation());
	auto scale = mLocation.GetScale();
	mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius() * std::max(scale.x, std::max(scale.y, scale.z)));
}

void SpatialObject::MergeBoundingVolume(const BoundingVolumePtr src){
	mBoundingVolume->Merge(src.get());
	*mBoundingVolumeWorld = *mBoundingVolume;
	mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + mLocation.GetTranslation());
	auto scale = mLocation.GetScale();
	mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius() * std::max(scale.x, std::max(scale.y, scale.z)));
}