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
#include "MeshFacade.h"
#include "EngineFacade.h"
#include "FBSceneObjectFactory/MeshObject.h"
#include "FBSceneObjectFactory/MeshGroup.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"
#include "FBSceneManager/Scene.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Camera.h"
using namespace fb;
class MeshFacade::Impl{
public:
	MeshFacadeWeakPtr mSelfPtr;
	MeshObjectPtr mMeshObject;
	MeshGroupPtr mMeshGroup;

	//---------------------------------------------------------------------------
	Impl(){

	}

	~Impl(){

	}

	MeshImportDesc MakeImportDesc(const MeshLoadOptions& options){
		MeshImportDesc desc;
		desc.generateTangent = options.generateTangent;
		desc.keepMeshData = options.keepMeshData;
		desc.mergeMaterialGroups = options.mergeMaterialGroups;
		desc.oppositeCull = options.oppositeCull;
		desc.useIndexBuffer = options.useIndexBuffer;
		desc.yzSwap = options.yzSwap;
		return desc;
	}

	bool LoadMeshObject(const char* daePath, const MeshLoadOptions& options){
		MeshImportDesc importDesc = MakeImportDesc(options);
		auto loaded = SceneObjectFactory::GetInstance().CreateMeshObject(daePath, importDesc);
		if (loaded){
			mMeshGroup = 0;
			mMeshObject = loaded;
			return true;
		}
		return false;
	}

	void CreateEmptyMeshObject(){
		mMeshGroup = 0;
		mMeshObject = MeshObject::Create();
	}

	bool LoadMeshGroup(const char* daePath){
		auto loaded = SceneObjectFactory::GetInstance().CreateMeshGroup(daePath);
		if (loaded){
			mMeshObject = 0;
			mMeshGroup = loaded;
			return true;
		}
		return false;
	}

	bool IsVaildMesh() const{
		return IsMeshObject() || IsMeshGroup();
	}

	bool IsMeshObject() const{
		return mMeshObject != 0;
	}

	bool IsMeshGroup() const{
		return mMeshGroup != 0;
	}

	void SetGameType(int type){
		if (mMeshObject)
			mMeshObject->SetGameType(type);
		else if (mMeshGroup)
			mMeshGroup->SetGameType(type);
	}

	void SetGameId(unsigned id){
		if (mMeshObject)
			mMeshObject->SetGameId(id);
		else if (mMeshGroup)
			mMeshGroup->SetGameId(id);
	}

	void SetGamePtr(void* ptr){
		if (mMeshObject)
			mMeshObject->SetGamePtr(ptr);
		else if (mMeshGroup)
			mMeshGroup->SetGamePtr(ptr);
	}

	/// Combination of SceneObjectFlag::Enum
	void ModifyObjFlag(unsigned flag, bool enable){
		if (mMeshObject)
			mMeshObject->ModifyObjFlag(flag, enable);
		else if (mMeshGroup)
			mMeshGroup->ModifyObjFlag(flag, enable);
	}

	void SetEnableHighlight(bool enable){
		if (mMeshObject)
			mMeshObject->SetEnableHighlight(enable);
		else if (mMeshGroup)
			mMeshGroup->SetEnableHighlight(enable);
	}

	MaterialPtr GetMaterial() const{
		if (mMeshObject)
			return mMeshObject->GetMaterial();
		else if (mMeshGroup)
			return mMeshGroup->GetMaterial();

		return 0;
	}

	void SetMaterialParameter(unsigned idx, const Vec4& value){
		auto material = GetMaterial();
		if (material){
			material->SetMaterialParameter(idx, value);
		}
	}

	void SetMaterial(MaterialPtr material){
		if (mMeshObject)
			mMeshObject->SetMaterial(material, PASS_NORMAL);
		else if (mMeshGroup)
			mMeshGroup->SetMaterial(material, PASS_NORMAL);
	}

	void SetMaterial(const char* path){
		if (mMeshObject)
			mMeshObject->SetMaterial(path, PASS_NORMAL);
		else if (mMeshGroup)
			mMeshGroup->SetMaterial(path, PASS_NORMAL);
	}

	void SetVisible(bool visible){
		if (mMeshObject)
			mMeshObject->SetVisible(visible);
		else if (mMeshGroup)
			mMeshGroup->SetVisible(visible);
	}

