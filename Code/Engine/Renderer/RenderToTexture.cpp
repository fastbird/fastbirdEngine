#include <Engine/StdAfx.h>
#include <Engine/Renderer/RenderToTexture.h>
#include <Engine/Renderer/Camera.h>
#include <Engine/SceneGraph/Scene.h>
#include <Engine/Renderer/Light.h>

namespace fastbird
{

RenderToTexture::RenderToTexture()
	: mClearColor(0, 0, 0, 1)
	, mDepthClear(1.f)
	, mStencilClear(0)
	, mEveryFrame(false)
{
	mScene = FB_NEW(Scene);
	mCamera = FB_NEW(Camera);
}

void RenderToTexture::SetClearValues(const Color& color, float z, UINT8 stencil)
{
	mClearColor = color;
	mDepthClear = z;
	mStencilClear = stencil;
}

void RenderToTexture::Render(size_t face)
{
	ITexture* rt[]={mRenderTargetTexture};
	if (mRenderTargetTexture)
		mRenderTargetTexture->Unbind();

	size_t rtIndex[] = {face};
	gFBEnv->pRenderer->SetRenderTarget(rt, rtIndex, 1, mDepthStencilTexture, face);
	gFBEnv->pRenderer->Clear(mClearColor.r(), mClearColor.g(), mClearColor.b(), mClearColor.a(),
		mDepthClear, mStencilClear);
	if (mLight)
		gFBEnv->pRenderer->SetDirectionalLight(mLight);
	gFBEnv->pRenderer->SetCamera(mCamera);
	const Vec2I& resol = mRenderTargetTexture->GetSize();
	Viewport vp = {0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f};
	gFBEnv->pRenderer->SetViewports(&vp, 1);
	gFBEnv->pRenderer->UpdateFrameConstantsBuffer();

	mScene->PreRender();
	mScene->Render();

	gFBEnv->pRenderer->RestoreRenderTarget();
	gFBEnv->pRenderer->RestoreViewports();
	gFBEnv->pRenderer->SetCamera(gFBEnv->pEngine->GetCamera(0));
	
	if (mLight)
		gFBEnv->pRenderer->SetDirectionalLight(0);
}

ILight* RenderToTexture::GetLight()
{
	if (!mLight)
	{
		mLight = ILight::CreateLight(ILight::LIGHT_TYPE_DIRECTIONAL);
		mLight->SetPosition(Vec3(1, 1, 1));
		mLight->SetDiffuse(Vec3(1, 1, 1));
		mLight->SetSpecular(Vec3(1, 1, 1));
		mLight->SetIntensity(1.0f);
	}

	return mLight;
}

}