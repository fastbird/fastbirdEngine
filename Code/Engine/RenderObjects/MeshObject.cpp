#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/MeshObject.h>
#include <Engine/Renderer/D3DEventMarker.h>
#include <Engine/IRenderer.h>
#include <Engine/GlobalEnv.h>
#include <Engine/ICamera.h>
#include <Engine/IConsole.h>

namespace fastbird
{
	//----------------------------------------------------------------------------
	IMeshObject* IMeshObject::CreateMeshObject()
	{
		return new MeshObject;
	}

	//----------------------------------------------------------------------------
	MeshObject::MeshObject()
		: mAuxCloned(0)
	{
		mTopology = PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		if (gFBEnv && gFBEnv->pRenderer && gFBEnv->pRenderer->GetCamera())
			mObjectConstants.gWorldViewProj = gFBEnv->pRenderer->GetCamera()->GetViewProjMat();
		else
			mObjectConstants.gWorldViewProj.MakeIdentity();

		mObjectConstants.gWorld.MakeIdentity();
		if (gFBEnv && gFBEnv->pRenderer)
		{
			SetDepthStencilState(DEPTH_STENCIL_DESC());
			SetRasterizerState(RASTERIZER_DESC());
			SetMaterial(gFBEnv->pRenderer->GetMissingMaterial());
		}
		
	}

	//----------------------------------------------------------------------------
	MeshObject::~MeshObject()
	{
	}