	bool GetVisible() const{
		if (mMeshObject)
			return mMeshObject->GetVisible();
		else if (mMeshGroup)
			return mMeshGroup->GetVisible();
		return false;
	}

	bool AttachToScene(){
		auto scene = EngineFacade::GetInstance().GetMainScene();
		return AttachToScene(scene);
	}

	bool AttachToCurrentScene(){
		auto scene = EngineFacade::GetInstance().GetCurrentScene();
		return AttachToScene(scene);
	}

	bool AttachToScene(IScenePtr scene){
		if (!scene)
			return false;
		if(mMeshObject){
			return scene->AttachObjectFB(mMeshObject);
		}
		else if (mMeshGroup){
			return scene->AttachObjectFB(mMeshGroup);
		}

		return false;
	}

	bool DetachFromScene(bool includingRtt){
		if (mMeshObject){
			mMeshObject->DetachFromScene(includingRtt);
		}
		else if (mMeshGroup){
			mMeshGroup->DetachFromScene(includingRtt);
		}
		return false;
	}

	bool IsAttached() const{
		if (mMeshObject)
			return mMeshObject->IsAttached();
		else if (mMeshGroup)
			return mMeshGroup->IsAttached();
		return false;
	}

	bool IsAttachedToMain() const{
		auto mainScene = EngineFacade::GetInstance().GetMainScene();
		if (mMeshObject)
			return mMeshObject->IsAttached(mainScene);
		else if (mMeshGroup)
			return mMeshGroup->IsAttached(mainScene);
		return false;
	}

	bool IsAttached(IScenePtr scene) const{
		if (mMeshObject)
			return mMeshObject->IsAttached(std::static_pointer_cast<Scene>(scene));
		else if (mMeshGroup)
			return mMeshGroup->IsAttached(std::static_pointer_cast<Scene>(scene));
		return false;
	}

	void SetAlpha(float alpha){
		if (mMeshObject)
			mMeshObject->SetAlpha(alpha);
		else if (mMeshGroup)
			mMeshGroup->SetAlpha(alpha);
	}

	void SetForceAlphaBlending(bool enable, float alpha, float forceGlow, bool disableDepth){
		if (mMeshObject)
			mMeshObject->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
		else if (mMeshGroup)
			mMeshGroup->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
	}

	void SetAmbientColor(const Color& color){
		if (mMeshObject)
			mMeshObject->SetAmbientColor(color);
		else if (mMeshGroup)
			mMeshGroup->SetAmbientColor(color);
	}

	const AUXILIARIES* GetAuxiliaries() const{
		if (mMeshObject)
			return mMeshObject->GetAuxiliaries();
		else if (mMeshGroup)
			mMeshGroup->GetAuxiliaries(-1);

		return 0;
	}

	const AUXILIARIES* GetAuxiliaries(unsigned idx) const{
		if (mMeshObject){
			return mMeshObject->GetAuxiliaries();
		}
		else if (mMeshGroup){
			return mMeshGroup->GetAuxiliaries(idx);
		}

		return 0;
	}

