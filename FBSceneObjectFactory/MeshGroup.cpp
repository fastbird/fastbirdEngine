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
#include "MeshGroup.h"
#include "MeshObject.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/CowPtr.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBDebugLib/Logger.h"
#include "FBTimer/Timer.h"
#include "FBAnimation/Animation.h"
#include "FBRenderer/RenderParam.h"
#include "FBStringLib/StringLib.h"
#include <map>
using namespace fb;
class MeshGroup::Impl{
public:
	struct Hierarchy
	{
		size_t mParentIndex;
		size_t mMyIndex;
		std::vector<size_t> mChildren;

		Hierarchy(){
		}

		Hierarchy(size_t parentIdx, size_t myIdx)
			: mParentIndex(parentIdx)
			, mMyIndex(myIdx)
		{
		}
	};
	MeshGroup* mSelf;
	typedef std::vector< std::pair< MeshObjectPtr, Transformation> > MESH_OBJECTS;
	MESH_OBJECTS mMeshObjects;
	typedef std::vector< Transformation > LOCAL_TRANSFORMATIONS;
	LOCAL_TRANSFORMATIONS mLocalTransforms;
	typedef std::vector< bool > CHANGES;
	CHANGES mChanges;
	typedef std::map<size_t, Hierarchy> HIERARCHY_MAP;
	CowPtr<HIERARCHY_MAP> mHierarchyMap;
	CowPtr<AUXILIARIES> mAuxiliaries;
	FRAME_PRECISION mLastUpdateFrame;
	typedef std::vector< FBCollisionShapePtr > COLLISION_SHAPES;
	CowPtr<COLLISION_SHAPES> mCollisions;

	unsigned mLastPreRendered;
	bool mRootAnimated;

	//---------------------------------------------------------------------------
	Impl(MeshGroup* self)
		: mSelf(self)
		, mLastUpdateFrame(0)
		, mRootAnimated(false)
		, mLastPreRendered(0)
	{

	}

	Impl(MeshGroup* self, const Impl& other)
		: mSelf(self)
		, mLocalTransforms(other.mLocalTransforms)		
		, mHierarchyMap(other.mHierarchyMap)
		, mAuxiliaries(other.mAuxiliaries)
		, mLastUpdateFrame(other.mLastUpdateFrame)
		, mCollisions(other.mCollisions)
		, mLastPreRendered(0)
	{
		for (auto it : other.mMeshObjects)
		{
			mMeshObjects.push_back(MESH_OBJECTS::value_type(it.first->Clone(), it.second));			
			mChanges.push_back(true);			
		}
	}

	//---------------------------------------------------------------------------
	// IRenderable
	//---------------------------------------------------------------------------
	void PreRender(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		if (mLastPreRendered == gpTimer->GetFrame())
			return;
		mLastPreRendered = gpTimer->GetFrame();

		UpdateTransform(param, paramOut, false); // and prerender children.
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		if (param.mRenderPass == PASS_NORMAL){
			auto radius = mSelf->GetRadius();
			auto distToCam = mSelf->GetDistToCam(param.mCamera);
			if (distToCam > 100 && radius < 5.0f)
				return;
			if (distToCam > 150 && radius < 10.0f)
				return;
		}

		for (auto& it : mMeshObjects){
			it.first->Render(param, paramOut);
		}
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		for (auto& it : mMeshObjects){
			it.first->PostRender(param, paramOut);
		}
	}

	void Update(TIME_PRECISION dt){
		for (auto& it : mMeshObjects){
			it.first->Update(dt);
		}
	}

	MaterialPtr GetMaterial(){
		if (mMeshObjects.empty())
			return 0;

		return mMeshObjects[0].first->GetMaterial();
	}

	void SetMaterial(MaterialPtr mat, RENDER_PASS pass){
		if (mMeshObjects.empty())
			return;

		return mMeshObjects[0].first->SetMaterial(mat, pass);
	}

	void SetMaterial(const char* path, RENDER_PASS pass){
		if (mMeshObjects.empty())
			return;

		return mMeshObjects[0].first->SetMaterial(path, pass);
	}

	//---------------------------------------------------------------------------
	// Own functions
	//---------------------------------------------------------------------------		
	MeshGroupPtr Clone() const{
		return MeshGroup::Create(*mSelf);
	}

	void SetEnableHighlight(bool enable){
		for (auto& it : mMeshObjects){
			it.first->SetEnableHighlight(enable);
		}
	}

