#include <Engine/StdAfx.h>
#include <Engine/RenderTargetD3D11.h>
#include <Engine/ILight.h>

namespace fastbird
{

void RenderTargetD3D11::SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap)
{
	mSize.x = width, mSize.y = height;
	mSizeCropped.x = gFBEnv->pRenderer->CropSize8(width);
	mSizeCropped.y = gFBEnv->pRenderer->CropSize8(height);
	mFormat = format;
	mSRV = srv;
	mMiplevel = miplevel;
	mCubeMap = cubeMap;

	int type;
	type = srv ? TEXTURE_TYPE_RENDER_TARGET_SRV : TEXTURE_TYPE_RENDER_TARGET;
	if (miplevel)
		type |= TEXTURE_TYPE_MIPS;
	if (cubeMap)
		type |= TEXTURE_TYPE_CUBE_MAP;
	mRenderTargetTexture = gFBEnv->pRenderer->CreateTexture(0, width, height, format,
		BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, type);
	mCamera->SetWidth((float)width);
	mCamera->SetHeight((float)height);

	mViewport.mTopLeftX = 0;
	mViewport.mTopLeftY = 0;
	mViewport.mWidth = mSize.x;
	mViewport.mHeight = mSize.y;
	mViewport.mMinDepth = 0.f;
	mViewport.mMaxDepth = 1.0f;
}

void RenderTargetD3D11::SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap)
{
	int type;
	type = srv ? TEXTURE_TYPE_DEPTH_STENCIL_SRV : TEXTURE_TYPE_DEPTH_STENCIL;
	if (cubeMap)
		type |= TEXTURE_TYPE_CUBE_MAP;
	mDepthStencilTexture = gFBEnv->pRenderer->CreateTexture(0, width, height, format,
		BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, type);
	mHasDepth = true;
}

}