	Transformation GetAuxiliaryWorldTransformation(const char* name, bool& outFound) const{
		outFound = false;
		if (mMeshObject){
			auto p = mMeshObject->GetAuxiliaries();
			if (!p){
				return Transformation::IDENTITY;
			}
			const auto& aux = *p;
			for (unsigned i = 0; i < aux.size(); i++){
				if (aux[i].first == name){
					outFound = true;
					return mMeshObject->GetLocation() * aux[i].second;
				}
			}
		}
		else if (mMeshGroup){
			auto p = mMeshGroup->GetAuxiliaries(-1);
			if (p){
				auto aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first == name){
						outFound = true;
						return mMeshGroup->GetLocation() * aux[i].second;
					}
				}
			}
			unsigned numMeshes = mMeshGroup->GetNumMeshes();
			for (unsigned m = 0; m < numMeshes; ++m){
				auto p = mMeshGroup->GetAuxiliaries(m);
				if (!p){
					continue;
				}
				const auto& aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first == name){
						outFound = true;
						return mMeshGroup->GetLocation() * aux[i].second;
					}
				}
			}
		}
		return Transformation::IDENTITY;
	}

	Transformations GetAuxiliaryWorldTransformations(const char* name) const{
		if (mMeshObject){
			Transformations ret = GetAuxiliaryTransformations(name);
			for (auto& it : ret){
				it = mMeshObject->GetLocation() * it;
			}
			return ret;
		}
		else if (mMeshGroup){
			Transformations ret;
			auto p = mMeshGroup->GetAuxiliaries(-1);
			if (p){
				auto& aux = *p;
				for (auto& it : aux){
					if (it.first.find(name) != std::string::npos){
						ret.push_back(mMeshGroup->GetLocation() * it.second);
					}
				}
			}
			unsigned numMeshes = mMeshGroup->GetNumMeshes();
			for (unsigned m = 0; m < numMeshes; ++m){
				auto p = mMeshGroup->GetAuxiliaries(m);
				if (!p){
					continue;
				}
				const auto& aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first.find(name) != std::string::npos){
						ret.push_back(mMeshGroup->GetMeshObject(m)->GetLocation() *aux[i].second);
					}
				}
			}
			return ret;
		}
		return Transformations();
	}

	Transformations GetAuxiliaryTransformations(const char* name) const{
		Transformations ret;
		if (mMeshObject){
			auto p = mMeshObject->GetAuxiliaries();
			if (!p){
				return Transformations();
			}
			
			const auto& aux = *p;
			for (unsigned i = 0; i < aux.size(); i++){
				if (aux[i].first.find(name) != std::string::npos){
					ret.push_back(aux[i].second);
				}
			}
		}
		else if (mMeshGroup){			
			auto p = mMeshGroup->GetAuxiliaries(-1);
			if (p){
				auto aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first.find(name) != std::string::npos){
						ret.push_back(aux[i].second);
					}
				}
			}
			unsigned numMeshes = mMeshGroup->GetNumMeshes();
			for (unsigned m = 0; m < numMeshes; ++m){
				auto p = mMeshGroup->GetAuxiliaries(m);
				if (!p){
					continue;
				}
				const auto& aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first.find(name) != std::string::npos){
						ret.push_back(aux[i].second);
					}
				}
			}
		}
		return ret;
	}

	Vec3s GetAuxiliaryPositions(const char* name) const{
		Vec3s ret;
		if (mMeshObject){
			auto p = mMeshObject->GetAuxiliaries();
			if (!p){
				return ret;
			}			
			const auto& aux = *p;
			for (unsigned i = 0; i < aux.size(); i++){
				if (aux[i].first.find(name) != std::string::npos){
					ret.push_back(aux[i].second.GetTranslation());
				}
			}
		}
		else if (mMeshGroup){
			auto p = mMeshGroup->GetAuxiliaries(-1);
			if (p){
				auto& aux = *p;
				for (auto& it : aux){
					if (it.first.find(name) != std::string::npos){
						ret.push_back(it.second.GetTranslation());
					}
				}
			}
			unsigned numMeshes = mMeshGroup->GetNumMeshes();
			for (unsigned m = 0; m < numMeshes; ++m){
				auto p = mMeshGroup->GetAuxiliaries(m);
				if (!p){
					continue;
				}
				const auto& aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first.find(name) != std::string::npos){
						ret.push_back(aux[i].second.GetTranslation());
					}
				}
			}
		}
		return ret;
	}

	Vec3s GetAuxiliaryWorldPositions(const char* name) const {
		if (mMeshObject){
			auto ret = GetAuxiliaryPositions(name);
			for (auto& it : ret){
				it = mMeshObject->GetLocation().ApplyForward(it);
			}
			return ret;
		}
		else if (mMeshGroup){
			Vec3s ret;
			auto p = mMeshGroup->GetAuxiliaries(-1);
			if (p){
				auto& aux = *p;
				for (auto& it : aux){
					if (it.first.find(name) != std::string::npos){
						ret.push_back(mMeshGroup->GetLocation().ApplyForward(it.second.GetTranslation()));
					}
				}
			}
			unsigned numMeshes = mMeshGroup->GetNumMeshes();
			for (unsigned m = 0; m < numMeshes; ++m){
				auto p = mMeshGroup->GetAuxiliaries(m);
				if (!p){
					continue;
				}
				const auto& aux = *p;
				for (unsigned i = 0; i < aux.size(); i++){
					if (aux[i].first.find(name) != std::string::npos){
						ret.push_back(mMeshGroup->GetMeshObject(m)->GetLocation().ApplyForward(
							aux[i].second.GetTranslation()));
					}
				}
			}
			return ret;
		}
		return Vec3s();
	}

	const Transformation& GetTransformation() const{
		if (mMeshObject)
			return mMeshObject->GetLocation();
		else if (mMeshGroup)
			return mMeshGroup->GetLocation();

		return Transformation::IDENTITY;
	}

	void SetTransformation(const Transformation& transform){
		if (mMeshObject)
			mMeshObject->SetLocation(transform);
		else if (mMeshGroup)
			mMeshGroup->SetLocation(transform);
	}

	void SetPosition(const Vec3& pos){
		if (mMeshObject)
			mMeshObject->SetPosition(pos);
		else if (mMeshGroup)
			mMeshGroup->SetPosition(pos);
	}

	const Vec3& GetPosition() const{
		if (mMeshObject)
			return mMeshObject->GetPosition();
		else if (mMeshGroup)
			return mMeshGroup->GetPosition();
		return Vec3::ZERO;
	}

	void SetRotation(const Quat& rot){
		if (mMeshObject)
			mMeshObject->SetRotation(rot);
		else if (mMeshGroup)
			mMeshGroup->SetRotation(rot);
	}

	const Quat& GetRotation() const{
		if (mMeshObject)
			return mMeshObject->GetRotation();
		else if (mMeshGroup)
			return mMeshGroup->GetRotation();
		return Quat::IDENTITY;
	}

	void SetScale(const Vec3& scale){
		if (mMeshObject)
			mMeshObject->SetScale(scale);
		else if (mMeshGroup)
			mMeshGroup->SetScale(scale);
	}

	const BoundingVolumePtr GetBoundingVolume() const{
		if (mMeshObject)
			return mMeshObject->GetBoundingVolume();
		else if (mMeshGroup)
			return mMeshGroup->GetBoundingVolume();
		return 0;
	}

	const BoundingVolumePtr GetBoundingVolumeWorld() const{
		if (mMeshObject)
			return mMeshObject->GetBoundingVolumeWorld();
		else if (mMeshGroup)
			return mMeshGroup->GetBoundingVolumeWorld();
		return 0;
	}

	bool RayCast(const Ray3& ray, Vec3& pos, const ModelTriangle** tri){
		if (mMeshObject)
			return mMeshObject->RayCast(ray, pos, tri);
		else if (mMeshGroup){
			assert(0 && "Not Implemented.");
		}
		return false;
	}

	bool CheckNarrowCollision(const BoundingVolume* bv){
		if (mMeshObject)
			return mMeshObject->CheckNarrowCollision(bv);
		else if (mMeshGroup){
			assert(0 && "Not Implemented");
		}
		return true;
	}

	Ray3::IResult CheckNarrowCollisionRay(const Ray3& ray){
		if (mMeshObject)
			return mMeshObject->CheckNarrowCollisionRay(ray);
		else if (mMeshGroup){
			assert(0 && "Not Implemented.");
		}

		return Ray3::IResult{ false, 0.f };
	}

	bool HasCollisionShapes() const{
		if (mMeshObject)
			return mMeshObject->HasCollisionShapes();
		else if (mMeshGroup)
			return mMeshGroup->HasCollisionShapes();
		return false;
	}

	CollisionShapeInfos GetCollisionShapeInfos() const{
		CollisionShapeInfos ret;
		std::vector<MeshObjectPtr> meshes;
		if (mMeshObject)
			meshes.push_back(mMeshObject);
		else{
			unsigned num = mMeshGroup->GetNumMeshes();
			for (unsigned i = 0; i < num; ++i){
				meshes.push_back(mMeshGroup->GetMeshObject(i));
			}
		}
		for (auto& mesh : meshes){
			unsigned numShapes = mesh->GetNumCollisionShapes();
			for (unsigned s = 0; s < numShapes; ++s){
				auto colShape = mesh->GetCollisionShape(s);
				assert(colShape);
				ret.push_back(CollisionShapeInfo());
				CollisionShapeInfo& info = ret.back();
				info.mType = colShape->GetColShape();
				info.mOffset = colShape->GetOffset();
				info.mRot = colShape->GetRot();
				info.mScale = colShape->GetScale();
				auto colMesh = colShape->GetCollisionMesh();
				if (colMesh){
					unsigned numVertices;
					auto src = colMesh->GetPositions(0, numVertices);
					info.mPositions.assign(src, src + numVertices);
				}
			}
		}
		return ret;		
	}

	Vec3 GetRandomPosInVolume(const Vec3* nearWorld){
		if (mMeshObject){
			return mMeshObject->GetRandomPosInVolume(nearWorld);
		}
		else if (mMeshGroup){

		}
		return Vec3::ZERO;
	}

	void PlayAction(const char* action, bool immediate, bool reverse){
		if (mMeshObject)
			mMeshObject->PlayAction(action, immediate, reverse);
		else if (mMeshGroup)
			mMeshGroup->PlayAction(action, immediate, reverse);
	}

	bool IsPlayingAction() const{
		if (mMeshObject)
			return mMeshObject->IsPlayingAction();
		else if (mMeshGroup)
			return mMeshGroup->IsPlayingAction();

		return false;
	}
	
	bool IsActionDone(const char* action) const{
		if (mMeshObject)
			return mMeshObject->IsActionDone(action);
		else if (mMeshGroup)
			return mMeshGroup->IsActionDone(action);
		return true;
	}

	unsigned GetNumMeshes() const{
		if (mMeshObject)
			return 1;
		else if (mMeshGroup)
			return mMeshGroup->GetNumMeshes();
		return 0;
	}

	const Vec3& GetMeshOffset(unsigned idx) const{
		if (mMeshObject)
			return mMeshObject->GetPosition();
		else if (mMeshGroup)
			return mMeshGroup->GetMeshOffset(idx);
		return Vec3::ZERO;
	}

	void SetMeshRotation(unsigned idx, const Quat& rot){
		if (mMeshObject)
			mMeshObject->SetRotation(rot);
		else if (mMeshGroup)
			mMeshGroup->SetMeshRotation(idx, rot);
	}

	SpatialObjectPtr GetSpatialObject() const{
		if (mMeshObject)
			return std::dynamic_pointer_cast<SpatialObject>(mMeshObject);
		else if (mMeshGroup)
			return std::dynamic_pointer_cast<SpatialObject>(mMeshGroup);
		return 0;
	}

};

