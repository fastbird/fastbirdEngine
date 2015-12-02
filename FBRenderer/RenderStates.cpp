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
#include "RenderStates.h"
#include "Renderer.h"
#include "FBCommonHeaders/CowPtr.h"

namespace fb{

class RasterizerState::Impl{
public:
	IPlatformRasterizerStatePtr mPlatformRasterizerState;
	
	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformRasterizerStatePtr state){
		mPlatformRasterizerState = state;
	}

	void Bind(){
		if (!Renderer::GetInstance().GetForcedWireFrame())
			mPlatformRasterizerState->Bind();
	}

	void SetDebugName(const char* name){
		mPlatformRasterizerState->SetDebugName(name);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(RasterizerState);
RasterizerState::RasterizerState()
	: mImpl(new Impl)
{
}
RasterizerState::~RasterizerState(){

}

void RasterizerState::SetPlatformState(IPlatformRasterizerStatePtr state){
	mImpl->SetPlatformState(state);
}

void RasterizerState::Bind(){
	mImpl->Bind();
}

void RasterizerState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

//---------------------------------------------------------------------------
class BlendState::Impl{
public:
	IPlatformBlendStatePtr mPlatformBlendState;
	static bool Lock;

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformBlendStatePtr state){
		mPlatformBlendState = state;
	}

	void Bind(){
		if (!Lock)
			mPlatformBlendState->Bind();
	}

	void SetDebugName(const char* name){
		mPlatformBlendState->SetDebugName(name);
	}
};
bool BlendState::Impl::Lock = false;

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(BlendState);
BlendState::BlendState()
	: mImpl(new Impl)
{
}

BlendState::~BlendState(){

}

void BlendState::SetLock(bool lock){
	Impl::Lock = lock;
}

void BlendState::SetPlatformState(IPlatformBlendStatePtr state){
	mImpl->SetPlatformState(state);
}

void BlendState::Bind(){
	mImpl->Bind();
}

void BlendState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

//---------------------------------------------------------------------------
class DepthStencilState::Impl{
public:
	IPlatformDepthStencilStatePtr mPlatformDepthStencilState;
	static bool Lock;

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformDepthStencilStatePtr state){
		mPlatformDepthStencilState = state;
	}

	void Bind(unsigned stencilRef){
		if (!Lock)
			mPlatformDepthStencilState->Bind(stencilRef);
	}

	void SetDebugName(const char* name){
		mPlatformDepthStencilState->SetDebugName(name);
	}
};
bool DepthStencilState::Impl::Lock = false;

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(DepthStencilState);
DepthStencilState::DepthStencilState()
	: mImpl(new Impl)
{
}

DepthStencilState::~DepthStencilState(){

}

void DepthStencilState::SetLock(bool lock){
	Impl::Lock = lock;
}

void DepthStencilState::SetPlatformState(IPlatformDepthStencilStatePtr state){
	mImpl->SetPlatformState(state);
}

void DepthStencilState::Bind(){
	mImpl->Bind(0);
}

void DepthStencilState::Bind(unsigned stencilRef){
	mImpl->Bind(stencilRef);
}

void DepthStencilState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

//---------------------------------------------------------------------------
class SamplerState::Impl{
public:
	IPlatformSamplerStatePtr mPlatformSamplerState;	

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformSamplerStatePtr state){
		mPlatformSamplerState = state;
	}

	void Bind(BINDING_SHADER shader, int slot){
		mPlatformSamplerState->Bind(shader, slot);
	}

	void SetDebugName(const char* name){
		mPlatformSamplerState->SetDebugName(name);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(SamplerState);
SamplerState::SamplerState()
	: mImpl(new Impl)
{
}

SamplerState::~SamplerState(){

}

void SamplerState::SetPlatformState(IPlatformSamplerStatePtr state){
	mImpl->SetPlatformState(state);
}

void SamplerState::Bind(BINDING_SHADER shader, int slot){
	mImpl->Bind(shader, slot);
}

void SamplerState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}


//---------------------------------------------------------------------------
class RenderStates::Impl{
public:
	RasterizerStatePtr mRasterizerState;
	BlendStatePtr mBlendState;
	DepthStencilStatePtr mDepthStencilState;

