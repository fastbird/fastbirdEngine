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
#include "SkySphere.h"
#include "FBRenderer/RenderStates.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/ResourceProvider.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Texture.h"
#include "FBRenderer/RenderStrategyMinimum.h"
#include "FBRenderer/RendererOptions.h"
#include "FBSceneManager/Scene.h"
#include "FBSceneManager/DirectionalLight.h"
#include "FBRenderer/Camera.h"
#include "SceneObjectFactory.h"
#include "MeshObject.h"
#include "ISkySphereLIstener.h"

using namespace fb;
static RenderTargetPtr sRT;
static ScenePtr sScene;
static RenderStrategyMinimumPtr sRenderStrategy;
static const int ENV_SIZE = 1024;
class SkySphere::Impl{
public:
	SkySphere* mSelf;
	SkySphereWeakPtr mSelfPtr;
	MaterialPtr mMaterial;
	MaterialPtr mMaterialOCC;
	VertexBufferPtr mVB;
	IndexBufferPtr mIB;
	Vec4 mMaterialParamCur[5];
	Vec4 mMaterialParamDest[5];
	float mCurInterpolationTime;
	float mInterpolationTime;
	bool mInterpolating;
	bool mUseAlphaBlend;	
	float mAlpha;	
	unsigned mLastPreRendered;
	// alphablend sky
	SkySpherePtr mBlendingSkySphere;
	MeshObjectPtr mMesh;
	

	Impl(SkySphere* self)
		: mSelf(self)
		, mCurInterpolationTime(0)
		, mInterpolating(false)
		, mUseAlphaBlend(false)
		, mAlpha(1.0f)
		, mLastPreRendered(0)
	{
	}

