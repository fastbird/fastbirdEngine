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
#include "MeshObject.h"
#include "SceneObjectFactory.h"
#include "FBCommonHeaders/CowPtr.h"
#include "FBStringMathLib/StringMathConverter.h"
#include "FBAnimation/Animation.h"
#include "FBAnimation/AnimationData.h"
#include "FBDebugLib/Logger.h"
#include "FBSceneManager/SceneManager.h"
#include "FBSceneManager/IScene.h"
#include "FBRenderer/Camera.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/RendererOptions.h"
#include "FBRenderer/RenderStates.h"
#include "FBRenderer/VertexBuffer.h"
#include "FBRenderer/IndexBuffer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/ResourceProvider.h"
#include "FBStringLib/StringLib.h"
#include "FBMathLib/GeomUtils.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

class MeshObject::Impl{
public:
	struct MaterialGroup
	{
		MaterialPtr mMaterial;
		MaterialPtr mForceAlphaMaterial;
		VertexBufferPtr mVBPos;
		VertexBufferPtr mVBNormal;
		VertexBufferPtr mVBUV;
		VertexBufferPtr mVBColor;
		VertexBufferPtr mVBTangent;
		IndexBufferPtr mIndexBuffer;
		std::vector<Vec3> mPositions;
		std::vector<Vec3> mNormals;
		std::vector<Vec2> mUVs;
		std::vector<ModelTriangle> mTriangles;
		std::vector<DWORD> mColors;
		std::vector<Vec3> mTangents;
	};

	struct ModelIntersection {
		const ModelTriangle        *pTri;      // Pointer to mesh triangle that was intersected by ray
		Vec3     position;   // Intersection point on the triangle
		Real           alpha;      // Alpha and beta are two of the barycentric coordinates of the intersection 
		Real           beta;       // ... the third barycentric coordinate can be calculated by: 1- (alpha + beta).
		bool            valid;      // "valid" is set to true if an intersection was found
	};

	MeshObject* mSelf;
	InputLayoutPtr mInputLayoutOverride;
	PRIMITIVE_TOPOLOGY mTopology;
	OBJECT_CONSTANTS mObjectConstants;
	POINT_LIGHT_CONSTANTS mPointLightConstants;
	// if you have only one MaterialGroup,
	// this vector will not be used.
	std::vector<MaterialGroup> mMaterialGroups; // can call it as SubMeshes.

	CowPtr<AUXILIARIES> mAuxiliary;	
	bool mModifying;
	bool mRenderHighlight;	

	typedef std::vector< FBCollisionShapePtr > COLLISION_SHAPES;
	CowPtr<COLLISION_SHAPES> mCollisions;	
	FRAME_PRECISION mLastPreRendered;

	CowPtr<MeshCameras> mMeshCameras;

	bool mUseDynamicVB[MeshVertexBufferType::Num];
	bool mForceAlphaBlending;
	bool mCheckDistance;

	//---------------------------------------------------------------------------
	Impl(MeshObject* self)
		: mSelf(self)
		, mRenderHighlight(false)
		, mForceAlphaBlending(false)
		, mLastPreRendered(0)
		, mCheckDistance(true)
	{
		mTopology = PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		mObjectConstants.gWorld.MakeIdentity();
		auto& renderer = Renderer::GetInstance();
		SetMaterial(renderer.GetResourceProvider()->GetMaterial(ResourceTypes::Materials::Missing), 0);
		for (int i = 0; i < MeshVertexBufferType::Num; ++i)
		{
			mUseDynamicVB[i] = false;
		}
		mSelf->UseAABBBoundingVolume();
	}

	Impl(MeshObject* self, const Impl& other)
		: mSelf(self)
		, mInputLayoutOverride(other.mInputLayoutOverride)
		, mTopology(other.mTopology)
		, mObjectConstants(other.mObjectConstants)
		, mPointLightConstants(other.mPointLightConstants)
		, mAuxiliary(other.mAuxiliary)
		, mModifying(other.mModifying)
		, mRenderHighlight(other.mRenderHighlight)
		, mCollisions(other.mCollisions)
		, mMeshCameras(other.mMeshCameras)
		, mLastPreRendered(other.mLastPreRendered)
		, mForceAlphaBlending(other.mForceAlphaBlending)

	{
		unsigned idx = 0;
		for (auto& it : other.mMaterialGroups){
			auto& group = GetMaterialGroupFor(idx);
			group.mMaterial = it.mMaterial->Clone();
			group.mVBPos = it.mVBPos;
			group.mVBNormal = it.mVBNormal;
			group.mVBUV = it.mVBUV;
			group.mVBColor = it.mVBColor;
			group.mVBTangent = it.mVBTangent;
			group.mIndexBuffer = it.mIndexBuffer;
			group.mPositions = it.mPositions;
		}
	}

	void SetCheckDistance(bool check){
		mCheckDistance = check;

	}
	//---------------------------------------------------------------------------
	// IRenderable
	//---------------------------------------------------------------------------
	void PreRender(const RenderParam& renderParam, RenderParamOut* renderParamOut){		
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;
		
		auto currentFrame = gpTimer->GetFrame();
		if (mLastPreRendered == currentFrame)
			return;

		mLastPreRendered = currentFrame;
		auto& animatedLocation = mSelf->GetAnimatedLocation();
		animatedLocation.GetHomogeneous(mObjectConstants.gWorld);
		auto& renderer = Renderer::GetInstance();
		assert(renderParam.mScene);
		renderParam.mScene->GatherPointLightData(mSelf->GetAABB().get(), animatedLocation, &mPointLightConstants);
	}
	