std::vector<MeshFacadeWeakPtr> sMeshes;
//---------------------------------------------------------------------------
MeshFacadePtr MeshFacade::Create(){
	MeshFacadePtr p(new MeshFacade, [](MeshFacade* obj){ delete obj; });
	sMeshes.push_back(p);
	p->mImpl->mSelfPtr = p;
	return p;
}

MeshFacade::MeshFacade()
	: mImpl(new Impl)
{

}

MeshFacade::~MeshFacade(){

}

MeshFacadePtr MeshFacade::LoadMeshObject(const char* daePath) {
	if (mImpl->LoadMeshObject(daePath, MeshLoadOptions()))
		return mImpl->mSelfPtr.lock();
	return 0;
}

MeshFacadePtr MeshFacade::LoadMeshObject(const char* daePath, const MeshLoadOptions& options){
	if (mImpl->LoadMeshObject(daePath, options))
		return mImpl->mSelfPtr.lock();
	return 0;
}

MeshFacadePtr MeshFacade::CreateEmptyMeshObject(){
	mImpl->CreateEmptyMeshObject();
	return mImpl->mSelfPtr.lock();
}

MeshFacadePtr MeshFacade::LoadMeshGroup(const char* daePath){
	if (mImpl->LoadMeshGroup(daePath))
		return mImpl->mSelfPtr.lock();
	return 0;
}