	// order of inserting meshes is important. parent first.
	// transformation is in parent space.
	size_t AddMesh(MeshObjectPtr mesh, const Transformation& transform, size_t parent){
		mMeshObjects.push_back(MESH_OBJECTS::value_type(mesh, transform));
		mLocalTransforms.push_back(Transformation::IDENTITY);
		mChanges.push_back(true);

		size_t idx = mMeshObjects.size() - 1;
		if (idx == 0)
		{
			mSelf->SetBoundingVolume(*mesh->GetBoundingVolume());
		}
		else
		{
			mSelf->MergeBoundingVolume(mesh->GetBoundingVolume());
		}
		if (!mHierarchyMap){
			mHierarchyMap = new HIERARCHY_MAP;
		}
		Hierarchy h(parent, idx);
		mHierarchyMap->insert({ idx, h });
		if (parent != -1)
		{
			mHierarchyMap->operator[](parent).mChildren.push_back(idx);
		}

		return idx;
	}

	const char* GetNameOfMesh(size_t idx) const{
		assert(idx < mMeshObjects.size());
		return mMeshObjects[idx].first->GetName();
	}

	size_t GetNumMeshes() const{
		return mMeshObjects.size();
	}

	void AddMeshRotation(size_t idx, const Quat& rot){
		assert(idx < mLocalTransforms.size());
		mLocalTransforms[idx].AddRotation(rot);
		mChanges[idx] = true;
		mSelf->NotifyTransformChanged();
		if (idx == 0)
			mRootAnimated = true;
	}

	const Quat& GetMeshRotation(size_t idx) const{
		assert(idx < mLocalTransforms.size());
		return mLocalTransforms[idx].GetRotation();
	}

	void SetMeshRotation(size_t idx, const Quat& rot){
		assert(idx < mLocalTransforms.size());
		mLocalTransforms[idx].SetRotation(rot);
		mChanges[idx] = true;
		mSelf->NotifyTransformChanged();
		if (idx == 0)
			mRootAnimated = true;
	}

	const Vec3& GetMeshOffset(size_t idx) const{
		assert(idx < mMeshObjects.size());
		// this would be wrong in hierarcy which has more than two depths.
		return mMeshObjects[idx].second.GetTranslation();
	}

	const AUXILIARIES* GetAuxiliaries(size_t idx) const{
		if (idx == -1){
			if (mAuxiliaries){
				return mAuxiliaries.const_get();
			}
		}
		else{
			assert(idx < mMeshObjects.size());
			if (idx < mMeshObjects.size()){
				return mMeshObjects[idx].first->GetAuxiliaries();
			}			
		}
		return 0;
	}

	void SetAuxiliaries(size_t idx, const AUXILIARIES& aux){
		if (idx == -1){
			if (!mAuxiliaries){
				mAuxiliaries = new AUXILIARIES;
			}
			*mAuxiliaries = aux;
		}
		else{
			assert(idx < mMeshObjects.size());
			if (idx < mMeshObjects.size()){
				mMeshObjects[idx].first->SetAuxiliaries(aux);
			}
		}
	}

	void AddAuxiliary(const AUXILIARY& aux){
		if (!mAuxiliaries){
			mAuxiliaries = new AUXILIARIES;
		}
		mAuxiliaries->push_back(aux);
	}

	void AddAuxiliary(size_t idx, const AUXILIARY& v){
		if (idx == -1){
			AddAuxiliary(v);
		}
		else{
			assert(idx < mMeshObjects.size());
			if (idx < mMeshObjects.size()){
				mMeshObjects[idx].first->AddAuxiliary(v);
			}
		}
	}

	void SetCollisionShapes(COLLISION_INFOS& colInfos){
		if (!mCollisions){
			mCollisions = new COLLISION_SHAPES;
		}
		else{
			mCollisions->clear();
		}

		for (auto& it : colInfos){
			mCollisions->push_back(FBCollisionShape::Create(it.mColShapeType, it.mTransform, it.mCollisionMesh));
		}
	}

	void AddCollisionShape(size_t idx, std::pair<ColisionShapeType::Enum, Transformation>& data){
		if (idx == -1){
			if (!mCollisions){
				mCollisions = new COLLISION_SHAPES;
			}
			mCollisions->push_back(FBCollisionShape::Create(data.first, data.second, 0));
		}
		else{
			assert(idx < mMeshObjects.size());
			if (idx < mMeshObjects.size()){
				mMeshObjects[idx].first->AddCollisionShape(data);
			}
		}
	}	

