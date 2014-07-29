#pragma once

#include <Engine/IMeshObject.h>
#include <Engine/Renderer/Shaders/Constants.h>

namespace fastbird
{
	class MeshObject : public IMeshObject
	{
	public:
		MeshObject();
		virtual ~MeshObject();
		
		//------------------------------------------------------------------------
		// IObject
		//------------------------------------------------------------------------
		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();
		virtual void SetMaterial(const char* name);
		virtual void SetMaterial(IMaterial* pMat);
		virtual void SetMaterialFor(int matGroupIdx, IMaterial* pMat);
		virtual void SetName(const char* name) { mName = name;}
		virtual const char* GetName() const { return mName.c_str(); }

		//------------------------------------------------------------------------
		// IMeshObject
		//------------------------------------------------------------------------
		virtual IObject* Clone() const;
		virtual bool LoadOgreMesh(const char* filename);
		virtual void ClearMeshData();

		virtual void StartModification();
		virtual void AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2);
		virtual void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4]);
		virtual void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4]);
		virtual void SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices);
		virtual void SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals);
		virtual void SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs);
		virtual void SetColors(int matGroupIdx, const DWORD* colors, size_t numColors);
		virtual void SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents);
		virtual void SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices);
		virtual void SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices);
		virtual void SetIndexBuffer(int matGroupIdx, IIndexBuffer* pIndexBuffer);
		virtual Vec3* GetPositions(int matGroupIdx, size_t& outNumPositions);
		virtual Vec3* GetNormals(int matGroupIdx, size_t& outNumNormals);
		virtual Vec2* GetUVs(int matGroupIdx, size_t& outNumUVs);
		virtual void GenerateTangent(UINT* indices, size_t num);
		virtual void EndModification(bool keepMeshData);

		virtual void SetTopology(PRIMITIVE_TOPOLOGY topology);
		virtual PRIMITIVE_TOPOLOGY GetTopology();
		virtual void SetInstanceVB(IVertexBuffer* pBuffer);

		virtual const AUXILIARIES& GetAuxiliaries() const { return mAuxCloned ? *mAuxCloned : mAuxil; }
		virtual void SetAuxiliaries(const AUXILIARIES& aux) {mAuxil = aux; }

		struct MaterialGroup
		{
			SmartPtr<IMaterial> mMaterial;
			SmartPtr<IVertexBuffer> mVBPos;
			SmartPtr<IVertexBuffer> mVBNormal;
			SmartPtr<IVertexBuffer> mVBUV;
			SmartPtr<IVertexBuffer> mVBColor;
			SmartPtr<IVertexBuffer> mVBTangent;
			
			SmartPtr<IIndexBuffer> mIndexBuffer;
			std::vector<Vec3> mPositions;
			std::vector<Vec3> mNormals;
			std::vector<Vec2> mUVs;	
			std::vector<DWORD> mColors;
			std::vector<Vec3> mTangents;
		};

	private:
		void CreateMaterialGroupFor(int matGroupIdx);

	private:
		// OBJECT
		SmartPtr<IInputLayout> mInputLayoutOverride;
		std::string mName;

		PRIMITIVE_TOPOLOGY mTopology;
		OBJECT_CONSTANTS mObjectConstants;
		// if you have only one MaterialGroup,
		// this vector will not be used.
		std::vector<MaterialGroup> mMaterialGroups; // can call it as SubMeshes.
		
		AUXILIARIES mAuxil;
		AUXILIARIES* mAuxCloned;
		bool mModifying;
	};
}