void MeshFacade::SetMeshObject(MeshObjectPtr mesh){
	mImpl->mMeshObject = mesh;
	mImpl->mMeshGroup = 0;
}

MeshFacadePtr MeshFacade::Clone(){
	MeshFacadePtr newMesh = MeshFacade::Create();
	if (mImpl->mMeshObject){
		newMesh->mImpl->mMeshObject = mImpl->mMeshObject->Clone();
	}
	else if (mImpl->mMeshGroup){
		newMesh->mImpl->mMeshGroup = mImpl->mMeshGroup->Clone();
	}
	return newMesh;
}

const char* MeshFacade::GetName() const{
	if (mImpl->mMeshObject)
		return mImpl->mMeshObject->GetName();
	else if (mImpl->mMeshGroup)
		return mImpl->mMeshGroup->GetName();
	return "";
}

bool MeshFacade::IsVaildMesh() const{
	return mImpl->IsVaildMesh();
}

bool MeshFacade::IsMeshObject() const{
	return mImpl->IsMeshObject();
}

bool MeshFacade::IsMeshGroup() const{
	return mImpl->IsMeshGroup();
}

void MeshFacade::SetGameType(int type) {
	mImpl->SetGameType(type);
}

void MeshFacade::SetGameId(unsigned id) {
	mImpl->SetGameId(id);
}