	void UpdateTransform(const RenderParam& param, RenderParamOut* paramOut, bool force){
		FRAME_PRECISION f = gpTimer->GetFrame();
		if (force || mLastUpdateFrame < f)
		{
			mLastUpdateFrame = f;

			size_t num = mChanges.size();
			for (size_t i = 0; i < num; ++i)
			{
				auto anim = mMeshObjects[i].first->GetAnimation();
				bool animated = anim && anim->Changed();
				if (mChanges[i] || mSelf->GetTransformChanged() || animated)
				{
					for (unsigned c = i; c < num; ++c){
						auto it = mHierarchyMap.const_get()->find(c);
						if (it != mHierarchyMap.const_get()->end() && it->second.mParentIndex == i){
							mChanges[c] = true;
						}						
					}
					// calc transform
					auto it = mHierarchyMap.const_get()->find(i);
					const Hierarchy& h = it->second;
					Transformation transform;
					if (h.mParentIndex != -1)
					{
						transform = mMeshObjects[h.mParentIndex].first->GetAnimatedLocation();
						transform = transform * mMeshObjects[i].second * mLocalTransforms[i];
					}
					else
					{
						if (mRootAnimated)
							transform = mSelf->GetLocation() * mMeshObjects[i].second * mLocalTransforms[i];
						else
							// for parents mesh, don't need to  multiply mLocalTransforms[i];
							transform = mSelf->GetLocation() * mMeshObjects[i].second;
					}
					mMeshObjects[i].first->SetLocation(transform);
					mChanges[i] = false;
				}
				mMeshObjects[i].first->PreRender(param, paramOut);
			}
			mSelf->ClearTransformChanged();
		}
	}

	void PlayAction(const std::string& name, bool immediate, bool reverse){
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

	bool IsActionDone(const char* action) const{
		for (auto& it : mMeshObjects)
		{
			auto meshObj = it.first;
			if (meshObj->IsActionDone(action))
				return true;
		}
		return false;
	}

	bool IsPlayingAction() const{
		auto numMeshes = GetNumMeshes();
		for (unsigned i = 0; i < numMeshes; ++i){
			if (mMeshObjects[i].first->IsPlayingAction())
				return true;
		}

		return false;
	}

	Transformation GetToLocalTransform(unsigned meshIdx) const{
		Transformation transform = mMeshObjects[meshIdx].second;
		auto it = mHierarchyMap.const_get()->find(meshIdx);
		if (it == mHierarchyMap.const_get()->end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("MeshIndex(%u) is not in the hierarchy", meshIdx).c_str());
			return Transformation();
		}
		auto parentIdx = it->second.mParentIndex;
		while (parentIdx != -1)
		{
			transform = mMeshObjects[parentIdx].second * transform;
			auto it2 = mHierarchyMap.const_get()->find(parentIdx);
			if (it2 == mHierarchyMap.const_get()->end()){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("MeshIndex(%u) is not in the hierarchy", parentIdx).c_str());
				return Transformation();
			}
			parentIdx = it2->second.mParentIndex;
		}
		Transformation invTransform;
		transform.Inverse(invTransform);
		return invTransform;
	}

	Transformation GetToLocalTransform(const char* meshName) const{
		unsigned i = 0;
		for (auto& it : mMeshObjects)
		{
			auto meshObj = it.first;
			if (strcmp(meshObj->GetName(), meshName) == 0)
			{
				return GetToLocalTransform(i);
			}
			++i;
		}
		return Transformation();
	}

	void SetAlpha(float alpha){
		for (auto& it : mMeshObjects)
		{
			it.first->SetAlpha(alpha);
		}
	}

	void SetForceAlphaBlending(bool enable, float alpha, float forceGlow = 0.f, bool disableDepth = false){
		for (auto& it : mMeshObjects)
		{
			it.first->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
		}
	}

	void SetAmbientColor(const Color& color){
		for (auto& it : mMeshObjects)
		{
			it.first->SetAmbientColor(color);
		}
	}

	MeshObjectPtr GetMeshObject(unsigned idx) const{
		if_assert_pass(idx < mMeshObjects.size())
		{
			return mMeshObjects[idx].first;
		}
		return 0;
	}

	unsigned GetNumCollisionShapes() const{
		if (!mCollisions)
			return 0;

		return mCollisions.const_get()->size();
	}

	const FBCollisionShapeConstPtr GetCollisionShape(unsigned idx) const{
		return mCollisions.const_get()->operator[](idx);
	}

	bool HasCollisionShapes() const{
		if (!mCollisions)
			return false;
		return mCollisions.const_get()->empty();
	}

	unsigned GetNumCollisionShapes(unsigned idx) const{
		if_assert_pass(idx < mMeshObjects.size())
		{
			return mMeshObjects[idx].first->GetNumCollisionShapes();
		}
		return 0;
	}

};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(MeshGroup);

MeshGroupPtr MeshGroup::Create(const MeshGroup& other){
	return MeshGroupPtr(new MeshGroup(other), [](MeshGroup* obj){ delete obj; });
}

MeshGroup::MeshGroup()
	: mImpl(new Impl(this))
{
}

MeshGroup::MeshGroup(const MeshGroup& other)
	: SpatialSceneObject(other)
	, mImpl(new Impl(this, *other.mImpl))
{
}

