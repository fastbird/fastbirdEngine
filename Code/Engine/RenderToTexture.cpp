#include <Engine/StdAfx.h>
#include <Engine/RenderToTexture.h>
#include <Engine/Camera.h>
#include <Engine/Renderer.h>
#include <Engine/Scene.h>
#include <Engine/Light.h>

namespace fastbird
{

RenderToTexture::RenderToTexture()
	: mClearColor(0, 0, 0, 1)
	, mDepthClear(1.f)
	, mStencilClear(0)
	, mEnabled(true)
	, mSize(0, 0)
	, mFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
	, mSRV(false)
	, mMiplevel(false)
	, mCubeMap(false)
	, mHasDepth(false), mUsePool(true)
{
	mScene = FB_NEW(Scene);
	mScene->SetRttScene(true);
	mCamera = FB_NEW(Camera);
}

bool RenderToTexture::CheckOptions(const RenderToTextureParam& param)
{
	return param.mSize == mSize && param.mPixelFormat == mFormat && 
		param.mShaderResourceView == mSRV && 
		param.mMipmap == mMiplevel && param.mCubemap == mCubeMap && param.mHasDepth == mHasDepth && param.mUsePool == mUsePool;
}

const Vec2I& RenderToTexture::GetSize() const
{
	return mSize;
}

void RenderToTexture::SetClearValues(const Color& color, float z, UINT8 stencil)
{
	mClearColor = color;
	mDepthClear = z;
	mStencilClear = stencil;
}

void RenderToTexture::Bind(size_t face)
{
	if (!mEnabled)
		return;
	
	ITexture* rt[] = { mRenderTargetTexture };
	if (mRenderTargetTexture)
		mRenderTargetTexture->Unbind();

	size_t rtIndex[] = { face };
	gFBEnv->pRenderer->SetRenderTarget(rt, rtIndex, 1, mDepthStencilTexture, face);
	gFBEnv->pRenderer->Clear(mClearColor.r(), mClearColor.g(), mClearColor.b(), mClearColor.a(),
		mDepthClear, mStencilClear);
	for (int i = 0; i < 2; i++)
	{
		if (mLight[i])
			gFBEnv->pRenderer->SetDirectionalLight(mLight[i], i);
	}
	gFBEnv->pRenderer->SetCamera(mCamera);
	mCamera->ProcessInputData();
	const Vec2I& resol = mRenderTargetTexture->GetSize();
	Viewport vp = { 0, 0, (float)resol.x, (float)resol.y, 0.f, 1.f };
	gFBEnv->pRenderer->SetViewports(&vp, 1);
	gFBEnv->pRenderer->UpdateFrameConstantsBuffer();
	if (mEnvTexture)
	{
		gFBEnv->pRenderer->SetEnvironmentTextureOverride(mEnvTexture);
	}
}

void RenderToTexture::Render(size_t face)
{
	if (!mEnabled)
		return;
	D3DEventMarker mark("RenderToTexture");
	Bind(face);
	gFBEnv->mRenderToTexture = this;
	/*D3DEventMarker mark("Shadow pass");
	Renderer* pRenderer = (Renderer*)gFBEnv->pRenderer;
	pRenderer->BindShadowMap(false);
	pRenderer->PrepareShadowMapRendering();
	mScene->Render();
	pRenderer->EndShadowMapRendering();*/

	gFBEnv->pRenderer->RestoreBlendState();
	mScene->PreRender();
	mScene->Render();
	gFBEnv->mRenderToTexture = 0;
	Unbind();	
}

void RenderToTexture::Unbind()
{
	gFBEnv->pRenderer->RestoreRenderTarget();
	gFBEnv->pRenderer->RestoreViewports();
	gFBEnv->pRenderer->SetCamera(gFBEnv->pEngine->GetCamera(0));

	for (int i = 0; i < 2; i++)
	{
		if (mLight[i])
			gFBEnv->pRenderer->SetDirectionalLight(0, i);
	}


	if (mEnvTexture)
	{
		gFBEnv->pRenderer->SetEnvironmentTextureOverride(0);
	}
}

ILight* RenderToTexture::GetLight(int idx)
{
	if (!mLight[idx])
	{
		mLight[idx] = ILight::CreateLight(ILight::LIGHT_TYPE_DIRECTIONAL);
		mLight[idx]->SetPosition(Vec3(1, 1, 1));
		mLight[idx]->SetDiffuse(Vec3(1, 1, 1));
		mLight[idx]->SetSpecular(Vec3(1, 1, 1));
		mLight[idx]->SetIntensity(1.0f);
	}

	return mLight[idx];
}

void RenderToTexture::OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	if (!pMouse || !pMouse->IsValid())
		return;
	mCamera->SetCurrent(true);
	mCamera->OnInputFromEngine(pMouse, pKeyboard);
	mCamera->SetCurrent(false);
}

void RenderToTexture::SetEnvTexture(ITexture* texture)
{
	mEnvTexture = texture;
}

void RenderToTexture::SetUsePool(bool usePool)
{
	mUsePool = usePool;
}

}