void MeshFacade::SetGamePtr(void* ptr) {
	mImpl->SetGamePtr(ptr);
}

void MeshFacade::ModifyObjFlag(unsigned flag, bool enable) {
	mImpl->ModifyObjFlag(flag, enable);
}

void MeshFacade::SetEnableHighlight(bool enable) {
	mImpl->SetEnableHighlight(enable);
}

MaterialPtr MeshFacade::GetMaterial() const {
	return mImpl->GetMaterial();
}

void MeshFacade::SetMaterialParameter(unsigned idx, const Vec4& value){
	return mImpl->SetMaterialParameter(idx, value);
}

void MeshFacade::SetMaterial(MaterialPtr material) {
	mImpl->SetMaterial(material);
}

void MeshFacade::SetMaterial(const char* path) {
	mImpl->SetMaterial(path);
}

void MeshFacade::SetVisible(bool visible){
	mImpl->SetVisible(visible);
}
bool MeshFacade::GetVisible() const{
	return mImpl->GetVisible();
}

bool MeshFacade::AttachToScene() {
	return mImpl->AttachToScene();
}

bool MeshFacade::AttachToCurrentScene(){
	return mImpl->AttachToCurrentScene();
}

bool MeshFacade::AttachToScene(IScenePtr scene){
	return mImpl->AttachToScene(scene);
}

bool MeshFacade::DetachFromScene() {
	return mImpl->DetachFromScene(false);
}

bool MeshFacade::DetachFromScene(bool includingRtt){
	return mImpl->DetachFromScene(includingRtt);
}


bool MeshFacade::IsAttached() const {
	return mImpl->IsAttached();
}

bool MeshFacade::IsAttachedToMain() const{
	return mImpl->IsAttachedToMain();
}

bool MeshFacade::IsAttached(IScenePtr scene) const{
	return mImpl->IsAttached(scene);
}

void MeshFacade::SetAlpha(float alpha) {
	mImpl->SetAlpha(alpha);
}

void MeshFacade::SetForceAlphaBlending(bool enable, float alpha, float forceGlow, bool disableDepth) {
	mImpl->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
}

void MeshFacade::SetAmbientColor(const Color& color) {
	mImpl->SetAmbientColor(color);
}

const AUXILIARIES* MeshFacade::GetAuxiliaries() const {
	return mImpl->GetAuxiliaries();
}

const AUXILIARIES* MeshFacade::GetAuxiliaries(unsigned idx) const{
	return mImpl->GetAuxiliaries(idx);
}

Transformation MeshFacade::GetAuxiliaryWorldTransformation(const char* name, bool& outFound) const{
	return mImpl->GetAuxiliaryWorldTransformation(name, outFound);
}

Transformations MeshFacade::GetAuxiliaryWorldTransformations(const char* name) const{
	return mImpl->GetAuxiliaryWorldTransformations(name);
}

Transformations MeshFacade::GetAuxiliaryTransformations(const char* name) const{
	return mImpl->GetAuxiliaryTransformations(name);
}

Vec3s MeshFacade::GetAuxiliaryPositions(const char* name) const{
	return mImpl->GetAuxiliaryPositions(name);
}

Vec3s MeshFacade::GetAuxiliaryWorldPositions(const char* name) const{
	return mImpl->GetAuxiliaryWorldPositions(name);
}

const Transformation& MeshFacade::GetTransformation() const {
	return mImpl->GetTransformation();
}

void MeshFacade::SetTransformation(const Transformation& transform) {
	mImpl->SetTransformation(transform);
}

void MeshFacade::SetPosition(const Vec3& pos) {
	mImpl->SetPosition(pos);
}

const Vec3& MeshFacade::GetPosition() const{
	return mImpl->GetPosition();
}

void MeshFacade::SetRotation(const Quat& rot) {
	mImpl->SetRotation(rot);
}

