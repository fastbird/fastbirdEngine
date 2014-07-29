#include <Engine/StdAfx.h>
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/IScene.h>

using namespace fastbird;

//----------------------------------------------------------------------------
SpatialObject::SpatialObject()
	:mDistToCam(-1.f)
{
}

//----------------------------------------------------------------------------
SpatialObject::~SpatialObject()
{
	mDestructing = true;
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
	mTransformation.SetTranslation(pos);
}

//----------------------------------------------------------------------------
void SpatialObject::SetRot(const Quat& rot)
{
	mTransformation.SetRotation(rot);
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
	mTransformation = t;
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