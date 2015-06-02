#pragma once
#include <Engine/SpatialObject.h>
#include <Engine/RendererEnums.h>
#include <Engine/CollisionInfo.h>
#include <CommonLib/Math/GeomUtils.h>
#include <CommonLib/FBColShape.h>

namespace fastbird
{
	

	typedef std::vector< std::pair<std::string, Transformation> > AUXILIARIES;
	class IMaterial;
	class IMeshObject : public SpatialObject
	{
	public:
		static IMeshObject* CreateMeshObject();

		virtual ~IMeshObject(){}

		enum BUFFER_TYPE
		{
			BUFFER_TYPE_POSITION,
			BUFFER_TYPE_NORMAL,
			BUFFER_TYPE_UV,
			BUFFER_TYPE_COLOR,
			BUFFER_TYPE_TANGENT,

			BUFFER_TYPE_NUM
		};

		virtual IObject* Clone() const{assert(0); return 0;}
		virtual void SetMaterialFor(int matGroupIdx, IMaterial* pMat) = 0;
		virtual bool LoadOgreMesh(const char* filename) = 0;
		virtual void ClearMeshData() = 0;
		virtual void StartModification() = 0;
		virtual void AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2) = 0;
		virtual void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4]) = 0;
		virtual void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4]) = 0;
		virtual void SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices) = 0;
		virtual void SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals) = 0;
		virtual void SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs) = 0;
		virtual void SetTriangles(int matGroupIdx, const ModelTriangle* tris, size_t numTri) = 0;
		virtual void SetColors(int matGroupIdx, const DWORD* colors, size_t numColors) = 0;
		virtual void SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents) = 0;
		virtual void SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices) = 0;
		virtual void SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices) = 0;
		virtual void SetIndexBuffer(int matGroupIdx, IIndexBuffer* pIndexBuffer) = 0;
		virtual Vec3* GetPositions(int matGroupIdx, size_t& outNumVertices) = 0;
		virtual Vec3* GetNormals(int matGroupIdx, size_t& outNumVertices) = 0;
		virtual Vec2* GetUVs(int matGroupIdx, size_t& outNumVertices) = 0;
		virtual void GenerateTangent(int matGroupIdx, UINT* indices, size_t num) = 0;
		virtual void EndModification(bool keepMeshData) = 0;
		virtual void SetTopology(PRIMITIVE_TOPOLOGY topology) = 0;
		virtual PRIMITIVE_TOPOLOGY GetTopology() = 0;
		virtual void SetInstanceVB(IVertexBuffer* pBuffer) = 0;
		virtual const AUXILIARIES& GetAuxiliaries() const = 0;
		virtual void SetAuxiliaries(const AUXILIARIES& aux) = 0;
		virtual void AddCollisionShape(const COL_SHAPE& data) = 0;
		virtual void SetCollisionShapes(COLLISION_INFOS& colInfos) = 0;
		//set mesh at last added collision shape info for meshes in a meshgroup
		virtual void SetCollisionMesh(IMeshObject* colMesh) = 0;
		// deprecated
		virtual void SetCollisionShapes(std::vector< COL_SHAPE >& shapes) = 0;
		

		virtual void SetUseDynamicVB(BUFFER_TYPE type, bool useDynamicVB) = 0;
		virtual MapData MapVB(BUFFER_TYPE type, size_t materialGroupIdx) = 0;
		virtual void UnmapVB(BUFFER_TYPE type, size_t materialGroupIdx) = 0;

		virtual bool RayCast(const Ray3& ray, Vec3& location, const ModelTriangle** outTri = 0) const = 0;
		virtual void MakeMaterialIndependent() = 0;

		virtual void RenderSimple() = 0;

		virtual BoundingVolume* GetAABB() const = 0;

		virtual void SetAlpha(float alpha) = 0;
		virtual void SetForceAlphaBlending(bool enable, float alpha) = 0;
		virtual void SetAmbientColor(const Color& color) = 0;

		virtual bool IsPlayingAction() const = 0;

	private:
		friend class Engine;
		virtual void Delete() = 0;
	};
}