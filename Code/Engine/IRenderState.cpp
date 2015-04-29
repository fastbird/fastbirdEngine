#include <Engine/StdAfx.h>
#include <Engine/IRenderState.h>

namespace fastbird
{
	RenderStates::RenderStates()
	{
		Reset();
	}

	void RenderStates::Reset()
	{
		ResetRasterizerState();
		ResetBlendState();
		ResetDepthStencilState();
	}
	void RenderStates::ResetRasterizerState()
	{
		mRDesc = RASTERIZER_DESC();
		CreateRasterizerState(mRDesc);
	}
	void RenderStates::ResetBlendState()
	{
		mBDesc = BLEND_DESC();
		CreateBlendState(mBDesc);
	}
	void RenderStates::ResetDepthStencilState()
	{
		mDDesc = DEPTH_STENCIL_DESC();
		CreateDepthStencilState(mDDesc);
	}

	void RenderStates::CreateRasterizerState(const RASTERIZER_DESC& desc)
	{
		mRDesc = desc;
		if (gFBEnv && gFBEnv->pRenderer)
			mRasterizerState = gFBEnv->pRenderer->CreateRasterizerState(desc);
	}
	void RenderStates::CreateBlendState(const BLEND_DESC& desc)
	{
		mBDesc = desc;
		if (gFBEnv && gFBEnv->pRenderer)
			mBlendState = gFBEnv->pRenderer->CreateBlendState(desc);
	}
	void RenderStates::CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc)
	{
		mDDesc = desc;
		if (gFBEnv && gFBEnv->pRenderer)
			mDepthStencilState = gFBEnv->pRenderer->CreateDepthStencilState(desc);
	}

	void RenderStates::Bind(unsigned stencilRef)
	{
		mRasterizerState->Bind();
		mBlendState->Bind();
		mDepthStencilState->Bind(stencilRef);
	}

	RenderStates* RenderStates::Clone() const
	{
		RenderStates* newObj = FB_NEW(RenderStates);
		newObj->CreateBlendState(mBDesc);
		newObj->CreateDepthStencilState(mDDesc);
		newObj->CreateRasterizerState(mRDesc);
		return newObj;
	}
}