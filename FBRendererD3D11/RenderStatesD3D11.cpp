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
#include "RenderStatesD3D11.h"
#include "IUnknownDeleter.h"
#include "RendererD3D11.h"

using namespace fb;

//----------------------------------------------------------------------------
// RASTERIZER STATE
//-------------------------------------------------------------------------
RasterizerStateD3D11Ptr RasterizerStateD3D11::Create(ID3D11RasterizerState* rasterizerState){
	return RasterizerStateD3D11Ptr(new RasterizerStateD3D11(rasterizerState), [](RasterizerStateD3D11* obj){ delete obj; });
}

RasterizerStateD3D11::RasterizerStateD3D11(ID3D11RasterizerState* rasterizerState)
	: mRasterizerState(rasterizerState, IUnknownDeleter())
{
}

//----------------------------------------------------------------------------
void RasterizerStateD3D11::Bind() {
	RendererD3D11::GetInstance().SetRasterizerState(this);
}

//----------------------------------------------------------------------------
void RasterizerStateD3D11::SetDebugName(const char* name) {
	if (mRasterizerState)
	{
		mRasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		mRasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

//----------------------------------------------------------------------------
ID3D11RasterizerState* RasterizerStateD3D11::GetHardwareRasterizerState() const {
	return mRasterizerState.get();
}


//----------------------------------------------------------------------------
// BLEND STATE
//----------------------------------------------------------------------------
BlendStateD3D11Ptr BlendStateD3D11::Create(ID3D11BlendState* blendState){
	return BlendStateD3D11Ptr(new BlendStateD3D11(blendState), [](BlendStateD3D11* obj){ delete obj; });
}
BlendStateD3D11::BlendStateD3D11(ID3D11BlendState* blendState)
	:mBlendState(blendState, IUnknownDeleter())
{
}

void BlendStateD3D11::Bind() {
	RendererD3D11::GetInstance().SetBlendState(this);
}

void BlendStateD3D11::SetDebugName(const char* name) {
	if (mBlendState)
	{
		mBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		mBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

ID3D11BlendState* BlendStateD3D11::GetHardwareBlendState() const{
	return mBlendState.get();
}


//----------------------------------------------------------------------------
// DEPTH STENCIL STATE
//----------------------------------------------------------------------------
DepthStencilStateD3D11Ptr DepthStencilStateD3D11::Create(ID3D11DepthStencilState* state){
	return DepthStencilStateD3D11Ptr(new DepthStencilStateD3D11(state), [](DepthStencilStateD3D11* obj){ delete obj; });
}
DepthStencilStateD3D11::DepthStencilStateD3D11(ID3D11DepthStencilState* depthStencilState)
	:mDepthStencilState(depthStencilState, IUnknownDeleter())
{
}

void DepthStencilStateD3D11::Bind(int stencilRef) {
	RendererD3D11::GetInstance().SetDepthStencilState(this, stencilRef);
}

void DepthStencilStateD3D11::SetDebugName(const char* name) {
	if (mDepthStencilState) {
		mDepthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		mDepthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

ID3D11DepthStencilState* DepthStencilStateD3D11::GetHardwareDSState() const{
	return mDepthStencilState.get();
}

//----------------------------------------------------------------------------
// Sampler STATE
//----------------------------------------------------------------------------
SamplerStateD3D11Ptr SamplerStateD3D11::Create(ID3D11SamplerState* state){
	return SamplerStateD3D11Ptr(new SamplerStateD3D11(state), [](SamplerStateD3D11* obj){ delete obj; });
}

SamplerStateD3D11::SamplerStateD3D11(ID3D11SamplerState* samplerState)
	:mSamplerState(samplerState, IUnknownDeleter())
{
}

void SamplerStateD3D11::Bind(SHADER_TYPE shader, int slot) {
	RendererD3D11::GetInstance().SetSamplerState(this, shader, slot);
}

void SamplerStateD3D11::SetDebugName(const char* name) {
	if (mSamplerState)
	{
		mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		mSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

ID3D11SamplerState* SamplerStateD3D11::GetHardwareSamplerState() const {
	return mSamplerState.get();
}