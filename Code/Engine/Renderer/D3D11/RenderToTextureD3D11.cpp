#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3D11/RenderToTextureD3D11.h>
#include <Engine/ILight.h>

namespace fastbird
{

void RenderToTextureD3D11::SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap)
{
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
}

void RenderToTextureD3D11::SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap)
{
	int type;
	type = srv ? TEXTURE_TYPE_DEPTH_STENCIL_SRV : TEXTURE_TYPE_DEPTH_STENCIL;
	if (cubeMap)
		type |= TEXTURE_TYPE_CUBE_MAP;
	mDepthStencilTexture = gFBEnv->pRenderer->CreateTexture(0, width, height, format,
		BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, type);
}

}