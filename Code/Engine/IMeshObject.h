#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/Renderer/RendererEnums.h>

namespace fastbird
{
	typedef std::vector< std::pair<std::string, Transformation> > AUXILIARIES;
	class IMaterial;
	class CLASS_DECLSPEC_ENGINE IMeshObject : public SpatialObject
	{
	public:
		static IMeshObject* CreateMeshObject();

		virtual ~IMeshObject(){}

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

	private:
		friend class Engine;
		virtual void Delete() = 0;
	};
}