	// IRenderable
	void PreRender(const RenderParam& param, RenderParamOut* paramOut){
		auto& renderer = Renderer::GetInstance();
		auto renderOption = renderer.GetRendererOptions();
		if (renderOption->r_noSky)
			return;

		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		if (mLastPreRendered == gpTimer->GetFrame())
			return;
		mLastPreRendered = gpTimer->GetFrame();

		if (mBlendingSkySphere && mBlendingSkySphere->GetAlpha() == 1.0f){
			mBlendingSkySphere->PreRender(param, paramOut);
			return;
		}

		if (mInterpolating)
		{
			mCurInterpolationTime += gpTimer->GetDeltaTime();
			float normTime = mCurInterpolationTime / mInterpolationTime;
			if (normTime >= 1.0f)
			{
				normTime = 1.0f;
				mInterpolating = false;
				if (!mUseAlphaBlend)
				{
					bool listenerFound = false;
					for (auto l : mSelf->mListeners) {
						l->OnInterpolationFinished(mSelf);
						listenerFound = true;
					}
					if (!listenerFound) {
						SceneObjectFactory::GetInstance().UpdateEnvMapInNextFrame(mSelfPtr.lock());
					}
				}
			}

			for (int i = 0; i < 5; i++)
			{
				mMaterial->SetShaderParameter(i, Lerp(mMaterialParamCur[i], mMaterialParamDest[i], normTime));
				if (mMaterialOCC)
				{
					mMaterialOCC->SetShaderParameter(i, Lerp(mMaterialParamCur[i], mMaterialParamDest[i], normTime));
				}
			}
		}

		if (mBlendingSkySphere && mBlendingSkySphere->GetAlpha() != 0.f){
			mBlendingSkySphere->PreRender(param, paramOut);
		}
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide))
			return;

		auto& renderer = Renderer::GetInstance();
		auto renderOption = renderer.GetRendererOptions();
		if (renderOption->r_noSky)
			return;

		if (mBlendingSkySphere && mBlendingSkySphere->GetAlpha() == 1.0f){
			mBlendingSkySphere->Render(param, paramOut);
			return;
		}

		if (!mMaterial)
		{
			assert(0);
			return;
		}
		
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		renderer.UnbindInputLayout();
		renderer.UnbindVertexBuffers();
		if (mMaterialOCC && param.mRenderPass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
		{
			RenderEventMarker mark("SkySphere_OCC");
			mMaterialOCC->Bind(false);
			renderer.Draw(3, 0);
		}
		else if (param.mRenderPass == RENDER_PASS::PASS_NORMAL)
		{
			RenderEventMarker mark("SkySphere");			
			mMaterial->Bind(false);
			if (mUseAlphaBlend)
			{
				renderer.GetResourceProvider()->BindBlendState(ResourceTypes::BlendStates::AlphaBlend);
			}
			if (mMesh) {
				renderer.GetResourceProvider()->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthWrite_LessEqual, 0);
				mMesh->RenderSimple(false);
			}
			else {
				renderer.Draw(3, 0);
			}
		}

		if (mBlendingSkySphere && mBlendingSkySphere->GetAlpha() != 0.f){
			mBlendingSkySphere->Render(param, paramOut);
		}
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}

	// SkySphere
	void SetMaterial(const char* filepath, int pass){
		auto material = Renderer::GetInstance().CreateMaterial(filepath);
		if (material)
		{
			if (pass == RENDER_PASS::PASS_NORMAL)
				mMaterial = material;
			else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
				mMaterialOCC = material;
			else{
				Logger::Log(FB_ERROR_LOG_ARG, "You can set sky mateirl only for NORMAL AND GODRAY_OCC Pass.");
				assert(0);
			}
		}
	}

	void SetGeometry(const char* path) {
		MeshImportDesc desc;
		desc.generateTangent = false;
		mMesh = SceneObjectFactory::GetInstance().CreateMeshObject(path, desc);
	}

	void SetMaterial(MaterialPtr pMat, int pass){
		if (pass == RENDER_PASS::PASS_NORMAL)
			mMaterial = pMat;
		else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
			mMaterialOCC = pMat;
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "You can set sky mateirl only for NORMAL AND GODRAY_OCC Pass.");
			assert(0);
		}
	}

	MaterialPtr GetMaterial(int pass = 0) const{
		if (pass == RENDER_PASS::PASS_NORMAL)
			return mMaterial;
		else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
			return mMaterialOCC;
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "You can get sky mateirls only for NORMAL AND GODRAY_OCC Pass.");
			assert(0);
		}

		return 0;
	}

	void UpdateEnvironmentMap(const Vec3& origin){
		if (!sRT)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No render target found.");
			return;
		}

		auto& renderer = Renderer::GetInstance();
		TexturePtr pTexture = sRT->GetRenderTargetTexture();		
		pTexture->Unbind();
		SceneManager::GetInstance().CopyDirectionalLight(sRT->GetScene(), 0, renderer.GetMainRenderTarget()->GetScene(), 0);		
		sRT->GetScene()->AttachSky(mSelfPtr.lock());
		sRT->GetCamera()->SetPosition(origin);
		sRT->GetCamera()->SetFOV(HALF_PI);
		sRT->GetCamera()->SetAspectRatio(1.0f);
		Vec3 dirs[] = {
			Vec3(1, 0, 0), Vec3(-1, 0, 0),
			Vec3(0, 1, 0), Vec3(0, -1, 0),
			Vec3(0, 0, 1), Vec3(0, 0, -1),			
		};
		for (int i = 0; i < 6; i++)
		{
			sRT->GetCamera()->SetDirection(dirs[i]);
			sRT->Render(i);
		}
		// this is for unbind the environment map from the output slot.
		renderer.GetMainRenderTarget()->BindTargetOnly(false);

		pTexture->GenerateMips();
		//pTexture->SaveToFile("environment.dds");
		// for bight test.
		//ITexture* textureFile = gFBEnv->pRenderer->CreateTexture("Data/textures/brightEnv.jpg");
		
		renderer.SetEnvironmentTexture(pTexture);
		sRT->GetScene()->DetachSky();
	}

	void SetEnvironmentUpdaterListener(ISceneObserverPtr observer) {
		// Duplication will be checked.
		sRT->GetScene()->AddSceneObserver(ISceneObserver::Timing, observer);
	}

	void SetInterpolationData(unsigned index, const Vec4& data){
		assert(index < 5);
		mMaterialParamDest[index] = data;
		if (index == 3)
		{
			mMaterialParamDest[index].w = mAlpha;
		}
	}

	void StartInterpolation(float time){
		mInterpolationTime = time;
		mCurInterpolationTime = 0;
		mInterpolating = true;
	}

	void PrepareInterpolation(float time, SkySpherePtr startFrom){
		MaterialPtr srcMaterial = mMaterial;
		if (startFrom)
			srcMaterial = startFrom->GetMaterial();
		for (int i = 0; i < 5; i++)
		{
			mMaterialParamCur[i] = srcMaterial->GetShaderParameter(i);
		}

		mInterpolationTime = time;
		mCurInterpolationTime = 0;
		mInterpolating = true;
	}

	void SetUseAlphaBlend(bool use){
		mUseAlphaBlend = use;
	}

	void SetAlpha(float alpha){
		Vec4 param = mMaterial->GetShaderParameter(3);
		mAlpha = param.w = alpha;
		mMaterial->SetShaderParameter(3, param);
		mMaterialParamDest[3].w = alpha;
		mMaterialParamDest[2].w = alpha;
	}

	float GetAlpha() const{
		return mAlpha;
	}

	void AttachBlendingSky(SkySpherePtr sky){
		mBlendingSkySphere = sky;
		mBlendingSkySphere->SetUseAlphaBlend(true);
	}

	void DetachBlendingSky(){
		mBlendingSkySphere = 0;
	}

	SkySpherePtr GetBlendingSky(){
		return mBlendingSkySphere;
	}
};

