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
#include "FBCommonHeaders/Types.h"
#include "FBRenderer/PrimitiveTopology.h"
#include "FBSceneManager/SceneObjectFlag.h"
#include "FBSceneObjectFactory/MeshVertexBufferType.h"
#include "FBSceneObjectFactory/MeshCamera.h"
#include "CollisionShapeInfo.h"
#include "MeshLoadOptions.h"
namespace fb{
	struct ModelTriangle;
	class Vec4;
	class Color;
	class Transformation;
	typedef std::vector< std::pair<std::string, Transformation> > AUXILIARIES;
	FB_DECLARE_SMART_PTR(SpatialObject);
	FB_DECLARE_SMART_PTR(BoundingVolume);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(MeshObject);	
	FB_DECLARE_SMART_PTR(Scene);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(MeshFacade);
	class FB_DLL_ENGINEFACADE MeshFacade{
		FB_DECLARE_PIMPL_NON_COPYABLE(MeshFacade);
		MeshFacade();
		~MeshFacade();

	public:
		static MeshFacadePtr Create();		
		/// Returned self.
		MeshFacadePtr LoadMeshObject(const char* daePath);
		MeshFacadePtr LoadMeshObject(const char* daePath, const MeshLoadOptions& options);
		MeshFacadePtr CreateEmptyMeshObject();
		MeshFacadePtr LoadMeshGroup(const char* daePath);
		/// Internal purpose
		void SetMeshObject(MeshObjectPtr mesh);
		MeshFacadePtr Clone();
		const char* GetName() const;

		bool IsVaildMesh() const;
		bool IsMeshObject() const;
		bool IsMeshGroup() const;

		void SetGameType(int type);
		void SetGameId(unsigned id);
		void SetGamePtr(void* ptr);
		/// Combination of SceneObjectFlag::Enum
		void ModifyObjFlag(unsigned flag, bool enable);		
		void SetEnableHighlight(bool enable);
		MaterialPtr GetMaterial() const;
		void SetMaterialParameter(unsigned idx, const Vec4& value);
		void SetMaterial(MaterialPtr material);
		void SetMaterial(const char* path);
		void SetVisible(bool visible);
		bool GetVisible() const;
		bool AttachToScene();
		/// Attach to the current scene which is bound to the current render target.
		bool AttachToCurrentScene();
		bool AttachToScene(IScenePtr scene);
		/// not including RTT
		bool DetachFromScene();
		bool DetachFromScene(bool includingRtt);
		// attached any of scenes
		bool IsAttached() const;
		bool IsAttachedToMain() const;
		bool IsAttached(IScenePtr scene) const;
		void SetAlpha(float alpha);
		void SetForceAlphaBlending(bool enable, float alpha, float forceGlow, bool disableDepth);
		void SetAmbientColor(const Color& color);
		/// Returns auxiliaries for mesh object, and master auxiliaries for the mesh group
		const AUXILIARIES* GetAuxiliaries() const;		
		/// Returns auxiliaries for sub meshes in the mesh gorup.
		/// calling with -1 is same with calling GetAuxiliaries()
		const AUXILIARIES* GetAuxiliaries(unsigned idx) const;
		Transformation GetAuxiliaryWorldTransformation(const char* name, bool& outFound) const;
		Transformations GetAuxiliaryWorldTransformations(const char* name) const;
		Transformations GetAuxiliaryTransformations(const char* name) const;
		const MeshCamera& GetMeshCameraAndWorldTransformation(const char* name, Transformation& outWorld, bool& outFound);
		Vec3s GetAuxiliaryWorldPositions(const char* name) const;
		Vec3s GetAuxiliaryPositions(const char* name) const;
		
		
		const Transformation& GetTransformation() const;
		void SetTransformation(const Transformation& transform);
		void SetPosition(const Vec3& pos);
		const Vec3& GetPosition() const;
		void SetRotation(const Quat& rot);
		const Quat& GetRotation() const;
		void SetScale(const Vec3& scale);
		const BoundingVolumePtr GetBoundingVolume() const;
		const BoundingVolumePtr GetBoundingVolumeWorld() const;
		bool RayCast(const Ray& ray, Vec3& pos, const ModelTriangle** tri);
		bool CheckNarrowCollision(const BoundingVolume* bv);
		Ray::IResult CheckNarrowCollisionRay(const Ray& ray);
		bool HasCollisionShapes() const;		
		CollisionShapeInfos GetCollisionShapeInfos() const;
		ColisionShapeType::Enum GetColShapeType() const;
		const Vec3& GetColShapeOffset(unsigned idx) const;
		const Quat& GetColShapeRot(unsigned idx) const;
		const Vec3& GetColShapeScale(unsigned idx) const;
		const Vec3* GetMeshColShapePositions(unsigned dolShapeIdx, unsigned& outNumPosition) const;

		Vec3 GetRandomPosInVolume(const Vec3* nearWorld);
		void PlayAction(const char* action, bool immediate, bool reverse);
		bool IsPlayingAction() const;		
		bool IsActionDone(const char* action) const;

		unsigned GetNumMeshes() const;
		const Vec3& GetMeshOffset(unsigned idx) const;
		void SetMeshRotation(unsigned idx, const Quat& rot);		
		SpatialObjectPtr GetSpatialObject() const;
		void AddAsCloudVolume(ScenePtr scene);
		void SetUseDynamicVB(MeshVertexBufferType::Enum type, bool use);
		void StartModification();
		void SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices);
		void SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals);
		void SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs);
		void SetColors(int matGroupIdx, const DWORD* colors, size_t numColors);
		void EndModification(bool keepMeshData);
		void SetTopology(PRIMITIVE_TOPOLOGY topology);
		void ClearVertexBuffers();
		void SetRadius(Real r);
		Real GetRadius() const;
		Vec3* GetPositionVertices(int matGroupIdx, size_t& outNumPositions);
		void OnMainCameraTargeted();
		void RenderSimple(bool bindPosOnly);
		void SetDebug(bool debug);
	};
}