	CowPtr<RASTERIZER_DESC> mRDesc;
	CowPtr<BLEND_DESC> mBDesc;
	CowPtr<DEPTH_STENCIL_DESC> mDDesc;

	static const RASTERIZER_DESC DefaultRDesc;
	static const BLEND_DESC DefaultBDesc;
	static const DEPTH_STENCIL_DESC DefaultDDesc;	

	Impl()
	{
		Reset();
	}

	//-----------------------------------------------------------------------
	void Reset(){
		ResetRasterizerState();
		ResetBlendState();
		ResetDepthStencilState();
	}

	void ResetRasterizerState(){
		mRDesc.reset();
		auto& renderer = Renderer::GetInstance();
		mRasterizerState = renderer.CreateRasterizerState(DefaultRDesc);
	}

	void ResetBlendState(){
		mBDesc.reset();
		auto& renderer = Renderer::GetInstance();
		mBlendState = renderer.CreateBlendState(DefaultBDesc);
	}

	void ResetDepthStencilState(){
		mDDesc.reset();
		auto& renderer = Renderer::GetInstance();
		mDepthStencilState = renderer.CreateDepthStencilState(DefaultDDesc);
	}

	void CreateRasterizerState(const RASTERIZER_DESC& desc){
		auto& renderer = Renderer::GetInstance();
		if (DefaultRDesc == desc){
			if (mRDesc){
				mRDesc.reset();
				mRasterizerState = renderer.CreateRasterizerState(DefaultRDesc);
			}
		}
		else{
			mRDesc = new RASTERIZER_DESC(desc);
			mRasterizerState = renderer.CreateRasterizerState(desc);
		}		
	}

	void CreateBlendState(const BLEND_DESC& desc){
		auto& renderer = Renderer::GetInstance();
		if (DefaultBDesc == desc){
			if (mBDesc){
				mBDesc.reset();
				mBlendState = renderer.CreateBlendState(DefaultBDesc);
			}
		}
		else{
			mBDesc = new BLEND_DESC(desc);
			mBlendState = renderer.CreateBlendState(desc);
		}
	}

	void CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc){
		auto& renderer = Renderer::GetInstance();
		if (DefaultDDesc == desc){
			if (mDDesc){
				mDDesc.reset();
				mDepthStencilState = renderer.CreateDepthStencilState(DefaultDDesc);
			}
		}
		else{
			mDDesc = new DEPTH_STENCIL_DESC(desc);
			mDepthStencilState = renderer.CreateDepthStencilState(desc);
		}		
	}

	void Bind(unsigned stencilRef) const{
		if (mRasterizerState)
			mRasterizerState->Bind();
		if (mBlendState)
			mBlendState->Bind();
		if (mDepthStencilState)
			mDepthStencilState->Bind(stencilRef);
	}
};

const RASTERIZER_DESC RenderStates::Impl::DefaultRDesc;
const BLEND_DESC RenderStates::Impl::DefaultBDesc;
const DEPTH_STENCIL_DESC RenderStates::Impl::DefaultDDesc;

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(RenderStates);

RenderStatesPtr RenderStates::Create(const RenderStates& other){
	return RenderStatesPtr(new RenderStates(other), [](RenderStates* obj){ delete obj; });
}
RenderStates::RenderStates()
	: mImpl(new Impl)
{
}

RenderStates::RenderStates(const RenderStates& other)
	: mImpl(new Impl(*other.mImpl)){

}

RenderStates::~RenderStates(){
}

void RenderStates::Reset(){
	mImpl->Reset();
}
void RenderStates::ResetRasterizerState(){
	mImpl->ResetRasterizerState();
}
void RenderStates::ResetBlendState(){
	mImpl->ResetBlendState();
}
void RenderStates::ResetDepthStencilState(){
	mImpl->ResetDepthStencilState();
}
void RenderStates::CreateRasterizerState(const RASTERIZER_DESC& desc){
	mImpl->CreateRasterizerState(desc);
}
void RenderStates::CreateBlendState(const BLEND_DESC& desc){
	mImpl->CreateBlendState(desc);
}
void RenderStates::CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc){
	mImpl->CreateDepthStencilState(desc);
}

void RenderStates::Bind() const{
	mImpl->Bind(0);
}

void RenderStates::Bind(unsigned stencilRef) const{
	mImpl->Bind(stencilRef);
}

}