const Quat& MeshFacade::GetRotation() const{
	return mImpl->GetRotation();
}

void MeshFacade::SetScale(const Vec3& scale) {
	mImpl->SetScale(scale);
}

const BoundingVolumePtr MeshFacade::GetBoundingVolume() const {
	return mImpl->GetBoundingVolume();
}

const BoundingVolumePtr MeshFacade::GetBoundingVolumeWorld() const {
	return mImpl->GetBoundingVolumeWorld();
}

bool MeshFacade::RayCast(const Ray3& ray, Vec3& pos, const ModelTriangle** tri) {
	return mImpl->RayCast(ray, pos, tri);
}

bool MeshFacade::CheckNarrowCollision(const BoundingVolume* bv) {
	return mImpl->CheckNarrowCollision(bv);
}

Ray3::IResult MeshFacade::CheckNarrowCollisionRay(const Ray3& ray) {
	return mImpl->CheckNarrowCollisionRay(ray);
}

bool MeshFacade::HasCollisionShapes() const {
	return mImpl->HasCollisionShapes();
}

CollisionShapeInfos MeshFacade::GetCollisionShapeInfos() const{
	return mImpl->GetCollisionShapeInfos();
}

Vec3 MeshFacade::GetRandomPosInVolume(const Vec3* nearWorld) {
	return mImpl->GetRandomPosInVolume(nearWorld);
}

void MeshFacade::PlayAction(const char* action, bool immediate, bool reverse) {
	mImpl->PlayAction(action, immediate, reverse);
}

bool MeshFacade::IsPlayingAction() const {
	return mImpl->IsPlayingAction();
}

bool MeshFacade::IsActionDone(const char* action) const{
	return mImpl->IsActionDone(action);
}

unsigned MeshFacade::GetNumMeshes() const{
	return mImpl->GetNumMeshes();
}

const Vec3& MeshFacade::GetMeshOffset(unsigned idx) const{
	return mImpl->GetMeshOffset(idx);
}

void MeshFacade::SetMeshRotation(unsigned idx, const Quat& rot){
	return mImpl->SetMeshRotation(idx, rot);
}

SpatialObjectPtr MeshFacade::GetSpatialObject() const{
	return mImpl->GetSpatialObject();
}

void MeshFacade::AddAsCloudVolume(ScenePtr scene){
	if (mImpl->mMeshObject)
		scene->AddCloudVolume(mImpl->mMeshObject);
}

void MeshFacade::SetUseDynamicVB(MeshVertexBufferType::Enum type, bool use){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetUseDynamicVB(type, use);
}

void MeshFacade::StartModification(){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->StartModification();
}

void MeshFacade::SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetPositions(matGroupIdx, p, numVertices);
}

void MeshFacade::SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetNormals(matGroupIdx, n, numNormals);
}

void MeshFacade::SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetUVs(matGroupIdx, uvs, numUVs);
}

void MeshFacade::SetColors(int matGroupIdx, const DWORD* colors, size_t numColors){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetColors(matGroupIdx, colors, numColors);
}

void MeshFacade::EndModification(bool keepMeshData){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->EndModification(keepMeshData);
}

void MeshFacade::SetTopology(PRIMITIVE_TOPOLOGY topology){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetTopology(topology);
}

void MeshFacade::ClearVertexBuffers(){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->ClearVertexBuffers();
}

void MeshFacade::SetRadius(Real r){
	if (mImpl->mMeshObject)
		mImpl->mMeshObject->SetRadius(r);
}

Real MeshFacade::GetRadius() const{
	if (mImpl->mMeshObject)
		return mImpl->mMeshObject->GetRadius();

	Logger::Log(FB_ERROR_LOG_ARG, "Mesh is not loaded.");
	return 0.5f;
}

Vec3* MeshFacade::GetPositionVertices(int matGroupIdx, size_t& outNumPositions){
	if (mImpl->mMeshObject)
		return mImpl->mMeshObject->GetPositions(matGroupIdx, outNumPositions);

	return 0;
}

void MeshFacade::OnMainCameraTargeted(){
	if (mImpl->mMeshObject){
		EngineFacade::GetInstance().GetMainCamera()->SetTarget(mImpl->mMeshObject);
	}
	else if (mImpl->mMeshGroup){
		EngineFacade::GetInstance().GetMainCamera()->SetTarget(mImpl->mMeshGroup);
	}
}