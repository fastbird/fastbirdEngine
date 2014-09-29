#include <Engine/StdAfx.h>
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/IScene.h>
#include <Engine/ICamera.h>

using namespace fastbird;

//----------------------------------------------------------------------------
SpatialObject::SpatialObject()
	:mDistToCam(-1.f)
	, mPrevPos(0, 0, 0)
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

	for each(auto scene in mScenes)
	{
		scene->DetachObject(this);
	}
}

//----------------------------------------------------------------------------
void SpatialObject::Clone(IObject* cloned) const
{
	__super::Clone(cloned);
	SpatialObject* object = (SpatialObject*)cloned;
	object->mTransformation = mTransformation;
	object->mTransformation.SetTranslation(mTransformation.GetTranslation()+Vec3(1.f, 0, 0));
	object->mDistToCam = mDistToCam;
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