MeshGroup::~MeshGroup(){

}

void MeshGroup::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void MeshGroup::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void MeshGroup::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}

void MeshGroup::Update(TIME_PRECISION dt){
	__super::Update(dt);
	mImpl->Update(dt);
}

MaterialPtr MeshGroup::GetMaterial(){
	return mImpl->GetMaterial();
}

void MeshGroup::SetMaterial(MaterialPtr mat, RENDER_PASS pass){
	mImpl->SetMaterial(mat, pass);
}

void MeshGroup::SetMaterial(const char* path, RENDER_PASS pass){
	mImpl->SetMaterial(path, pass);
}

void MeshGroup::SetEnableHighlight(bool enable) {
	mImpl->SetEnableHighlight(enable);
}

MeshGroupPtr MeshGroup::Clone() const {
	return mImpl->Clone();
}

size_t MeshGroup::AddMesh(MeshObjectPtr mesh, const Transformation& transform, size_t parent) {
	return mImpl->AddMesh(mesh, transform, parent);
}

const char* MeshGroup::GetNameOfMesh(size_t idx) {
	return mImpl->GetNameOfMesh(idx);
}

size_t MeshGroup::GetNumMeshes() const {
	return mImpl->GetNumMeshes();
}

void MeshGroup::AddMeshRotation(size_t idx, const Quat& rot) {
	mImpl->AddMeshRotation(idx, rot);
}

const Quat& MeshGroup::GetMeshRotation(size_t idx) const {
	return mImpl->GetMeshRotation(idx);
}

void MeshGroup::SetMeshRotation(size_t idx, const Quat& rot) {
	mImpl->SetMeshRotation(idx, rot);
}

const Vec3& MeshGroup::GetMeshOffset(size_t idx) const {
	return mImpl->GetMeshOffset(idx);
}

const AUXILIARIES* MeshGroup::GetAuxiliaries(size_t idx) const {
	return mImpl->GetAuxiliaries(idx);
}

void MeshGroup::SetAuxiliaries(size_t idx, const AUXILIARIES& aux) {
	mImpl->SetAuxiliaries(idx, aux);
}

void MeshGroup::AddAuxiliary(const AUXILIARY& aux) {
	mImpl->AddAuxiliary(aux);
}

void MeshGroup::AddAuxiliary(size_t idx, const AUXILIARY& v) {
	mImpl->AddAuxiliary(idx, v);
}

void MeshGroup::SetCollisionShapes(COLLISION_INFOS& colInfos) {
	mImpl->SetCollisionShapes(colInfos);
}

void MeshGroup::AddCollisionShape(size_t idx, std::pair<ColisionShapeType::Enum, Transformation>& data) {
	mImpl->AddCollisionShape(idx, data);
}

void MeshGroup::UpdateTransform(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->UpdateTransform(param, paramOut, false);
}

void MeshGroup::UpdateTransform(const RenderParam& param, RenderParamOut* paramOut, bool force) {
	mImpl->UpdateTransform(param, paramOut, force);
}

void MeshGroup::PlayAction(const std::string& name, bool immediate, bool reverse) {
	mImpl->PlayAction(name, immediate, reverse);
}

bool MeshGroup::IsActionDone(const char* action) const {
	return mImpl->IsActionDone(action);
}

bool MeshGroup::IsPlayingAction() const {
	return mImpl->IsPlayingAction();
}

Transformation MeshGroup::GetToLocalTransform(unsigned meshIdx) {
	return mImpl->GetToLocalTransform(meshIdx);
}

Transformation MeshGroup::GetToLocalTransform(const char* meshName) {
	return mImpl->GetToLocalTransform(meshName);
}

void MeshGroup::SetAlpha(float alpha) {
	mImpl->SetAlpha(alpha);
}

void MeshGroup::SetForceAlphaBlending(bool enable, float alpha, float forceGlow, bool disableDepth) {
	mImpl->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
}

void MeshGroup::SetAmbientColor(const Color& color) {
	mImpl->SetAmbientColor(color);
}

MeshObjectPtr MeshGroup::GetMeshObject(unsigned idx) {
	return mImpl->GetMeshObject(idx);
}

unsigned MeshGroup::GetNumCollisionShapes() const {
	return mImpl->GetNumCollisionShapes();
}

const FBCollisionShapeConstPtr MeshGroup::GetCollisionShape(unsigned idx) const {
	return mImpl->GetCollisionShape(idx);
}

bool MeshGroup::HasCollisionShapes() const {
	return mImpl->HasCollisionShapes();
}

unsigned MeshGroup::GetNumCollisionShapes(unsigned idx) const {
	return mImpl->GetNumCollisionShapes(idx);
}

