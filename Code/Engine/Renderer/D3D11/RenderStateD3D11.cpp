#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3D11/RenderStateD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>

using namespace fastbird;

//----------------------------------------------------------------------------
// RASTERIZER STATE
//-------------------------------------------------------------------------
//----------------------------------------------------------------------------
RasterizerStateD3D11::RasterizerStateD3D11()
	: mRasterizerState(0)
{
}

//----------------------------------------------------------------------------
RasterizerStateD3D11::~RasterizerStateD3D11()
{
	SAFE_RELEASE(mRasterizerState);
}

//----------------------------------------------------------------------------
void RasterizerStateD3D11::Bind()
{
	gFBEnv->pEngine->GetRenderer()->SetRasterizerState(this);
}

//----------------------------------------------------------------------------
void RasterizerStateD3D11::SetHardwareRasterizerState(ID3D11RasterizerState* pRasterizerState)
{
	SAFE_RELEASE(mRasterizerState);
	mRasterizerState = pRasterizerState;
}


//----------------------------------------------------------------------------
// BLEND STATE
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
BlendStateD3D11::BlendStateD3D11()
	:mBlendState(0)
{
}

//----------------------------------------------------------------------------
BlendStateD3D11::~BlendStateD3D11()
{
	SAFE_RELEASE(mBlendState);
}

//----------------------------------------------------------------------------
void BlendStateD3D11::Bind()
{
	gFBEnv->pEngine->GetRenderer()->SetBlendState(this);
}

//----------------------------------------------------------------------------
void BlendStateD3D11::SetHardwareBlendState(ID3D11BlendState* pBlendState)
{
	SAFE_RELEASE(mBlendState);
	mBlendState = pBlendState;
}


//----------------------------------------------------------------------------
// DEPTH STENCIL STATE
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
DepthStencilStateD3D11::DepthStencilStateD3D11()
	:mDepthStencilState(0)
{
}

//----------------------------------------------------------------------------
DepthStencilStateD3D11::~DepthStencilStateD3D11()
{
	SAFE_RELEASE(mDepthStencilState);
}

//----------------------------------------------------------------------------
void DepthStencilStateD3D11::Bind(unsigned stencilRef)
{
	gFBEnv->pEngine->GetRenderer()->SetDepthStencilState(this, stencilRef);
}

//----------------------------------------------------------------------------
void DepthStencilStateD3D11::SetHardwareBlendState(ID3D11DepthStencilState* pDepthStencilState)
{
	SAFE_RELEASE(mDepthStencilState);
	mDepthStencilState = pDepthStencilState;
}