	//----------------------------------------------------------------------------
	void MeshObject::PreRender()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;
		mTransformation.GetHomogeneous(mObjectConstants.gWorld);
		mObjectConstants.gWorldViewProj = gFBEnv->pRenderer->GetCamera()->GetViewProjMat() * mObjectConstants.gWorld;
	}

	//----------------------------------------------------------------------------
	void MeshObject::Render()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;

		D3DEventMarker mark("MeshObject");

		if (!gFBEnv->pConsole->GetEngineCommand()->r_noObjectConstants)
			gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);

		gFBEnv->pRenderer->SetPrimitiveTopology(mTopology);
		BindRenderStates();

		bool includeInputLayout = true;
		if (mInputLayoutOverride)
		{
			mInputLayoutOverride->Bind();
			includeInputLayout = false;
		}

		FB_FOREACH(it, mMaterialGroups)
		{		
			if (!it->mMaterial)
				continue;
			it->mMaterial->Bind(includeInputLayout);
			const unsigned int numBuffers = 5;
			if (!it->mVBPos)
				continue;
			IVertexBuffer* buffers[numBuffers] = {it->mVBPos, it->mVBNormal, it->mVBUV, it->mVBColor, it->mVBTangent};
			unsigned int strides[numBuffers] = {it->mVBPos->GetStride(), 
				it->mVBNormal ? it->mVBNormal->GetStride() : 0,
				it->mVBUV ? it->mVBUV->GetStride() :0,
				it->mVBColor ? it->mVBColor->GetStride() : 0,
				it->mVBTangent ? it->mVBTangent->GetStride() : 0};
			unsigned int offsets[numBuffers] = {0, 0, 0, 0, 0};
			gFBEnv->pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
			if (it->mIndexBuffer)
			{
				gFBEnv->pRenderer->SetIndexBuffer(it->mIndexBuffer);
				gFBEnv->pRenderer->DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
			}
			else
			{
				gFBEnv->pRenderer->Draw(it->mVBPos->GetNumVertices(), 0);			
			}
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::PostRender()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetMaterial(const char* name)
	{
		CreateMaterialGroupFor(0);
		IMaterial* pMat = fastbird::IMaterial::CreateMaterial(name);
		if (!pMat)
			Log("Failed to load a material %s", name);
		mMaterialGroups[0].mMaterial = pMat;
	}

	void MeshObject::SetMaterial(IMaterial* pMat)
	{
		CreateMaterialGroupFor(0);
		mMaterialGroups[0].mMaterial = pMat;
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetMaterialFor(int matGroupIdx, IMaterial* pMat)
	{
		assert(matGroupIdx < (int)mMaterialGroups.size());
		mMaterialGroups[matGroupIdx].mMaterial = pMat;
	}

	//----------------------------------------------------------------------------
	IObject* MeshObject::Clone() const
	{
		MeshObject* cloned = (MeshObject*)IMeshObject::CreateMeshObject();
		SpatialObject::Clone(cloned);
		
		cloned->mInputLayoutOverride = mInputLayoutOverride;
		cloned->mName = mName;
		cloned->mTopology = mTopology;
		cloned->mObjectConstants = mObjectConstants;		
		FB_FOREACH(it, mMaterialGroups)
		{
			size_t idx = std::distance(mMaterialGroups.begin(), it);
			if (idx >= cloned->mMaterialGroups.size())
			{
				cloned->mMaterialGroups.push_back(MaterialGroup());
			}
			MaterialGroup& mg = cloned->mMaterialGroups.back();			
			mg.mMaterial = it->mMaterial;
			mg.mVBPos = it->mVBPos;
			mg.mVBNormal = it->mVBNormal;
			mg.mVBUV = it->mVBUV;
			mg.mVBColor = it->mVBColor;
			mg.mVBTangent = it->mVBTangent;
			mg.mIndexBuffer = it->mIndexBuffer;
		}
		cloned->mAuxCloned = mAuxCloned ? mAuxCloned : (AUXILIARIES*)&mAuxil;
		return cloned;
	}

	//----------------------------------------------------------------------------
	bool MeshObject::LoadOgreMesh(const char* filename)
	{
		return true;
	}

	//----------------------------------------------------------------------------
	void MeshObject::ClearMeshData()
	{
		FB_FOREACH(it, mMaterialGroups)
		{
			it->mPositions.clear();
			it->mNormals.clear();
			it->mUVs.clear();

			it->mPositions.resize(0);
			it->mNormals.resize(0);
			it->mUVs.resize(0);
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::CreateMaterialGroupFor(int matGroupIdx)
	{
		if ((int)mMaterialGroups.size() <= matGroupIdx)
		{
			mMaterialGroups.push_back(MaterialGroup());
			if (gFBEnv && gFBEnv->pRenderer)
				mMaterialGroups.back().mMaterial = gFBEnv->pRenderer->GetMissingMaterial();
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::StartModification()
	{		
		mModifying = true;
	}

	//----------------------------------------------------------------------------
	void MeshObject::AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos0);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos1);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos2);
	}

	//----------------------------------------------------------------------------
	void MeshObject::AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4])
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[0]);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[1]);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[2]);

		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[2]);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[1]);
		mMaterialGroups[matGroupIdx].mPositions.push_back(pos[3]);

		
		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[0]);
		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[1]);
		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[2]);

		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[2]);
		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[1]);
		mMaterialGroups[matGroupIdx].mNormals.push_back(normal[3]);
	}

	void MeshObject::AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4])
	{
		AddQuad(matGroupIdx, pos, normal);
		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[0]);
		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[1]);
		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[2]);

		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[2]);
		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[1]);
		mMaterialGroups[matGroupIdx].mUVs.push_back(uv[3]);
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mPositions.assign(p, p+numVertices);
	}

	
	void MeshObject::SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mNormals.assign(n, n+numNormals);
	}

	void MeshObject::SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mUVs.assign(uvs, uvs+numUVs);
	}

	void MeshObject::SetColors(int matGroupIdx, const DWORD* colors, size_t numColors)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mColors.assign(colors, colors+numColors);
	}

	void MeshObject::SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mTangents.assign(t, t+numTangents);
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices)
	{
		if (!gFBEnv)
			return;

		CreateMaterialGroupFor(matGroupIdx);
		if (numIndices <= std::numeric_limits<USHORT>::max())
		{
			std::vector<USHORT> sIndices(indices, indices+numIndices);
			mMaterialGroups[matGroupIdx].mIndexBuffer = 
				gFBEnv->pRenderer->CreateIndexBuffer(&sIndices[0], numIndices, INDEXBUFFER_FORMAT_16BIT);
		}
		else
		{
			mMaterialGroups[matGroupIdx].mIndexBuffer = 
				gFBEnv->pRenderer->CreateIndexBuffer((void*)indices, numIndices, INDEXBUFFER_FORMAT_32BIT);
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mIndexBuffer = gFBEnv->pRenderer->CreateIndexBuffer((void*)indices, numIndices, INDEXBUFFER_FORMAT_16BIT);
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetIndexBuffer(int matGroupIdx, IIndexBuffer* pIndexBuffer)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mIndexBuffer = pIndexBuffer;
	}

	//----------------------------------------------------------------------------
	Vec3* MeshObject::GetPositions(int matGroupIdx, size_t& outNumPositions)
	{
		CreateMaterialGroupFor(matGroupIdx);
		outNumPositions = mMaterialGroups[matGroupIdx].mPositions.size();
		if (outNumPositions)
			return &(mMaterialGroups[matGroupIdx].mPositions[0]);
		else
			return 0;
	}

	//----------------------------------------------------------------------------
	Vec3* MeshObject::GetNormals(int matGroupIdx, size_t& outNumNormals)
	{
		CreateMaterialGroupFor(matGroupIdx);
		outNumNormals = mMaterialGroups[matGroupIdx].mNormals.size();
		if (outNumNormals)
			return &(mMaterialGroups[matGroupIdx].mNormals[0]);
		else
			return 0;
	}

	//----------------------------------------------------------------------------
	Vec2* MeshObject::GetUVs(int matGroupIdx, size_t& outNumUVs)
	{
		CreateMaterialGroupFor(matGroupIdx);
		outNumUVs = mMaterialGroups[matGroupIdx].mUVs.size();
		if (outNumUVs)
			return &(mMaterialGroups[matGroupIdx].mUVs[0]);
		else
			return 0;
	}

	//----------------------------------------------------------------------------
	void MeshObject::GenerateTangent(UINT* indices, size_t num)
	{
		assert(mModifying);

		FB_FOREACH(it, mMaterialGroups)
		{
			if (it->mUVs.empty())
				continue;
			it->mTangents.assign(it->mPositions.size(), Vec3(1, 0, 0));
			if (num)
			{
				for (size_t i=0; i<num; i+=3)
				{
					Vec3 p1 = it->mPositions[indices[i]];
					Vec3 p2 = it->mPositions[indices[i+1]];
					Vec3 p3 = it->mPositions[indices[i+2]];
					Vec2 uv1 = it->mUVs[indices[i]];
					Vec2 uv2 = it->mUVs[indices[i+1]];
					Vec2 uv3 = it->mUVs[indices[i+2]];
					Vec3 tan = CalculateTangentSpaceVector(p1, p2, p3,
						uv1, uv2, uv3);
					it->mTangents[indices[i]] = tan;
					it->mTangents[indices[i+1]] = tan;
					it->mTangents[indices[i+2]] = tan;
				}
			}
			else
			{
				size_t nump = it->mPositions.size();
				for (size_t i=0; i<nump; i+=3)
				{
					Vec3 p1 = it->mPositions[i];
					Vec3 p2 = it->mPositions[i+1];
					Vec3 p3 = it->mPositions[i+2];
					Vec2 uv1 = it->mUVs[i];
					Vec2 uv2 = it->mUVs[i+1];
					Vec2 uv3 = it->mUVs[i+2];
					Vec3 tan = CalculateTangentSpaceVector(p1, p2, p3,
						uv1, uv2, uv3);
					it->mTangents[i] = tan;
					it->mTangents[i+1] = tan;
					it->mTangents[i+2] = tan;
				}				
			}
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::EndModification(bool keepMeshData)
	{
		mModifying = false;
		mBoundingVolume->StartComputeFromData();
		FB_FOREACH(it , mMaterialGroups)
		{
			if (!it->mPositions.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				it->mVBPos = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mPositions[0], sizeof(Vec3), it->mPositions.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
				mBoundingVolume->AddComputeData(&it->mPositions[0], it->mPositions.size());
			}
			else
			{
				it->mVBPos = 0;
			}
			if (!it->mNormals.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mNormals.size());
				it->mVBNormal = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mNormals[0], sizeof(Vec3), it->mNormals.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBNormal = 0;
			}
			if (!it->mUVs.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mUVs.size());
				it->mVBUV = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mUVs[0], sizeof(Vec2), it->mUVs.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBUV = 0;
			}

			if (!it->mColors.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mColors.size());
				it->mVBColor = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mColors[0], sizeof(DWORD), it->mColors.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBColor = 0;
			}

			if (!it->mTangents.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mTangents.size());
				it->mVBTangent = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mTangents[0], sizeof(Vec3), it->mTangents.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBTangent = 0;
			}
		}
		mBoundingVolume->EndComputeFromData();
		mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() +  mTransformation.GetTranslation());
		mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius());

		if (!keepMeshData)
			ClearMeshData();
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetTopology(PRIMITIVE_TOPOLOGY topology)
	{
		mTopology = topology;
	}

	//----------------------------------------------------------------------------
	PRIMITIVE_TOPOLOGY MeshObject::GetTopology()
	{
		return mTopology;
	}

	void MeshObject::SetInstanceVB(IVertexBuffer* pBuffer)
	{
	}
}