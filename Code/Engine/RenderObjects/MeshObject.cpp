
#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/MeshObject.h>
#include <Engine/IRenderer.h>
#include <Engine/GlobalEnv.h>
#include <Engine/ICamera.h>
#include <Engine/IConsole.h>
#include <Engine/Animation/Animation.h>
#include <Engine/FBCollisionShape.h>

namespace fastbird
{

	SmartPtr<IMaterial> MeshObject::mHighlightMaterial;

	//----------------------------------------------------------------------------
	IMeshObject* IMeshObject::CreateMeshObject()
	{
		return FB_NEW(MeshObject);
	}

	//----------------------------------------------------------------------------
	MeshObject::MeshObject()
		: mAuxCloned(0)
		, mRenderHighlight(false)
		, mCollisionsCloned(0)
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
		for (int i = 0; i < BUFFER_TYPE_NUM; ++i)
		{
			mUseDynamicVB[i] = false;
		}
		mAABB = BoundingVolume::Create(BoundingVolume::BV_AABB);
	}

	//----------------------------------------------------------------------------
	MeshObject::~MeshObject()
	{
		DeleteCollisionShapes();
	}

	//----------------------------------------------------------------------------
	void MeshObject::ClearHighlightMesh()
	{
		mHighlightMaterial = 0;
	}

	//----------------------------------------------------------------------------
	void MeshObject::PreRender()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;
		__super::PreRender();
		
		bool pointLightDataGathered = false;
		if (mTransformChanged)
		{
			mTransformChanged = false;

			if (mAnim)
			{
				mAnimatedTransformation = mTransformation * mAnim->GetResult();
				mAnimatedTransformation.GetHomogeneous(mObjectConstants.gWorld);
				gFBEnv->pRenderer->GatherPointLightData(mAABB, mAnimatedTransformation, &mPointLightConstants);
			}
			else
			{
				mTransformation.GetHomogeneous(mObjectConstants.gWorld);
				gFBEnv->pRenderer->GatherPointLightData(mAABB, mTransformation, &mPointLightConstants);
			}		
			pointLightDataGathered = true;
			
		}
		else
		{
			if (mAnim && mAnim->Changed())
			{
				mAnimatedTransformation = mTransformation * mAnim->GetResult();
				mAnimatedTransformation.GetHomogeneous(mObjectConstants.gWorld);
				gFBEnv->pRenderer->GatherPointLightData(mAABB, mAnimatedTransformation, &mPointLightConstants);
				pointLightDataGathered = true;
			}
		}

		if (!pointLightDataGathered && gFBEnv->pRenderer->NeedToRefreshPointLight())
		{
			if (mAnim)
			{
				gFBEnv->pRenderer->GatherPointLightData(mAABB, mAnimatedTransformation, &mPointLightConstants);
			}
			else
			{
				gFBEnv->pRenderer->GatherPointLightData(mAABB, mTransformation, &mPointLightConstants);
			}
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::Render()
	{
		if (mObjFlag & IObject::OF_HIDE)
			return;

		D3DEventMarker mark("MeshObject");
		IRenderer* pRenderer = gFBEnv->pRenderer;
		
		mObjectConstants.gWorldView = gFBEnv->pRenderer->GetCamera()->GetViewMat() * mObjectConstants.gWorld;
		mObjectConstants.gWorldViewProj = gFBEnv->pRenderer->GetCamera()->GetViewProjMat() * mObjectConstants.gWorld;


		if (!gFBEnv->pConsole->GetEngineCommand()->r_noObjectConstants)
			pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);

		pRenderer->UpdatePointLightConstantsBuffer(&mPointLightConstants);

		pRenderer->SetPrimitiveTopology(mTopology);
		BindRenderStates();

		if (gFBEnv->mRenderPass == RENDER_PASS::PASS_SHADOW && !HasObjFlag(OF_HIGHLIGHT_DEDI))
		{
			FB_FOREACH(it, mMaterialGroups)
			{
				if (!it->mMaterial || !it->mVBPos || it->mMaterial->IsNoShadowCast())
					continue;
				if (!it->mMaterial->BindSubPass(RENDER_PASS::PASS_SHADOW, true))
				{
					gFBEnv->pRenderer->SetPositionInputLayout();
					gFBEnv->pRenderer->SetShadowMapShader();
				}
				RenderMaterialGroup(&(*it), true);
			}
			return;
		}

		if (gFBEnv->mRenderPass == RENDER_PASS::PASS_DEPTH && !HasObjFlag(OF_HIGHLIGHT_DEDI))
		{
			FB_FOREACH(it, mMaterialGroups)
			{
				if (!it->mMaterial || !it->mVBPos || it->mMaterial->IsNoShadowCast())
					continue;
				bool materialReady = false;
				if (it->mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
				{
					gFBEnv->pRenderer->SetPositionInputLayout();
					materialReady = true;
				}
				else if (!(mObjFlag & OF_NO_DEPTH_PASS))
				{
					gFBEnv->pRenderer->SetPositionInputLayout();
					gFBEnv->pRenderer->SetDepthWriteShader();
					materialReady = true;
				}
				if (materialReady)
				{
					RenderMaterialGroup(&(*it), true);
				}
			}
			return;
		}
		// PASS_GODRAY_OCC_PRE
		if (gFBEnv->mRenderPass == RENDER_PASS::PASS_GODRAY_OCC_PRE && !HasObjFlag(OF_HIGHLIGHT_DEDI))
		{
			gFBEnv->pRenderer->SetPositionInputLayout();
			FB_FOREACH(it, mMaterialGroups)
			{
				if (!it->mMaterial || !it->mVBPos || it->mMaterial->IsNoShadowCast())
					continue;

				if (it->mMaterial->GetBindingShaders() & BINDING_SHADER_GS) {
					gFBEnv->pRenderer->SetOccPreGSShader();
				}
				else {
					gFBEnv->pRenderer->SetOccPreShader();
				}
				it->mMaterial->BindMaterialParams();

				RenderMaterialGroup(&(*it), true);
			}

			return;
		}

		// NORMAL PASS
		if (!HasObjFlag(OF_HIGHLIGHT_DEDI))
		{
			bool includeInputLayout = true;
			if (mInputLayoutOverride)
			{
				mInputLayoutOverride->Bind();
				includeInputLayout = false;
			}
			FB_FOREACH(it, mMaterialGroups)
			{
				if (!it->mMaterial || !it->mVBPos)
					continue;

				if (it->mMaterial->IsGlow())
					pRenderer->SetGlowRenderTarget();

				if (it->mMaterial->IsTransparent())
					pRenderer->SetAlphaBlendState();
				it->mMaterial->Bind(includeInputLayout);
				RenderMaterialGroup(&(*it), false);
				if (it->mMaterial->IsGlow())
					pRenderer->UnSetGlowRenderTarget();
			}
		}

		// HIGHLIGHT
		if (gFBEnv->mRenderPass == RENDER_PASS::PASS_NORMAL &&  mRenderHighlight)
		{
			// draw silhouett to samll buffer
			pRenderer->SetSamllSilouetteBuffer();
			{
				FB_FOREACH(it, mMaterialGroups)
				{
					if (!it->mVBPos)
						continue;
					bool materialReady = false;
					if (it->mMaterial && it->mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
					{
						gFBEnv->pRenderer->SetPositionInputLayout();
						materialReady = true;
					}
					else
					{
						gFBEnv->pRenderer->SetPositionInputLayout();
						gFBEnv->pRenderer->SetDepthWriteShader();
						materialReady = true;
					}
					if (materialReady)
					{
						RenderMaterialGroup(&(*it), true);
					}
				}
			}
			pRenderer->SetBigSilouetteBuffer();
			{
				FB_FOREACH(it, mMaterialGroups)
				{
					if (!it->mVBPos)
						continue;
					bool materialReady = false;
					if (it->mMaterial && it->mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
					{
						gFBEnv->pRenderer->SetPositionInputLayout();
						materialReady = true;
					}
					else
					{
						gFBEnv->pRenderer->SetPositionInputLayout();
						gFBEnv->pRenderer->SetDepthWriteShader();
						materialReady = true;
					}
					if (materialReady)
					{
						RenderMaterialGroup(&(*it), true);
					}
				}
			}

			gFBEnv->mSilouetteRendered = true;
			//mRenderHighlight = false;

			if (gFBEnv->pConsole->GetEngineCommand()->r_HDR)
				pRenderer->SetHDRTarget();
			else
				pRenderer->RestoreRenderTarget();
			pRenderer->RestoreViewports();


			/*if (!mHighlightMaterial)
				mHighlightMaterial = IMaterial::CreateMaterial("es/materials/MeshHighlight.material");

				if (!mHighlightRasterizeState)
				{
				RASTERIZER_DESC desc;
				desc.CullMode = CULL_MODE_FRONT;
				desc.FillMode = FILL_MODE_WIREFRAME;
				mHighlightRasterizeState = pRenderer->CreateRasterizerState(desc);
				}

				mHighlightRasterizeState->Bind();
				mHighlightMaterial->Bind(true);
				FB_FOREACH(it, mMaterialGroups)
				{
				const unsigned int numBuffers = 5;
				if (!it->mVBPos)
				continue;
				IVertexBuffer* buffers[numBuffers] = { it->mVBPos, it->mVBNormal, it->mVBUV};
				unsigned int strides[numBuffers] = { it->mVBPos->GetStride(),
				it->mVBNormal ? it->mVBNormal->GetStride() : 0,
				it->mVBUV ? it->mVBUV->GetStride() : 0,
				it->mVBColor ? it->mVBColor->GetStride() : 0,
				it->mVBTangent ? it->mVBTangent->GetStride() : 0 };
				unsigned int offsets[numBuffers] = { 0, 0, 0, 0, 0 };
				pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
				if (it->mIndexBuffer)
				{
				pRenderer->SetIndexBuffer(it->mIndexBuffer);
				pRenderer->DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
				}
				else
				{
				pRenderer->Draw(it->mVBPos->GetNumVertices(), 0);
				}
				}
				mRenderHighlight = false;
				mRasterizerState->Bind();*/
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::RenderMaterialGroup(MaterialGroup* it, bool onlyPos)
	{
		assert(it);
		if (!it || !it->mMaterial || !it->mVBPos)
			return;

		IRenderer* pRenderer = gFBEnv->pRenderer;
		if (onlyPos)
		{
			const unsigned int numBuffers = 1;

			IVertexBuffer* buffers[numBuffers] = { it->mVBPos };
			unsigned int strides[numBuffers] = { it->mVBPos->GetStride() };
			unsigned int offsets[numBuffers] = { 0 };
			pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
			if (it->mIndexBuffer)
			{
				pRenderer->SetIndexBuffer(it->mIndexBuffer);
				pRenderer->DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
			}
			else
			{
				pRenderer->Draw(it->mVBPos->GetNumVertices(), 0);
			}
		}
		else
		{
			const unsigned int numBuffers = 5;

			IVertexBuffer* buffers[numBuffers] = { it->mVBPos, it->mVBNormal, it->mVBUV, it->mVBColor, it->mVBTangent };
			unsigned int strides[numBuffers] = { it->mVBPos->GetStride(),
				it->mVBNormal ? it->mVBNormal->GetStride() : 0,
				it->mVBUV ? it->mVBUV->GetStride() : 0,
				it->mVBColor ? it->mVBColor->GetStride() : 0,
				it->mVBTangent ? it->mVBTangent->GetStride() : 0 };
			unsigned int offsets[numBuffers] = { 0, 0, 0, 0, 0 };
			pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
			if (it->mIndexBuffer)
			{
				pRenderer->SetIndexBuffer(it->mIndexBuffer);
				pRenderer->DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
			}
			else
			{
				pRenderer->Draw(it->mVBPos->GetNumVertices(), 0);
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
	void MeshObject::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
	{
		CreateMaterialGroupFor(0);
		IMaterial* pMat = fastbird::IMaterial::CreateMaterial(name);
		if (!pMat)
			Log("Failed to load a material %s", name);
		mMaterialGroups[0].mMaterial = pMat;
		if (pMat && pMat->IsTransparent())
		{
			BLEND_DESC bdesc;
			bdesc.RenderTarget[0].BlendEnable = true;
			bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;

			SetBlendState(bdesc);
		}
	}

	void MeshObject::SetMaterial(IMaterial* pMat, int pass /*= RENDER_PASS::PASS_NORMAL*/)
	{
		CreateMaterialGroupFor(0);
		mMaterialGroups[0].mMaterial = pMat;
		if (pMat->IsTransparent())
		{
			BLEND_DESC bdesc;
			bdesc.RenderTarget[0].BlendEnable = true;
			bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;

			SetBlendState(bdesc);
		}
	}


	IMaterial* MeshObject::GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
	{
		if (!mMaterialGroups.empty())
			return mMaterialGroups[0].mMaterial;

		return 0;
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetMaterialFor(int matGroupIdx, IMaterial* pMat)
	{
		assert(matGroupIdx < (int)mMaterialGroups.size());
		mMaterialGroups[matGroupIdx].mMaterial = pMat;
	}

	//----------------------------------------------------------------------------
	void MeshObject::RenderSimple()
	{
		auto pRenderer = gFBEnv->pRenderer;
		FB_FOREACH(it, mMaterialGroups)
		{
			if (!it->mVBPos)
				continue;
			RenderMaterialGroup(&(*it), true);
		}
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
			mg.mPositions = it->mPositions;
		}
		cloned->mAuxCloned = mAuxCloned ? mAuxCloned : (AUXILIARIES*)&mAuxil;
		cloned->mCollisionsCloned = mCollisionsCloned ? mCollisionsCloned : (COLLISION_SHAPES*)&mCollisions;
		cloned->mAABB = mAABB;
		return cloned;
	}

	//----------------------------------------------------------------------------
	void MeshObject::Delete()
	{
		FB_DELETE(this);
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
		mMaterialGroups[matGroupIdx].mPositions.assign(p, p + numVertices);
	}


	void MeshObject::SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mNormals.assign(n, n + numNormals);
	}

	void MeshObject::SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mUVs.assign(uvs, uvs + numUVs);
	}

	void MeshObject::SetTriangles(int matGroupIdx, const ModelTriangle* tris, size_t numTri)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mTriangles.assign(tris, tris + numTri);
	}

	void MeshObject::SetColors(int matGroupIdx, const DWORD* colors, size_t numColors)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mColors.assign(colors, colors + numColors);
	}

	void MeshObject::SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents)
	{
		CreateMaterialGroupFor(matGroupIdx);
		mMaterialGroups[matGroupIdx].mTangents.assign(t, t + numTangents);
	}

	//----------------------------------------------------------------------------
	void MeshObject::SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices)
	{
		if (!gFBEnv)
			return;

		CreateMaterialGroupFor(matGroupIdx);
		if (numIndices <= std::numeric_limits<USHORT>::max())
		{
			std::vector<USHORT> sIndices(indices, indices + numIndices);
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
	void MeshObject::GenerateTangent(int matGroupIdx, UINT* indices, size_t num)
	{
		assert(mModifying);
		assert(matGroupIdx < (int)mMaterialGroups.size());
		MaterialGroup& group = mMaterialGroups[matGroupIdx];
		if (group.mUVs.empty())
			return;
		group.mTangents.assign(group.mPositions.size(), Vec3(1, 0, 0));
		if (num)
		{
			for (size_t i = 0; i < num; i += 3)
			{
				Vec3 p1 = group.mPositions[indices[i]];
				Vec3 p2 = group.mPositions[indices[i + 1]];
				Vec3 p3 = group.mPositions[indices[i + 2]];
				Vec2 uv1 = group.mUVs[indices[i]];
				Vec2 uv2 = group.mUVs[indices[i + 1]];
				Vec2 uv3 = group.mUVs[indices[i + 2]];
				Vec3 tan = CalculateTangentSpaceVector(p1, p2, p3,
					uv1, uv2, uv3);
				group.mTangents[indices[i]] = tan;
				group.mTangents[indices[i + 1]] = tan;
				group.mTangents[indices[i + 2]] = tan;
			}
		}
		else
		{
			size_t nump = group.mPositions.size();
			for (size_t i = 0; i < nump; i += 3)
			{
				Vec3 p1 = group.mPositions[i];
				Vec3 p2 = group.mPositions[i + 1];
				Vec3 p3 = group.mPositions[i + 2];
				Vec2 uv1 = group.mUVs[i];
				Vec2 uv2 = group.mUVs[i + 1];
				Vec2 uv3 = group.mUVs[i + 2];
				Vec3 tan = CalculateTangentSpaceVector(p1, p2, p3,
					uv1, uv2, uv3);
				group.mTangents[i] = tan;
				group.mTangents[i + 1] = tan;
				group.mTangents[i + 2] = tan;
			}
		}
	}

	//----------------------------------------------------------------------------
	void MeshObject::EndModification(bool keepMeshData)
	{
		mModifying = false;
		mAABB->StartComputeFromData();
		FB_FOREACH(it, mMaterialGroups)
		{
			if (!it->mPositions.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				it->mVBPos = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mPositions[0], sizeof(Vec3), it->mPositions.size(),
					mUseDynamicVB[BUFFER_TYPE_POSITION] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[BUFFER_TYPE_POSITION] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
				mAABB->AddComputeData(&it->mPositions[0], it->mPositions.size());
			}
			else
			{
				it->mVBPos = 0;
			}
			if (!it->mNormals.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mNormals.size());
				it->mVBNormal = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mNormals[0], sizeof(Vec3), it->mNormals.size(),
					mUseDynamicVB[BUFFER_TYPE_NORMAL] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[BUFFER_TYPE_NORMAL] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBNormal = 0;
			}
			if (!it->mUVs.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mUVs.size());
				it->mVBUV = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mUVs[0], sizeof(Vec2), it->mUVs.size(),
					mUseDynamicVB[BUFFER_TYPE_UV] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[BUFFER_TYPE_UV] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBUV = 0;
			}

			if (!it->mColors.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mColors.size());
				it->mVBColor = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mColors[0], sizeof(DWORD), it->mColors.size(),
					mUseDynamicVB[BUFFER_TYPE_COLOR] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[BUFFER_TYPE_COLOR] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBColor = 0;
			}

			if (!it->mTangents.empty() && gFBEnv && gFBEnv->pRenderer)
			{
				assert(it->mPositions.size() == it->mTangents.size());
				it->mVBTangent = gFBEnv->pRenderer->CreateVertexBuffer(
					&it->mTangents[0], sizeof(Vec3), it->mTangents.size(),
					mUseDynamicVB[BUFFER_TYPE_TANGENT] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[BUFFER_TYPE_TANGENT] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it->mVBTangent = 0;
			}


		}
		mAABB->EndComputeFromData();
		mBoundingVolume->SetCenter(mAABB->GetCenter());
		mBoundingVolume->SetRadius(mAABB->GetRadius());
		mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter() + mTransformation.GetTranslation());
		const Vec3& s = mTransformation.GetScale();
		mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius() * std::max(std::max(s.x, s.y), s.z));

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

	void MeshObject::SetEnableHighlight(bool enable)
	{
		mRenderHighlight = enable;
	}

	//----------------------------------------------------------------------------
	void MeshObject::AddCollisionShape(const COL_SHAPE& data)
	{
		FBCollisionShape* cs = FB_NEW(FBCollisionShape)(data.first, data.second, 0);
		mCollisions.push_back(cs);
	}

	void MeshObject::SetCollisionShapes(COLLISION_INFOS& colInfos)
	{
		DeleteCollisionShapes();
		for (const auto& var : colInfos)
		{
			mCollisions.push_back(FB_NEW(FBCollisionShape(var.mColShapeType, var.mTransform, var.mCollisionMesh)));
		}
	}

	// deprecated
	void MeshObject::SetCollisionShapes(std::vector< std::pair<FBColShape::Enum, Transformation> >& shapes)
	{
		DeleteCollisionShapes();
		for (const auto& var : shapes)
		{
			AddCollisionShape(var);
		}
	}

	void MeshObject::SetCollisionMesh(IMeshObject* colMesh)
	{
		assert(!mCollisions.empty());
		mCollisions.back()->SetCollisionMesh(colMesh);
	}

	void MeshObject::DeleteCollisionShapes()
	{
		for (auto cs : mCollisions)
		{
			FB_DELETE(cs);
		}
		mCollisions.clear();
	}

	void MeshObject::SetUseDynamicVB(BUFFER_TYPE type, bool useDynamicVB)
	{
		mUseDynamicVB[type] = useDynamicVB;
	}

	MapData MeshObject::MapVB(BUFFER_TYPE type, size_t materialGroupIdx)
	{
		if (!mUseDynamicVB[type])
		{
			assert(0);
			return MapData();
		}
		if (materialGroupIdx >= mMaterialGroups.size())
		{
			assert(0);
			return MapData();
		}
			
		MaterialGroup& mg = mMaterialGroups[materialGroupIdx];
		switch (type)
		{
		case BUFFER_TYPE_POSITION:
			return mg.mVBPos->Map(MAP_TYPE_WRITE, 0, MAP_FLAG_NONE);
		case BUFFER_TYPE_NORMAL:
			return mg.mVBNormal->Map(MAP_TYPE_WRITE_NO_OVERWRITE, 0, MAP_FLAG_NONE);
		case BUFFER_TYPE_UV:
			return mg.mVBUV->Map(MAP_TYPE_WRITE, 0, MAP_FLAG_NONE);
		case BUFFER_TYPE_COLOR:
			return mg.mVBColor->Map(MAP_TYPE_WRITE, 0, MAP_FLAG_NONE);
		case BUFFER_TYPE_TANGENT:
			return mg.mVBTangent->Map(MAP_TYPE_WRITE, 0, MAP_FLAG_NONE);
		}
		assert(0);

		return MapData();
	}

	void MeshObject::UnmapVB(BUFFER_TYPE type, size_t materialGroupIdx)
	{
		if (!mUseDynamicVB[type])
		{
			assert(0);
			return;
		}
		if (materialGroupIdx >= mMaterialGroups.size())
		{
			assert(0);
			return;
		}
		MaterialGroup& mg = mMaterialGroups[materialGroupIdx];
		switch (type)
		{
		case BUFFER_TYPE_POSITION:
			return mg.mVBPos->Unmap();
		case BUFFER_TYPE_NORMAL:
			return mg.mVBNormal->Unmap();
		case BUFFER_TYPE_UV:
			return mg.mVBUV->Unmap();
		case BUFFER_TYPE_COLOR:
			return mg.mVBColor->Unmap();
		case BUFFER_TYPE_TANGENT:
			return mg.mVBTangent->Unmap();
		}
		assert(0);
	}

	bool MeshObject::RayCast(const Ray3& ray, Vec3& location, const ModelTriangle** outTri) const
	{
		auto mesh = (MeshObject*)gFBEnv->pEngine->GetMeshArchetype(mName);
		assert(mesh);
		const float maxRayDistance = 100000;
		float tMin = maxRayDistance;
		const float epsilon = 0.001f;
		ModelIntersection rayTriIntersection;
		rayTriIntersection.valid = false;

		for (const auto& mg : mesh->mMaterialGroups)
		{
			for (const auto& tri : mg.mTriangles)
			{
				float NdotD = tri.faceNormal.Dot(ray.GetDir());
				if (abs(NdotD) < epsilon)
				{
					// ray is parallel or nearly parallel to polygon plane
					continue;
				}

				float t = (tri.d - tri.faceNormal.Dot(ray.GetOrigin())) / NdotD;
				if (t <= 0)
				{
					// ray origin is behind the triangle;
					continue;
				}
				if (t >= tMin)
				{
					continue;
				}
				Vec3 intersectionPoint = ray.GetOrigin() + ray.GetDir() * t;
				// find the interpolation parameters alpha and beta using 2D projections
				float alpha;
				float beta;
				Vec2 P;    // projection of the intersection onto an axis aligned plane
				switch (tri.dominantAxis)
				{
				case 0:
					P.x = intersectionPoint.y;
					P.y = intersectionPoint.z;
					break;
				case 1:
					P.x = intersectionPoint.x;
					P.y = intersectionPoint.z;
					break;
				case 2:
				default:
					P.x = intersectionPoint.x;
					P.y = intersectionPoint.y;
				}
				float u0, u1, u2, v0, v1, v2;
				u0 = P.x - tri.v0Proj.x;
				v0 = P.y - tri.v0Proj.y;
				u1 = tri.v1Proj.x - tri.v0Proj.x;
				u2 = tri.v2Proj.x - tri.v0Proj.x;
				v1 = tri.v1Proj.y - tri.v0Proj.y;
				v2 = tri.v2Proj.y - tri.v0Proj.y;
				if (abs(u1) < epsilon)
				{
					beta = u0 / u2;
					if ((beta >= 0) && (beta <= 1))
					{
						alpha = (v0 - beta * v2) / v1;
						if ((alpha >= 0) && ((alpha + beta) <= 1))
						{
							rayTriIntersection.valid = true;
							rayTriIntersection.alpha = alpha;
							rayTriIntersection.beta = beta;
							rayTriIntersection.pTri = &tri;
							tMin = t;
						}
					}
				}
				else
				{
					beta = (v0*u1 - u0*v1) / (v2*u1 - u2*v1);
					if ((beta >= 0) && (beta <= 1))
					{
						alpha = (u0 - beta * u2) / u1;
						if ((alpha >= 0) && ((alpha + beta) <= 1))
						{
							rayTriIntersection.valid = true;
							rayTriIntersection.alpha = alpha;
							rayTriIntersection.beta = beta;
							rayTriIntersection.pTri = &tri;
							tMin = t;
						}
					}
				}
			}
			if (!rayTriIntersection.valid)
			{
				if (outTri)
					*outTri = NULL;
				return false;
			}
			else
			{
				// compute the location using barycentric coordinates
				const ModelTriangle* pTri = rayTriIntersection.pTri;
				float alpha = rayTriIntersection.alpha;
				float beta = rayTriIntersection.beta;
				location.x = ((1 - (alpha + beta)) * mg.mPositions[pTri->v[0]].x) + 
					alpha * mg.mPositions[pTri->v[2]].x +
					beta * mg.mPositions[pTri->v[2]].x;
				location.y = ((1 - (alpha + beta)) * mg.mPositions[pTri->v[0]].y) +
					alpha * mg.mPositions[pTri->v[1]].y + 
					beta * mg.mPositions[pTri->v[2]].y;
				location.z = ((1 - (alpha + beta)) * mg.mPositions[pTri->v[0]].z) + 
					alpha * mg.mPositions[pTri->v[1]].z +
					beta * mg.mPositions[pTri->v[2]].z;
				if (outTri)
				{
					*outTri = pTri;
				}
				return true;
			}
		}
		return false;
	}

	void MeshObject::MakeMaterialIndependent()
	{
		for (auto& mg : mMaterialGroups)
		{
			mg.mMaterial = mg.mMaterial->Clone();
		}
	}

	bool MeshObject::CheckNarrowCollision(fastbird::BoundingVolume* pBV) const
	{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return true;

		for (unsigned i = 0; i < num; ++i)
		{
			const FBCollisionShape* cs = GetCollisionShape(i);
			bool collide = cs->TestCollision(pBV, mTransformation);
			if (collide)
			{
				return true;
			}
		}

		return false;
	}

	Ray3::IResult MeshObject::CheckNarrowCollisionRay(const Ray3& ray) const
	{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return Ray3::IResult(false, 0.f);

		float minDist = FLT_MAX;
		bool collided = false;
		for (unsigned i = 0; i < num; ++i)
		{
			const FBCollisionShape* cs = GetCollisionShape(i);
			auto result = cs->intersects(ray, mTransformation);
			if (result.first)
			{
				collided = true;
				if (minDist > result.second)
					minDist = result.second;
			}
		}

		return Ray3::IResult(collided, minDist);
	}

	Vec3 MeshObject::GetRandomPosInVolume(const Vec3* nearWorld) const
	{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return Vec3::ZERO;

		auto index = Random((unsigned)0, num-1);
		assert(index < num);
		auto cs = GetCollisionShape(index);
		assert(cs);
		return cs->GetRandomPosInVolume(nearWorld, mTransformation);
	}
}