	void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut){
		auto& renderer = Renderer::GetInstance();		
		auto renderOption = renderer.GetRendererOptions();
		if (renderOption->r_noMesh)
			return;		
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		if (mCheckDistance && renderParam.mRenderPass == PASS_NORMAL){
			auto radius = mSelf->GetRadius();
			auto distToCam = mSelf->GetDistToCam(renderParam.mCamera);
			if (distToCam > 70 && radius < 0.5f)
				return;

			if (distToCam > 100 && radius < 2.0f)
				return;

			if (distToCam > 250 && radius < 5.0f)
				return;
		}

		RenderEventMarker marker("MeshObject");

		auto camera = renderer.GetCamera();
		mObjectConstants.gWorldView = camera->GetMatrix(Camera::View) * mObjectConstants.gWorld;
		mObjectConstants.gWorldViewProj = camera->GetMatrix(Camera::ViewProj) * mObjectConstants.gWorld;		
		renderer.UpdateObjectConstantsBuffer(&mObjectConstants, true);

		if (renderParam.mRenderPass == RENDER_PASS::PASS_NORMAL)
			renderer.UpdatePointLightConstantsBuffer(&mPointLightConstants);

		renderer.SetPrimitiveTopology(mTopology);

		bool noDedicatedHighlight = !mSelf->HasObjFlag(SceneObjectFlag::HighlightDedi);
		bool renderDepthPath = !mSelf->HasObjFlag(SceneObjectFlag::NoDepthPath);
		bool useDepthPassAndNormalHighlight = noDedicatedHighlight && renderDepthPath;
		auto provider = renderer.GetResourceProvider();
		if (renderParam.mRenderPass == RENDER_PASS::PASS_SHADOW && useDepthPassAndNormalHighlight)
		{
			for(auto& it : mMaterialGroups)
			{
				if (!it.mMaterial || !it.mVBPos || it.mMaterial->IsNoShadowCast())
					continue;
				if (!it.mMaterial->BindSubPass(RENDER_PASS::PASS_SHADOW, true))
				{
					renderer.SetPositionInputLayout();
					provider->BindShader(ResourceTypes::Shaders::ShadowMapShader, true);
				}
				RenderMaterialGroup(&it, true);
			}
			return;
		}
		
		if (renderParam.mRenderPass == RENDER_PASS::PASS_DEPTH && useDepthPassAndNormalHighlight)
		{
			for(auto& it: mMaterialGroups)
			{
				if (!it.mMaterial || !it.mVBPos || it.mMaterial->IsNoShadowCast())
					continue;
				bool materialReady = false;
				if (it.mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
				{
					renderer.SetPositionInputLayout();
					materialReady = true;
				}
				else
				{
					renderer.SetDepthWriteShader();
					materialReady = true;
				}

				if (materialReady)
				{
					RenderMaterialGroup(&it, true);
				}
			}
			return;
		}
		// PASS_GODRAY_OCC_PRE
		if (renderParam.mRenderPass == RENDER_PASS::PASS_GODRAY_OCC_PRE && useDepthPassAndNormalHighlight && !mForceAlphaBlending)
		{
			renderer.SetPositionInputLayout();
			for(auto& it: mMaterialGroups)
			{
				if (!it.mMaterial || !it.mVBPos || it.mMaterial->IsNoShadowCast())
					continue;

				if (it.mMaterial->GetBindingShaders() & SHADER_TYPE_GS) {
					// TODO: Get shader from the material.
					renderer.GetResourceProvider()->BindShader(ResourceTypes::Shaders::OcclusionPrePassVSGSPS, true);
				}
				else {
					renderer.GetResourceProvider()->BindShader(ResourceTypes::Shaders::OcclusionPrePassVSPS, true);
				}
				it.mMaterial->BindShaderConstants();

				RenderMaterialGroup(&it, true);
			}

			return;
		}

		// NORMAL PASS
		if (renderParam.mRenderPass == RENDER_PASS::PASS_NORMAL)
		{
			if (noDedicatedHighlight)
			{
				bool includeInputLayout = true;
				if (mInputLayoutOverride)
				{
					mInputLayoutOverride->Bind();
					includeInputLayout = false;
				}
				if (mForceAlphaBlending && !mMaterialGroups.empty()){
					auto it = mMaterialGroups.begin();
					renderer.SetPositionInputLayout();

					bool hasSubMat = it->mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH_ONLY, false);
					if (hasSubMat){
						// write only depth
						for(auto& it : mMaterialGroups)
						{
							it.mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH_ONLY, false);
							RenderMaterialGroup(&it, true);
						}
					}
					else{
						renderer.RestoreDepthStencilState();
						provider->BindRasterizerState(ResourceTypes::RasterizerStates::OneBiased);
						provider->BindBlendState(ResourceTypes::BlendStates::NoColorWrite);
						provider->BindShader(ResourceTypes::Shaders::DepthOnlyVSPS, true);
						
						// write only depth
						for(auto& it:mMaterialGroups)
						{
							RenderMaterialGroup(&it, true);
						}
					}
				}
				for(auto& it : mMaterialGroups)
				{
					auto material = mForceAlphaBlending ? it.mForceAlphaMaterial : it.mMaterial;
					if (!material || !it.mVBPos)
						continue;

					material->Bind(includeInputLayout);
					RenderMaterialGroup(&it, false);
					material->Unbind();
				}
			}
			bool mainRt = renderer.IsMainRenderTarget();
			
			// HIGHLIGHT
			if (mRenderHighlight && mainRt && !mForceAlphaBlending)
			{
				// draw silhouett to samll buffer
				auto rt = renderer.GetCurrentRenderTarget();
				assert(rt);
				if (rt->SetSmallSilouetteBuffer())
				{
					for(auto& it: mMaterialGroups)
					{
						if (!it.mVBPos)
							continue;
						bool materialReady = false;
						if (it.mMaterial && it.mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
						{
							renderer.SetPositionInputLayout();
							materialReady = true;
						}
						else
						{
							renderer.SetPositionInputLayout();
							renderer.SetDepthWriteShader();
							materialReady = true;
						}
						if (materialReady)
						{
							RenderMaterialGroup(&it, true);
						}
					}
				}
				if (rt->SetBigSilouetteBuffer())
				{
					for (auto& it : mMaterialGroups)
					{
						if (!it.mVBPos)
							continue;
						bool materialReady = false;
						if (it.mMaterial && it.mMaterial->BindSubPass(RENDER_PASS::PASS_DEPTH, false))
						{
							renderer.SetPositionInputLayout();
							materialReady = true;
						}
						else
						{
							renderer.SetPositionInputLayout();
							renderer.SetDepthWriteShader();
							materialReady = true;
						}
						if (materialReady)
						{
							RenderMaterialGroup(&it, true);
						}
					}
				}
				renderParamOut->mSilouetteRendered = true;
				rt->BindTargetOnly(true);
			}
			// debug
			if (renderOption->r_gameId && mSelf->GetGameId() != -1){
				char buf[255];
				sprintf_s(buf, "%u", mSelf->GetGameId());
				renderer.QueueDraw3DText(mSelf->GetPosition(), buf, Color::White);
			}
		}
	}
	
	void PostRender(const RenderParam& renderParam, RenderParamOut* renderParamOut){
		
	}

	//---------------------------------------------------------------------------
	// Own
	//---------------------------------------------------------------------------
	MeshObjectPtr Clone() const{
		return MeshObject::Create(*mSelf);
	}

	void CheckMaterialOptions(MaterialPtr mat){
		if (mat && mat->IsTransparent())
			mSelf->ModifyObjFlag(SceneObjectFlag::Transparent, true);
	}

	void SetMaterial(const char* filepath, int pass){
		auto& group = GetMaterialGroupFor(0);
		auto& renderer = Renderer::GetInstance();
		group.mMaterial = renderer.CreateMaterial(filepath);
		CheckMaterialOptions(group.mMaterial);
	}

	void SetMaterial(MaterialPtr pMat, int pass){
		auto& group = GetMaterialGroupFor(0);
		group.mMaterial = pMat;
		CheckMaterialOptions(group.mMaterial);
	}

	MaterialPtr GetMaterial(int pass) const{
		if (!mMaterialGroups.empty())
		{
			return mForceAlphaBlending ? mMaterialGroups[0].mForceAlphaMaterial : mMaterialGroups[0].mMaterial;
		}
		return 0;
	}

	void SetEnableHighlight(bool enable){
		mRenderHighlight = enable;
	}

	void RenderSimple(bool bindPosOnly){
		for(auto& it: mMaterialGroups)
		{
			if (!it.mVBPos)
				continue;
			RenderMaterialGroup(&it, bindPosOnly);
		}
	}

	void ClearMeshData(){
		for(auto& it: mMaterialGroups)
		{
			it.mPositions.clear();
			it.mNormals.clear();
			it.mUVs.clear();
			it.mColors.clear();

			it.mPositions.resize(0);
			it.mNormals.resize(0);
			it.mUVs.resize(0);
			it.mColors.resize(0);
		}
	}

	void StartModification(){
		mModifying = true;
	}

	void AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mPositions.push_back(pos0);
		group.mPositions.push_back(pos1);
		group.mPositions.push_back(pos2);
	}

	void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4]){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mPositions.push_back(pos[0]);
		group.mPositions.push_back(pos[1]);
		group.mPositions.push_back(pos[2]);

		group.mPositions.push_back(pos[2]);
		group.mPositions.push_back(pos[1]);
		group.mPositions.push_back(pos[3]);


		group.mNormals.push_back(normal[0]);
		group.mNormals.push_back(normal[1]);
		group.mNormals.push_back(normal[2]);

		group.mNormals.push_back(normal[2]);
		group.mNormals.push_back(normal[1]);
		group.mNormals.push_back(normal[3]);
	}

	void AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4]){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		AddQuad(matGroupIdx, pos, normal);
		group.mUVs.push_back(uv[0]);
		group.mUVs.push_back(uv[1]);
		group.mUVs.push_back(uv[2]);

		group.mUVs.push_back(uv[2]);
		group.mUVs.push_back(uv[1]);
		group.mUVs.push_back(uv[3]);
	}

	void SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mPositions.assign(p, p + numVertices);
	}

	void SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mNormals.assign(n, n + numNormals);
	}

	void SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mUVs.assign(uvs, uvs + numUVs);
	}

	void SetTriangles(int matGroupIdx, const ModelTriangle* tris, size_t numTri){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mTriangles.assign(tris, tris + numTri);
	}

	void SetColors(int matGroupIdx, const DWORD* colors, size_t numColors){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mColors.assign(colors, colors + numColors);
	}

	void SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mTangents.assign(t, t + numTangents);
	}

	void SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices){
		auto& renderer = Renderer::GetInstance();
		auto& group = GetMaterialGroupFor(matGroupIdx);
		if (numIndices <= std::numeric_limits<USHORT>::max())
		{
			std::vector<USHORT> sIndices(indices, indices + numIndices);
			group.mIndexBuffer =
				renderer.CreateIndexBuffer(&sIndices[0], numIndices, INDEXBUFFER_FORMAT_16BIT);
		}
		else
		{
			group.mIndexBuffer =
				renderer.CreateIndexBuffer((void*)indices, numIndices, INDEXBUFFER_FORMAT_32BIT);
		}
	}

	void SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		auto& renderer = Renderer::GetInstance();
		group.mIndexBuffer = renderer.CreateIndexBuffer((void*)indices, numIndices, INDEXBUFFER_FORMAT_16BIT);
	}

	void SetIndexBuffer(int matGroupIdx, IndexBufferPtr pIndexBuffer){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mIndexBuffer = pIndexBuffer;
	}

	Vec3* GetPositions(int matGroupIdx, size_t& outNumPositions){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		outNumPositions = group.mPositions.size();
		if (outNumPositions)
			return &(group.mPositions[0]);
		else
			return 0;
	}

	Vec3* GetNormals(int matGroupIdx, size_t& outNumNormals){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		outNumNormals = group.mNormals.size();
		if (outNumNormals)
			return &(group.mNormals[0]);
		else
			return 0;
	}

	Vec2* GetUVs(int matGroupIdx, size_t& outNumUVs){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		outNumUVs = group.mUVs.size();
		if (outNumUVs)
			return &(group.mUVs[0]);
		else
			return 0;
	}

	void GenerateTangent(int matGroupIdx, UINT* indices, size_t num){
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

	void EndModification(bool keepMeshData){
		mModifying = false;
		auto bv = mSelf->GetBoundingVolume();
		bv->StartComputeFromData();
		for(auto& it: mMaterialGroups)
		{
			auto& renderer = Renderer::GetInstance();
			if (!it.mPositions.empty())
			{
				it.mVBPos = renderer.CreateVertexBuffer(
					&it.mPositions[0], sizeof(Vec3f), it.mPositions.size(),
					mUseDynamicVB[MeshVertexBufferType::Position] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[MeshVertexBufferType::Position] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
				bv->AddComputeData(&it.mPositions[0], it.mPositions.size());
			}
			else
			{
				it.mVBPos = 0;
			}
			if (!it.mNormals.empty())
			{
				assert(it.mPositions.size() == it.mNormals.size());
				it.mVBNormal = renderer.CreateVertexBuffer(
					&it.mNormals[0], sizeof(Vec3f), it.mNormals.size(),
					mUseDynamicVB[MeshVertexBufferType::Normal] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[MeshVertexBufferType::Normal] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
			}
			else
			{
				it.mVBNormal = 0;
			}
			if (!it.mUVs.empty())
			{
				assert(it.mPositions.size() == it.mUVs.size());
				it.mVBUV = renderer.CreateVertexBuffer(
					&it.mUVs[0], sizeof(Vec2f), it.mUVs.size(),
					mUseDynamicVB[MeshVertexBufferType::UV] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[MeshVertexBufferType::UV] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);				
			}
			else
			{
				it.mVBUV = 0;
			}

			if (!it.mColors.empty())
			{
				assert(it.mPositions.size() == it.mColors.size());
				it.mVBColor = renderer.CreateVertexBuffer(
					&it.mColors[0], sizeof(DWORD), it.mColors.size(),
					mUseDynamicVB[MeshVertexBufferType::Color] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[MeshVertexBufferType::Color] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);				
			}
			else
			{
				it.mVBColor = 0;
			}

			if (!it.mTangents.empty())
			{
				assert(it.mPositions.size() == it.mTangents.size());
				it.mVBTangent = renderer.CreateVertexBuffer(
					&it.mTangents[0], sizeof(Vec3f), it.mTangents.size(),
					mUseDynamicVB[MeshVertexBufferType::Tangent] ? BUFFER_USAGE_DYNAMIC : BUFFER_USAGE_IMMUTABLE,
					mUseDynamicVB[MeshVertexBufferType::Tangent] ? BUFFER_CPU_ACCESS_WRITE : BUFFER_CPU_ACCESS_NONE);
				
			}
			else
			{
				it.mVBTangent = 0;
			}


		}
		bv->EndComputeFromData();		
		auto boundingVolumeWorld = mSelf->GetBoundingVolumeWorld();
		boundingVolumeWorld->SetCenter(bv->GetCenter() + mSelf->GetPosition());
		const auto& s = mSelf->GetScale();
		boundingVolumeWorld->SetRadius(bv->GetRadius() * std::max(std::max(s.x, s.y), s.z));;

		if (!keepMeshData)
			ClearMeshData();
	}

	void SetMaterialFor(int matGroupIdx, MaterialPtr material){
		auto& group = GetMaterialGroupFor(matGroupIdx);
		group.mMaterial = material;
		if (material && matGroupIdx == 0){
			CheckMaterialOptions(material);			
		}
	}

	void SetTopology(PRIMITIVE_TOPOLOGY topology){
		mTopology = topology;
	}

	PRIMITIVE_TOPOLOGY GetTopology(){
		return mTopology;
	}

	const AUXILIARIES* GetAuxiliaries() const { 
		return mAuxiliary.const_get();
	}

	void AddAuxiliary(const AUXILIARY& aux){
		if (!mAuxiliary){
			mAuxiliary = new AUXILIARIES;
		}
		mAuxiliary->push_back(aux);
	}

	void SetAuxiliaries(const AUXILIARIES& aux) { 
		if (!mAuxiliary){
			mAuxiliary = new AUXILIARIES;
		}
		*mAuxiliary = aux;
	}

	void AddCollisionShape(const COL_SHAPE& data){
		if (!mCollisions){
			mCollisions = new COLLISION_SHAPES;
		}
		FBCollisionShapePtr cs = FBCollisionShape::Create(data.first, data.second, 0);
		mCollisions->push_back(cs);
	}

	void SetCollisionShapes(const COLLISION_INFOS& colInfos){
		if (!mCollisions){
			mCollisions = new COLLISION_SHAPES;
		}
		DeleteCollisionShapes();
		for (const auto& var : colInfos)
		{
			mCollisions->push_back(FBCollisionShape::Create(var.mColShapeType, var.mTransform, var.mCollisionMesh));
		}
	}

	
	void SetCollisionMesh(MeshObjectPtr colMesh){		
		assert(!mCollisions->empty());
		mCollisions->back()->SetCollisionMesh(colMesh);
	}

	void SetMeshCameras(const MeshCameras& cam){
		if (!mMeshCameras)
			mMeshCameras = new MeshCameras;

		*mMeshCameras = cam;
	}

	const MeshCamera& GetMeshCamera(const char* name, bool& outFound) const {
		if (mMeshCameras.const_get()){
			auto it = mMeshCameras.const_get()->Find(name);
			if (it != mMeshCameras.const_get()->end()){
				outFound = true;
				return it->second;
			}
		}
		outFound = false;
		static MeshCamera dummy;		
		return dummy;
	}

	unsigned GetNumCollisionShapes() const { 
		if (!mCollisions)
			return 0;
		return mCollisions.const_get()->size();
	}

	bool HasCollisionShapes() const {
		if (!mCollisions)
			return false;
		return !mCollisions.const_get()->empty();		
	}

	FBCollisionShapeConstPtr GetCollisionShape(unsigned idx) const { 
		if (!mCollisions)
			return 0;
		return mCollisions.const_get()->operator[](idx);		
	}

	bool CheckNarrowCollision(const BoundingVolume* pBV) const{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return true;		
		auto& location = mSelf->GetLocation();
		for (unsigned i = 0; i < num; ++i)
		{
			FBCollisionShapeConstPtr cs = GetCollisionShape(i);
			
			bool collide = cs->TestCollision(pBV, location);
			if (collide) {
				return true;
			}
		}

		return false;
	}

	Ray::IResult CheckNarrowCollisionRay(const Ray& ray) const{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return Ray::IResult(false, 0.f);

		Real minDist = FLT_MAX;
		bool collided = false;
		auto& location = mSelf->GetLocation();
		for (unsigned i = 0; i < num; ++i)
		{
			auto cs = GetCollisionShape(i);
			auto result = cs->Intersects(ray, location);
			if (result.first)
			{
				collided = true;
				if (minDist > result.second)
					minDist = result.second;
			}
		}

		return Ray::IResult(collided, minDist);
	}

	Vec3 GetRandomPosInVolume(const Vec3* nearWorld = 0) const{
		unsigned num = GetNumCollisionShapes();
		if (!num)
			return Vec3::ZERO;

		auto index = Random((unsigned)0, num - 1);
		assert(index < num);
		auto cs = GetCollisionShape(index);
		assert(cs);		
		return cs->GetRandomPosInVolume(nearWorld, mSelf->GetLocation());
	}

	void DeleteCollisionShapes(){
		if (mCollisions){
			mCollisions->clear();
		}
	}

	void SetUseDynamicVB(MeshVertexBufferType::Enum type, bool useDynamicVB){
		mUseDynamicVB[type] = useDynamicVB;
	}

	MapData MapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx){
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
		case MeshVertexBufferType::Position:
			return mg.mVBPos->Map(0, MAP_TYPE_WRITE, MAP_FLAG_NONE);
		case MeshVertexBufferType::Normal:
			return mg.mVBNormal->Map(0, MAP_TYPE_WRITE_NO_OVERWRITE, MAP_FLAG_NONE);
		case MeshVertexBufferType::UV:
			return mg.mVBUV->Map(0, MAP_TYPE_WRITE, MAP_FLAG_NONE);
		case MeshVertexBufferType::Color:
			return mg.mVBColor->Map(0, MAP_TYPE_WRITE, MAP_FLAG_NONE);
		case MeshVertexBufferType::Tangent:
			return mg.mVBTangent->Map(0, MAP_TYPE_WRITE, MAP_FLAG_NONE);
		}
		assert(0);

		return MapData();
	}

	void UnmapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx){
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
		case MeshVertexBufferType::Position:
			return mg.mVBPos->Unmap(0);
		case MeshVertexBufferType::Normal:
			return mg.mVBNormal->Unmap(0);
		case MeshVertexBufferType::UV:
			return mg.mVBUV->Unmap(0);
		case MeshVertexBufferType::Color:
			return mg.mVBColor->Unmap(0);
		case MeshVertexBufferType::Tangent:
			return mg.mVBTangent->Unmap(0);
		}
		assert(0);
	}

	bool RayCast(const Ray& ray, Vec3& location, const ModelTriangle** outTri = 0) const{
		auto& factory = SceneObjectFactory::GetInstance();		
		auto mesh = factory.GetMeshArcheType(mSelf->GetName());
		assert(mesh);
		const Real maxRayDistance = 100000;
		Real tMin = maxRayDistance;
		const Real epsilon = 0.001f;
		ModelIntersection rayTriIntersection;
		rayTriIntersection.valid = false;

		for (const auto& mg : mesh->mImpl->mMaterialGroups)
		{
			for (const auto& tri : mg.mTriangles)
			{
				Real NdotD = tri.faceNormal.Dot(ray.GetDir());
				if (abs(NdotD) < epsilon)
				{
					// ray is parallel or nearly parallel to polygon plane
					continue;
				}

				Real t = (tri.d - tri.faceNormal.Dot(ray.GetOrigin())) / NdotD;
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
				Real alpha;
				Real beta;
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
				Real u0, u1, u2, v0, v1, v2;
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
				Real alpha = rayTriIntersection.alpha;
				Real beta = rayTriIntersection.beta;
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

	BoundingVolumeConstPtr GetAABB() const { 
		return mSelf->GetBoundingVolume(); 
	}

	void ClearVertexBuffers(){
		for (auto& it : mMaterialGroups)
		{
			it.mVBPos = 0;
		}
	}

	void SetAlpha(Real alpha){
		for (auto& it : mMaterialGroups)
		{
			if (it.mMaterial)
			{
				BLEND_DESC bdesc;
				bdesc.RenderTarget[0].BlendEnable = alpha != 1.0f;
				bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
				bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
				bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
				it.mMaterial->SetBlendState(bdesc);
				auto diffuse = it.mMaterial->GetDiffuseColor();
				diffuse.w = alpha;
				it.mMaterial->SetDiffuseColor(diffuse);
			}
		}
	}

	void SetForceAlphaBlending(bool enable, Real alpha, Real forceGlow = 0.f, bool disableDepth = false){
		mForceAlphaBlending = enable;
		if (mForceAlphaBlending){
			for (auto& it : mMaterialGroups)
			{
				if (it.mMaterial)
				{
					if (!it.mForceAlphaMaterial){
						it.mForceAlphaMaterial = it.mMaterial->Clone();						
						if (forceGlow != 0.f){
							it.mForceAlphaMaterial->AddShaderDefine("_FORCE_GLOW", FormatString(".2f", forceGlow).c_str());
							//it.mForceAlphaMaterial->ApplyShaderDefines();
						}
					}
					BLEND_DESC bdesc;
					bdesc.RenderTarget[0].BlendEnable = alpha != 1.0f;
					bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
					bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
					bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
					it.mForceAlphaMaterial->SetBlendState(bdesc);
					auto diffuse = it.mForceAlphaMaterial->GetDiffuseColor();
					diffuse.w = alpha;
					it.mForceAlphaMaterial->SetDiffuseColor(diffuse);

					RASTERIZER_DESC rs;
					rs.CullMode = CULL_MODE_NONE;
					it.mForceAlphaMaterial->SetRasterizerState(rs);

					DEPTH_STENCIL_DESC ds;
					ds.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
					if (disableDepth){
						ds.DepthEnable = false;
					}
					it.mForceAlphaMaterial->SetDepthStencilState(ds);

					it.mForceAlphaMaterial->SetTransparent(true);
					mSelf->ModifyObjFlag(SceneObjectFlag::Transparent, true);
				}
			}
		}
		else{
			for (auto& it : mMaterialGroups)
			{
				if (it.mMaterial)
				{
					mSelf->ModifyObjFlag(SceneObjectFlag::Transparent, it.mMaterial->IsTransparent());
				}
			}
		}
	}

	void SetAmbientColor(const Color& color){
		for (auto& it : mMaterialGroups)
		{
			if (it.mMaterial)
			{
				it.mMaterial->SetAmbientColor(color.GetVec4());
			}
			if (it.mForceAlphaMaterial){
				it.mForceAlphaMaterial->SetAmbientColor(color.GetVec4());
			}
		}
	}

	MaterialGroup& GetMaterialGroupFor(int matGroupIdx){
		while ((int)mMaterialGroups.size() <= matGroupIdx){
			mMaterialGroups.push_back(MaterialGroup());			
			auto& renderer = Renderer::GetInstance();
			mMaterialGroups.back().mMaterial = renderer.GetResourceProvider()->GetMaterial(
				ResourceTypes::Materials::Missing);
		}
		return mMaterialGroups[matGroupIdx];		
	}
	void RenderMaterialGroup(MaterialGroup* it, bool onlyPos){
		assert(it);
		if (!it || !it->mMaterial || !it->mVBPos)
			return;
		auto& renderer = Renderer::GetInstance();
		if (onlyPos)
		{
			const unsigned int numBuffers = 1;

			VertexBufferPtr buffers[numBuffers] = { it->mVBPos };
			unsigned int strides[numBuffers] = { it->mVBPos->GetStride() };
			unsigned int offsets[numBuffers] = { 0 };
			renderer.SetVertexBuffers(0, numBuffers, buffers, strides, offsets);
			if (it->mIndexBuffer)
			{
				it->mIndexBuffer->Bind(0);
				renderer.DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
			}
			else
			{
				renderer.Draw(it->mVBPos->GetNumVertices(), 0);
			}
		}
		else
		{
			const unsigned int numBuffers = 5;

			VertexBufferPtr buffers[numBuffers] = { it->mVBPos, it->mVBNormal, it->mVBUV, it->mVBColor, it->mVBTangent };
			unsigned int strides[numBuffers] = { it->mVBPos->GetStride(),
				it->mVBNormal ? it->mVBNormal->GetStride() : 0,
				it->mVBUV ? it->mVBUV->GetStride() : 0,
				it->mVBColor ? it->mVBColor->GetStride() : 0,
				it->mVBTangent ? it->mVBTangent->GetStride() : 0 };
			unsigned int offsets[numBuffers] = { 0, 0, 0, 0, 0 };
			renderer.SetVertexBuffers(0, numBuffers, buffers, strides, offsets);
			if (it->mIndexBuffer)
			{
				it->mIndexBuffer->Bind(0);				
				renderer.DrawIndexed(it->mIndexBuffer->GetNumIndices(), 0, 0);
			}
			else
			{
				renderer.Draw(it->mVBPos->GetNumVertices(), 0);
			}
		}
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(MeshObject);
MeshObjectPtr MeshObject::Create(const MeshObject& other){
	return MeshObjectPtr(new MeshObject(other), [](MeshObject* obj){ delete obj; });
}
MeshObject::MeshObject()
	: mImpl(new Impl(this))
{
	
}

MeshObject::MeshObject(const MeshObject& other)
	: SpatialSceneObject(other)
	, mImpl(new Impl(this, *other.mImpl))
{

}

MeshObject::~MeshObject(){

}

MeshObjectPtr MeshObject::Clone() const{
	return mImpl->Clone();
}

void MeshObject::SetMaterial(const char* filepath){
	mImpl->SetMaterial(filepath, PASS_NORMAL);
}

void MeshObject::SetMaterial(const char* filepath, int pass) {
	mImpl->SetMaterial(filepath, pass);
}

void MeshObject::SetMaterial(MaterialPtr pMat, int pass) {
	mImpl->SetMaterial(pMat, pass);
}

MaterialPtr MeshObject::GetMaterial(int pass) const {
	return mImpl->GetMaterial(pass);
}

void MeshObject::PreRender(const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->PreRender(renderParam, renderParamOut);
}

void MeshObject::Render(const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->Render(renderParam, renderParamOut);
}

void MeshObject::PostRender(const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->PostRender(renderParam, renderParamOut);
}

void MeshObject::SetEnableHighlight(bool enable) {
	mImpl->SetEnableHighlight(enable);
}

void MeshObject::RenderSimple(bool bindPosOnly) {
	mImpl->RenderSimple(bindPosOnly);
}

void MeshObject::ClearMeshData() {
	mImpl->ClearMeshData();
}

void MeshObject::StartModification() {
	mImpl->StartModification();
}

void MeshObject::AddTriangle(int matGroupIdx, const Vec3& pos0, const Vec3& pos1, const Vec3& pos2) {
	mImpl->AddTriangle(matGroupIdx, pos0, pos1, pos2);
}

void MeshObject::AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4]) {
	mImpl->AddQuad(matGroupIdx, pos, normal);
}

void MeshObject::AddQuad(int matGroupIdx, const Vec3 pos[4], const Vec3 normal[4], const Vec2 uv[4]) {
	mImpl->AddQuad(matGroupIdx, pos, normal, uv);
}

void MeshObject::SetPositions(int matGroupIdx, const Vec3* p, size_t numVertices) {
	mImpl->SetPositions(matGroupIdx, p, numVertices);
}

void MeshObject::SetNormals(int matGroupIdx, const Vec3* n, size_t numNormals) {
	mImpl->SetNormals(matGroupIdx, n, numNormals);
}

void MeshObject::SetUVs(int matGroupIdx, const Vec2* uvs, size_t numUVs) {
	mImpl->SetUVs(matGroupIdx, uvs, numUVs);
}

void MeshObject::SetTriangles(int matGroupIdx, const ModelTriangle* tris, size_t numTri) {
	mImpl->SetTriangles(matGroupIdx, tris, numTri);
}

void MeshObject::SetColors(int matGroupIdx, const DWORD* colors, size_t numColors) {
	mImpl->SetColors(matGroupIdx, colors, numColors);
}

void MeshObject::SetTangents(int matGroupIdx, const Vec3* t, size_t numTangents) {
	mImpl->SetTangents(matGroupIdx, t, numTangents);
}

void MeshObject::SetIndices(int matGroupIdx, const UINT* indices, size_t numIndices) {
	mImpl->SetIndices(matGroupIdx, indices, numIndices);
}

void MeshObject::SetIndices(int matGroupIdx, const USHORT* indices, size_t numIndices) {
	mImpl->SetIndices(matGroupIdx, indices, numIndices);
}

void MeshObject::SetIndexBuffer(int matGroupIdx, IndexBufferPtr pIndexBuffer) {
	mImpl->SetIndexBuffer(matGroupIdx, pIndexBuffer);
}

Vec3* MeshObject::GetPositions(int matGroupIdx, size_t& outNumPositions) {
	return mImpl->GetPositions(matGroupIdx, outNumPositions);
}

Vec3* MeshObject::GetNormals(int matGroupIdx, size_t& outNumNormals) {
	return mImpl->GetNormals(matGroupIdx, outNumNormals);
}

Vec2* MeshObject::GetUVs(int matGroupIdx, size_t& outNumUVs) {
	return mImpl->GetUVs(matGroupIdx, outNumUVs);
}

void MeshObject::GenerateTangent(int matGroupIdx, UINT* indices, size_t num) {
	mImpl->GenerateTangent(matGroupIdx, indices, num);
}

void MeshObject::EndModification(bool keepMeshData) {
	mImpl->EndModification(keepMeshData);
}

void MeshObject::SetMaterialFor(int matGroupIdx, MaterialPtr material) {
	mImpl->SetMaterialFor(matGroupIdx, material);
}

void MeshObject::SetTopology(PRIMITIVE_TOPOLOGY topology) {
	mImpl->SetTopology(topology);
}

PRIMITIVE_TOPOLOGY MeshObject::GetTopology() {
	return mImpl->GetTopology();
}

const AUXILIARIES* MeshObject::GetAuxiliaries() const {
	return mImpl->GetAuxiliaries();
}

void MeshObject::AddAuxiliary(const AUXILIARY& aux) {
	mImpl->AddAuxiliary(aux);
}

void MeshObject::SetAuxiliaries(const AUXILIARIES& auxes) {
	mImpl->SetAuxiliaries(auxes);
}

void MeshObject::AddCollisionShape(const COL_SHAPE& data) {
	mImpl->AddCollisionShape(data);
}

void MeshObject::SetCollisionShapes(const COLLISION_INFOS& colInfos) {
	mImpl->SetCollisionShapes(colInfos);
}

void MeshObject::SetCollisionMesh(MeshObjectPtr colMesh) {
	mImpl->SetCollisionMesh(colMesh);
}

void MeshObject::SetMeshCameras(const MeshCameras& cam){
	mImpl->SetMeshCameras(cam);
}

const MeshCamera& MeshObject::GetMeshCamera(const char* name, bool& outFound) const{
	return mImpl->GetMeshCamera(name, outFound);
}

unsigned MeshObject::GetNumCollisionShapes() const {
	return mImpl->GetNumCollisionShapes();
}

bool MeshObject::HasCollisionShapes() const {
	return mImpl->HasCollisionShapes();
}

FBCollisionShapeConstPtr MeshObject::GetCollisionShape(unsigned idx) const {
	return mImpl->GetCollisionShape(idx);
}

bool MeshObject::CheckNarrowCollision(const BoundingVolume* pBV) const {
	return mImpl->CheckNarrowCollision(pBV);
}

Ray::IResult MeshObject::CheckNarrowCollisionRay(const Ray& ray) const {
	return mImpl->CheckNarrowCollisionRay(ray);
}

Vec3 MeshObject::GetRandomPosInVolume(const Vec3* nearWorld) const {
	return mImpl->GetRandomPosInVolume(nearWorld);
}

void MeshObject::DeleteCollisionShapes() {
	mImpl->DeleteCollisionShapes();
}

void MeshObject::SetUseDynamicVB(MeshVertexBufferType::Enum type, bool useDynamicVB) {
	mImpl->SetUseDynamicVB(type, useDynamicVB);
}

MapData MeshObject::MapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx) {
	return mImpl->MapVB(type, materialGroupIdx);
}

void MeshObject::UnmapVB(MeshVertexBufferType::Enum type, size_t materialGroupIdx) {
	mImpl->UnmapVB(type, materialGroupIdx);
}

bool MeshObject::RayCast(const Ray& ray, Vec3& location, const ModelTriangle** outTri) const {
	return mImpl->RayCast(ray, location, outTri);
}

BoundingVolumeConstPtr MeshObject::GetAABB() const {
	return mImpl->GetAABB();
}

void MeshObject::ClearVertexBuffers() {
	mImpl->ClearVertexBuffers();
}

void MeshObject::SetAlpha(Real alpha) {
	mImpl->SetAlpha(alpha);
}

void MeshObject::SetForceAlphaBlending(bool enable, Real alpha, Real forceGlow, bool disableDepth) {
	mImpl->SetForceAlphaBlending(enable, alpha, forceGlow, disableDepth);
}

void MeshObject::SetAmbientColor(const Color& color) {
	mImpl->SetAmbientColor(color);
}

void MeshObject::SetCheckDistance(bool check){
	mImpl->SetCheckDistance(check);
}