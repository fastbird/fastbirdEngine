#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/MeshGroup.h>
#include <Engine/RenderObjects/MeshObject.h>

namespace fastbird
{
MeshGroup::MeshGroup()
	: mAuxCloned(0)
{
}

MeshGroup::~MeshGroup()
{
}
		
void MeshGroupPreRender()
{
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

void MeshGroup::RotateMesh(size_t idx, const Quat& rot)
{
	assert(idx < mLocalTransforms.size());
	mLocalTransforms[idx].SetRotation(rot);
	mChanges[idx] = true;
}

const Quat& MeshGroup::GetRotation(size_t idx) const
{
	assert(idx < mLocalTransforms.size());
	return mLocalTransforms[idx].GetRotation();
}

const Vec3& MeshGroup::GetOffset(size_t idx) const
{
	assert(idx < mMeshObjects.size());
	// this would be wrong in hierarcy which has more than two depths.
	return mMeshObjects[idx].second.GetTranslation();
}

IObject* MeshGroup::Clone() const
{
	MeshGroup* cloned = new MeshGroup();
	SpatialObject::Clone(cloned);

	for each(auto it in mMeshObjects)
	{
		cloned->mMeshObjects.push_back(MESH_OBJECTS::value_type((IMeshObject*)it.first->Clone(), it.second));
		cloned->mLocalTransforms.push_back(Transformation::IDENTITY);
		cloned->mChanges.push_back(true);
		cloned->mHierarchyMap = mHierarchyMap;
	}
	cloned->mAuxCloned = mAuxCloned ? mAuxCloned : (AUXILIARIES*)&mAuxil;
	return cloned;
}

void MeshGroup::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	size_t num = mChanges.size();
	std::vector< bool > handled(num, false);
	for (size_t i=0; i<num; i++)
	{
		if (mChanges[i] && !handled[i])
		{
			// calc transform
			const Hierarchy& h = mHierarchyMap[i];
			Transformation transform;
			if (h.mParentIndex!=-1)
			{
				transform = mMeshObjects[h.mParentIndex].first->GetTransform();
				transform = transform * mMeshObjects[i].second;
			}
			else
			{
				transform = mTransformation * mMeshObjects[i].second;
			}
			transform = transform * mLocalTransforms[i];
			mMeshObjects[i].first->SetTransform(transform);			
			mChanges[i] = false;
			handled[i] = true;
		}
	}

	for (size_t i=0; i<num; i++)
	{
		mMeshObjects[i].first->PreRender();
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

}