//---------------------------------------------------------------------------
void SkySphere::CreateSharedEnvRT(){
	if (!sRT)
	{
		RenderTargetParam param;
		param.mEveryFrame = false;
		param.mSize = Vec2I(ENV_SIZE, ENV_SIZE);
		param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
		param.mShaderResourceView = true;
		param.mMipmap = true;
		param.mCubemap = true;
		param.mWillCreateDepth = true;		
		param.mUsePool = true;
		auto& renderer = Renderer::GetInstance();
		sRT = renderer.CreateRenderTarget(param);		
		sRT->SetDepthStencilDesc(ENV_SIZE, ENV_SIZE, PIXEL_FORMAT_D24_UNORM_S8_UINT, false, true);
		sScene = Scene::Create("SkySphereScene");
		sRT->RegisterScene(sScene);
		sRenderStrategy = RenderStrategyMinimum::Create();
		sRT->SetRenderStrategy(sRenderStrategy);		
	}
}
void SkySphere::DestroySharedEnvRT(){
	sRenderStrategy = 0;
	sScene = 0;
	sRT = 0;
}

//---------------------------------------------------------------------------
SkySpherePtr SkySphere::Create(){
	SkySpherePtr p(new SkySphere, [](SkySphere* obj){ delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}
SkySphere::SkySphere()
	: mImpl(new Impl(this))
{
}

SkySphere::~SkySphere()
{
}

void SkySphere::SetMaterial(const char* filepath, int pass) {
	mImpl->SetMaterial(filepath, pass);
}

void SkySphere::SetMaterial(MaterialPtr pMat, int pass) {
	mImpl->SetMaterial(pMat, pass);
}

MaterialPtr SkySphere::GetMaterial(int pass) const {
	return mImpl->GetMaterial(pass);
}

void SkySphere::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void SkySphere::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void SkySphere::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}

void SkySphere::UpdateEnvironmentMap(const Vec3& origin) {
	mImpl->UpdateEnvironmentMap(origin);
}

void SkySphere::SetEnvironmentUpdaterListener(ISceneObserverPtr observer) {
	mImpl->SetEnvironmentUpdaterListener(observer);
}

void SkySphere::SetInterpolationData(unsigned index, const Vec4& data) {
	mImpl->SetInterpolationData(index, data);
}

void SkySphere::StartInterpolation(float time){
	mImpl->StartInterpolation(time);
}

void SkySphere::PrepareInterpolation(float time, SkySpherePtr startFrom) {
	mImpl->PrepareInterpolation(time, startFrom);
}

void SkySphere::SetUseAlphaBlend(bool use) {
	mImpl->SetUseAlphaBlend(use);
}

void SkySphere::SetAlpha(float alpha) {
	mImpl->SetAlpha(alpha);
}

float SkySphere::GetAlpha() const {
	return mImpl->GetAlpha();
}

void SkySphere::AttachBlendingSky(SkySpherePtr sky){
	mImpl->AttachBlendingSky(sky);
}

void SkySphere::DetachBlendingSky(){
	mImpl->DetachBlendingSky();
}

SkySpherePtr SkySphere::GetBlendingSky(){
	return mImpl->GetBlendingSky();
}

void SkySphere::SetGeometry(const char* path) {
	mImpl->SetGeometry(path);
}