#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/MeshGroup.h>
#include <Engine/RenderObjects/MeshObject.h>
#include <Engine/Animation/Animation.h>

namespace fastbird
{
MeshGroup::MeshGroup()
	: mAuxCloned(0)
	, mLastUpdateFrame(0)
	, mRootAnimated(false)
{
}

MeshGroup::~MeshGroup()
{
}
		
void MeshGroupPreRender()
{
}

IMaterial* MeshGroup::GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
{
	if (!mMeshObjects.empty())
	{
		return mMeshObjects[0].first->GetMaterial();
	}
	return 0;
}

// transform : parent space
size_t MeshGroup::AddMesh(IMeshObject* mesh, const Transformation& transform, size_t parent)
{
	mMeshObjects.push_back(MESH_OBJECTS::value_type(mesh, transform));
	mLocalTransforms.push_back(Transformation::IDENTITY);
	mChanges.push_back(true);
	
	size_t idx = mMeshObjects.size() -1;
	if (idx==0)
	{
		*mBoundingVolume = *(mesh->GetBoundingVolume());
	}
	else
	{
		mBoundingVolume->Merge(mesh->GetBoundingVolume());
	}
	mHierarchyMap[idx].mParentIndex = parent;
	mHierarchyMap[idx].mMyIndex = idx;
	if (parent !=-1)
	{
		mHierarchyMap[parent].mChildren.push_back(idx);
	}

	*mBoundingVolumeWorld = *mBoundingVolume;
	return idx;
}

const char* MeshGroup::GetNameOfMesh(size_t idx)
{
	assert(idx < mMeshObjects.size());
	return mMeshObjects[idx].first->GetName();
}

size_t MeshGroup::GetNumMeshes() const
{
	return mMeshObjects.size();
}

void MeshGroup::AddMeshRotation(size_t idx, const Quat& rot)
{
	assert(idx < mLocalTransforms.size());
	mLocalTransforms[idx].AddRotation(rot);
	mChanges[idx] = true;
	mTransformChanged = true;
	if (idx == 0)
		mRootAnimated = true;
}

const Quat& MeshGroup::GetMeshRotation(size_t idx) const
{
	assert(idx < mLocalTransforms.size());
	return mLocalTransforms[idx].GetRotation();
}

void MeshGroup::SetMeshRotation(size_t idx, const Quat& rot)
{
	assert(idx < mLocalTransforms.size());
	mLocalTransforms[idx].SetRotation(rot);
	mChanges[idx] = true;
	mTransformChanged = true;
	if (idx == 0)
		mRootAnimated = true;
}

const Vec3& MeshGroup::GetMeshOffset(size_t idx) const
{
	assert(idx < mMeshObjects.size());
	// this would be wrong in hierarcy which has more than two depths.
	return mMeshObjects[idx].second.GetTranslation();
}

IObject* MeshGroup::Clone() const
{
	MeshGroup* cloned = FB_NEW(MeshGroup);
	SpatialObject::Clone(cloned);

	for (auto it : mMeshObjects)
	{
		cloned->mMeshObjects.push_back(MESH_OBJECTS::value_type((IMeshObject*)it.first->Clone(), it.second));
		cloned->mLocalTransforms.push_back(Transformation::IDENTITY);
		cloned->mChanges.push_back(true);
		cloned->mHierarchyMap = mHierarchyMap;
	}
	cloned->mAuxCloned = mAuxCloned ? mAuxCloned : (AUXIL_MAP*)&mAuxil;
	return cloned;
}

void MeshGroup::Delete()
{
	FB_DELETE(this);
}

void MeshGroup::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	UpdateTransform();

	FB_FOREACH(it, mMeshObjects)
	{
		it->first->PreRender();
	}
}

void MeshGroup::Render()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	FB_FOREACH(it, mMeshObjects)
	{
		it->first->Render();
	}
}

void MeshGroup::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	FB_FOREACH(it, mMeshObjects)
	{
		it->first->PostRender();
	}
}

const AUXILIARIES* MeshGroup::GetAuxiliaries(size_t idx) const
{ 
	if (mAuxCloned)
	{
		return (*mAuxCloned).Find(idx) == (*mAuxCloned).end() ? 0 : &(*mAuxCloned)[idx];
	}
	else
	{
		return mAuxil.Find(idx) == mAuxil.end() ? 0 : &mAuxil[idx];
	}
}
void MeshGroup::SetAuxiliaries(size_t idx, const AUXILIARIES& aux)
{ 
	assert(!mAuxCloned);
	mAuxil[idx] = aux; 
}
void MeshGroup::AddAuxiliary(size_t idx, const AUXILIARIES::value_type& v)
{
	assert(!mAuxCloned);
	mAuxil[idx].push_back(v);
}

void MeshGroup::AddCollisionShape(size_t idx, std::pair<FBColShape::Enum, Transformation>& data)
{
	mMeshObjects[idx].first->AddCollisionShape(data);
}

void MeshGroup::SetCollisionMesh(size_t idx, IMeshObject* colMesh)
{
	mMeshObjects[idx].first->SetCollisionMesh(colMesh);
}

void MeshGroup::UpdateTransform(bool force)
{
	Timer::FRAME_PRECISION f = gFBEnv->pTimer->GetFrame();
	if (force || mLastUpdateFrame < f)
	{
		mLastUpdateFrame = f;

		if (force || mTransformChanged)
		{
			size_t num = mChanges.size();
			for (size_t i = 0; i < num; i++)
			{
				if (mChanges[i] || mTransformChanged)
				{
					// calc transform
					const Hierarchy& h = mHierarchyMap[i];
					Transformation transform;
					if (h.mParentIndex != -1)
					{
						transform = mMeshObjects[h.mParentIndex].first->GetTransform();
						transform = transform * mMeshObjects[i].second * mLocalTransforms[i];
					}
					else
					{
						if (mRootAnimated)
							transform = mTransformation * mMeshObjects[i].second * mLocalTransforms[i];
						else
							// for parents mesh, don't need to  multiply mLocalTransforms[i];
							transform = mTransformation * mMeshObjects[i].second;
					}

					mMeshObjects[i].first->SetTransform(transform);
					mChanges[i] = false;
				}
			}
		}
		mTransformChanged = false;
	}
}

//---------------------------------------------------------------------------
void MeshGroup::SetEnableHighlight(bool enable)
{
	FB_FOREACH(it, mMeshObjects)
	{
		it->first->SetEnableHighlight(enable);
	}
}

//---------------------------------------------------------------------------
void MeshGroup::SetAnimationData(const char* meshName, const AnimationData& anim, const char* actionFile)
{
	for (auto& it : mMeshObjects)
	{
		auto meshObj = it.first;
		if (strcmp(meshObj->GetName(), meshName) == 0)
		{
			meshObj->SetAnimationData(anim, actionFile);
			return;
		}
	}
	Error("Mesh group doesn't have a name %s", meshName);
}

void MeshGroup::PlayAction(const std::string& name, bool immediate, bool reverse)
{
	for (auto& it : mMeshObjects)
	{
		auto meshObj = it.first;
		auto anim = meshObj->GetAnimation();
		if (anim)
		{
			anim->PlayAction(name, immediate, reverse);
		}
	}
}

bool MeshGroup::IsActionDone(const char* action) const
{
	for (auto& it : mMeshObjects)
	{
		auto meshObj = it.first;
		auto anim = meshObj->GetAnimation();
		if (anim)
		{
			if (anim->IsActionDone(action))
				return true;
		}
	}
	return false;
}

}