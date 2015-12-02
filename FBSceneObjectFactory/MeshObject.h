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
#include "FBCollisionShape.h"
#include "CollisionInfo.h"
#include "MeshAuxiliary.h"
#include "MeshVertexBufferType.h"
#include "FBSceneManager/SpatialSceneObject.h"
#include "FBRenderer/RenderParam.h"
#include "FBRenderer/RendererEnums.h"
#include "FBRenderer/RendererStructs.h"
namespace fb{
	struct ModelTriangle;
	FB_DECLARE_SMART_PTR(IndexBuffer);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(MeshObject);
	class FB_DLL_SCENEOBJECTFACTORY MeshObject : public SpatialSceneObject{
		FB_DECLARE_PIMPL_CLONEABLE(MeshObject);		
		static MeshObjectPtr Create(const MeshObject& other);
	protected:
		MeshObject();
		~MeshObject();

	public:

		/** Creates new instance.
		Usullay when you want to load a .dae file, you use SceneManager::CreateMesh(filepath) function instead
		calling this function directly. */
		static MeshObjectPtr Create();		

		//---------------------------------------------------------------------------
		// SceneObject Interfaces
		//---------------------------------------------------------------------------				
		void PreRender(const RenderParam& renderParam, RenderParamOut* renderParamOut);
		void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut);
		void PostRender(const RenderParam& renderParam, RenderParamOut* renderParamOut);

		//---------------------------------------------------------------------------
		// Own functions
		//---------------------------------------------------------------------------		

		MeshObjectPtr Clone() const;
		void SetMaterial(const char* filepath);
		void SetMaterial(const char* filepath, int pass);
		void SetMaterial(MaterialPtr pMat, int pass);
		MaterialPtr GetMaterial(int pass = 0) const;
		void SetEnableHighlight(bool enable);
		void RenderSimple();
		void ClearMeshData();
		void StartModification();
		void AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2);
		void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4]);
		void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4]);
		void SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices);
		void SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals);
		void SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs);
		void SetTriangles(int matGroupIdx, const ModelTriangle* tris, size_t numTri);
		void SetColors(int matGroupIdx, const DWORD* colors, size_t numColors);
		void SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents);
		void SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices);
		void SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices);
		void SetIndexBuffer(int matGroupIdx, IndexBufferPtr pIndexBuffer);
		Vec3* GetPositions(int matGroupIdx, size_t& outNumPositions);
		Vec3* GetNormals(int matGroupIdx, size_t& outNumNormals);
		Vec2* GetUVs(int matGroupIdx, size_t& outNumUVs);
		void GenerateTangent(int matGroupIdx, UINT* indices, size_t num);
		void EndModification(bool keepMeshData);
		void SetMaterialFor(int matGroupIdx, MaterialPtr material);
		void SetTopology(PRIMITIVE_TOPOLOGY topology);
		PRIMITIVE_TOPOLOGY GetTopology();		

		/// Returns auxiliaries data
		/// You do not own the returned pointer.
		const AUXILIARIES* GetAuxiliaries() const;
		void AddAuxiliary(const AUXILIARY& aux);
		void SetAuxiliaries(const AUXILIARIES& auxes);
		void AddCollisionShape(const COL_SHAPE& data);
		void SetCollisionShapes(const COLLISION_INFOS& colInfos);
		void SetCollisionMesh(MeshObjectPtr colMesh);
		unsigned GetNumCollisionShapes() const;
		bool HasCollisionShapes() const;
		FBCollisionShapeConstPtr GetCollisionShape(unsigned idx) const;
		bool CheckNarrowCollision(const BoundingVolume* pBV) const;
		Ray3::IResult CheckNarrowCollisionRay(const Ray3& ray) const;
		Vec3 GetRandomPosInVolume(const Vec3* nearWorld = 0) const;
		void DeleteCollisionShapes();
		void SetUseDynamicVB(MeshVertexBufferType::Enum type, bool useDynamicVB);
		MapData MapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx);
		void UnmapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx);
		bool RayCast(const Ray3& ray, Vec3& location, const ModelTriangle** outTri = 0) const;
		BoundingVolumeConstPtr GetAABB() const;
		void ClearVertexBuffers();
		void SetAlpha(Real alpha);
		void SetForceAlphaBlending(bool enable, Real alpha, Real forceGlow = 0.f, bool disableDepth = false);
		void SetAmbientColor(const Color& color);		

	};
}