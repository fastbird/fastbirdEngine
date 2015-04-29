#include <Engine/StdAfx.h>
#include <Engine/RenderStateD3D11.h>
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
void RasterizerStateD3D11::SetDebugName(const char* name)
{
	if (mRasterizerState)
		mRasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
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
void BlendStateD3D11::SetDebugName(const char* name)
{
	if (mBlendState)
		mBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
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
void DepthStencilStateD3D11::SetDebugName(const char* name)
{
	if (mDepthStencilState)
		mDepthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
}

//----------------------------------------------------------------------------
void DepthStencilStateD3D11::SetHardwareDSState(ID3D11DepthStencilState* pDepthStencilState)
{
	SAFE_RELEASE(mDepthStencilState);
	mDepthStencilState = pDepthStencilState;
}

//----------------------------------------------------------------------------
// Sampler STATE
//----------------------------------------------------------------------------

SamplerStateD3D11::SamplerStateD3D11()
:mSamplerState(0)
{

}
SamplerStateD3D11::~SamplerStateD3D11()
{
	SAFE_RELEASE(mSamplerState);
}

void SamplerStateD3D11::Bind(BINDING_SHADER shader, int slot)
{
	gFBEnv->pEngine->GetRenderer()->SetSamplerState(this, shader, slot);
}

void SamplerStateD3D11::SetDebugName(const char* name)
{
	if (mSamplerState)
		mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
}

void SamplerStateD3D11::SetHardwareSamplerState(ID3D11SamplerState* pSamplerState)
{
	SAFE_RELEASE(mSamplerState);
	mSamplerState = pSamplerState;
}