#include <Engine/StdAfx.h>
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/IScene.h>
#include <Engine/ICamera.h>
#include <Engine/Animation/Animation.h>
using namespace fastbird;

//----------------------------------------------------------------------------
SpatialObject::SpatialObject()
	:mDistToCam(-1.f)
	, mPrevPos(0, 0, 0)
	, mAnimData(0)
	, mAnimOwner(false)
	, mTransformChanged(true)
	, mAnim(0)
{
}

//----------------------------------------------------------------------------
SpatialObject::~SpatialObject()
{
	mDestructing = true;
	while (!mCameraTargetingMe.empty())
	{
		mCameraTargetingMe[0]->SetTarget(0);
	}

	for (auto scene : mScenes)
	{
		scene->DetachObject(this);
	}
	if (mAnimData && mAnimOwner)
		FB_DELETE(mAnimData);

	if (mAnim)
		FB_DELETE(mAnim);
}

//----------------------------------------------------------------------------
void SpatialObject::Clone(IObject* cloned) const
{
	__super::Clone(cloned);
	SpatialObject* object = (SpatialObject*)cloned;
	object->mTransformation = mTransformation;
	object->mTransformation.SetTranslation(mTransformation.GetTranslation()+Vec3(1.f, 0, 0));
	object->mDistToCam = mDistToCam;
	object->mAnimData = mAnimData;
	if (mAnimData)
	{
		object->mAnim = FB_NEW(Animation);
		object->mAnim->SetAnimationData(mAnimData);
	}
}

//----------------------------------------------------------------------------
void SpatialObject::SetPos(const Vec3& pos)
{
	if (mBoundingVolume && mBoundingVolumeWorld)
		mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + pos);
	mPrevPos = mTransformation.GetTranslation();
	mTransformation.SetTranslation(pos);
	mTransformChanged = true;
}

//----------------------------------------------------------------------------
void SpatialObject::SetRot(const Quat& rot)
{
	mTransformation.SetRotation(rot);
	mTransformChanged = true;
}

void SpatialObject::SetScale(const Vec3& scale)
{
	if (mBoundingVolume && mBoundingVolumeWorld)
		mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius() * std::max(scale.x, std::max(scale.y, scale.z)));
	mTransformation.SetScale(scale);
	mTransformChanged = true;
}

//----------------------------------------------------------------------------
// dir should be already normalized.
void SpatialObject::SetDir(const Vec3& dir)
{
	mTransformation.SetDir(dir);
	mTransformChanged = true;
}

void SpatialObject::SetDirAndRight(const Vec3& dir, const Vec3& right)
{
	mTransformation.SetDirAndRight(dir, right);
	mTransformChanged = true;
}

//----------------------------------------------------------------------------
const Vec3& SpatialObject::GetPos() const
{
	return mTransformation.GetTranslation();
}

//----------------------------------------------------------------------------
const Quat& SpatialObject::GetRot() const
{
	return mTransformation.GetRotation();
}

//----------------------------------------------------------------------------
void SpatialObject::SetTransform(const Transformation& t)
{
	if (mBoundingVolume && mBoundingVolumeWorld)
		mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + t.GetTranslation());
	mPrevPos = mTransformation.GetTranslation();
	mTransformation = t;
	mTransformChanged = true;
}

//----------------------------------------------------------------------------
void SpatialObject::SetDistToCam(float dist)
{
	mDistToCam = dist;
}

//----------------------------------------------------------------------------
float SpatialObject::GetDistToCam() const
{
	return mDistToCam;
}

void SpatialObject::AttachToScene()
{
	IScene* pScene = gFBEnv->pEngine->GetScene();
	if (!IsAttached(pScene))
		pScene->AttachObject(this);
}

void SpatialObject::DetachFromScene()
{
	gFBEnv->pEngine->GetScene()->DetachObject(this);
}

void SpatialObject::AddCameraTargetingMe(ICamera* pCam)
{
	auto it = std::find(mCameraTargetingMe.begin(), mCameraTargetingMe.end(), pCam);
	if (it != mCameraTargetingMe.end())
		return;
	mCameraTargetingMe.push_back(pCam);
}

void SpatialObject::RemoveCameraTargetingMe(ICamera* pCam)
{
	auto it = std::find(mCameraTargetingMe.begin(), mCameraTargetingMe.end(), pCam);
	if (it != mCameraTargetingMe.end())
		mCameraTargetingMe.erase(it);
}

void SpatialObject::SetAnimationData(const AnimationData& anim, const char* actionFile)
{
	if (mAnimData && mAnimOwner)
		FB_DELETE(mAnimData);
	mAnimData = FB_NEW(AnimationData);
	*mAnimData = anim;
	mAnimOwner = true;

	mAnimData->ParseAction(actionFile);
}

void SpatialObject::PreRender()
{
	if (mAnim)
		mAnim->Update(gpTimer->GetDeltaTime());
}