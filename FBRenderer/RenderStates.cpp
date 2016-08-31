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
static RasterizerStateWeakPtr sCurrentRasterizerState;
class RasterizerState::Impl{
public:
	IPlatformRasterizerStatePtr mPlatformRasterizerState;
	RasterizerStateWeakPtr mSelfPtr;
	static bool sLock;
	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformRasterizerStatePtr state){
		mPlatformRasterizerState = state;
	}

	void Bind(){
		if (!sLock) {
			mPlatformRasterizerState->Bind();
			sCurrentRasterizerState = mSelfPtr;
		}
	}

	void SetDebugName(const char* name){
		mPlatformRasterizerState->SetDebugName(name);
	}
};

bool RasterizerState::Impl::sLock = false;
//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE_SELF_PTR(RasterizerState);
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

void RasterizerState::SetLock(bool lock)
{
	Impl::sLock = lock;
}

RasterizerStatePtr RasterizerState::GetCurrentState() {
	return sCurrentRasterizerState.lock();
}

BlendStateWeakPtr sCurrentBlendState;
//---------------------------------------------------------------------------
class BlendState::Impl{
public:
	IPlatformBlendStatePtr mPlatformBlendState;
	BlendStateWeakPtr mSelfPtr;
	static bool Lock;

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformBlendStatePtr state){
		mPlatformBlendState = state;
	}

	void Bind(){
		if (!Lock) {
			mPlatformBlendState->Bind();
			sCurrentBlendState = mSelfPtr;
		}
	}

	void SetDebugName(const char* name){
		mPlatformBlendState->SetDebugName(name);
	}
};
bool BlendState::Impl::Lock = false;

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE_SELF_PTR(BlendState);
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

BlendStatePtr BlendState::GetCurrentState() {
	return sCurrentBlendState.lock();
}

static DepthStencilStateWeakPtr sCurrentDepthStencil;
//---------------------------------------------------------------------------
class DepthStencilState::Impl{
public:
	IPlatformDepthStencilStatePtr mPlatformDepthStencilState;
	DepthStencilStateWeakPtr mSelfPtr;
	static bool Lock;

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformDepthStencilStatePtr state){
		mPlatformDepthStencilState = state;
	}

	void Bind(int stencilRef){
		if (!Lock) {
			mPlatformDepthStencilState->Bind(stencilRef);
			sCurrentDepthStencil = mSelfPtr;
		}
	}

	void SetDebugName(const char* name){
		mPlatformDepthStencilState->SetDebugName(name);
	}
};
bool DepthStencilState::Impl::Lock = false;

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE_SELF_PTR(DepthStencilState);
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

void DepthStencilState::Bind(int stencilRef){
	mImpl->Bind(stencilRef);
}

void DepthStencilState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

DepthStencilStatePtr DepthStencilState::GetCurrentState() {
	return sCurrentDepthStencil.lock();
}

//---------------------------------------------------------------------------
class SamplerState::Impl{
public:
	IPlatformSamplerStatePtr mPlatformSamplerState;	

	//---------------------------------------------------------------------------
	void SetPlatformState(IPlatformSamplerStatePtr state){
		mPlatformSamplerState = state;
	}

	void Bind(SHADER_TYPE shader, int slot){
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

void SamplerState::Bind(SHADER_TYPE shader, int slot){
	mImpl->Bind(shader, slot);
}

void SamplerState::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

bool sForceIncrementalStencil = false;
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

	bool operator==(const Impl& other) const {
		return !operator!=(other);
	}

	bool operator!=(const Impl& other) const {
		return *mRDesc.const_get() != *other.mRDesc.const_get() ||
			*mBDesc.const_get() != *other.mBDesc.const_get() ||
			*mDDesc.const_get() != *other.mDDesc.const_get();
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

	void Bind(int stencilRef) const{
		mRasterizerState->Bind();
		mBlendState->Bind();	
		if (sForceIncrementalStencil) {
			Renderer::GetInstance().BindIncrementalStencilState(2);
		}
		else {
			mDepthStencilState->Bind(stencilRef);
		}
	}

	void DebugPrint() const{
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) RasterizerStates : 0x%x", mRasterizerState.get()).c_str());
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) BlendStates : 0x%x", mBlendState.get()).c_str());
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) DepthStencilStates : 0x%x", mDepthStencilState.get()).c_str());

		if (mRDesc){
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) RDesc scissor: %d", mRDesc->GetScissorEnable() ? 1 : 0).c_str());
		}
		else{
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) RDesc null").c_str());
		}

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

RenderStatesPtr RenderStates::Create(
	const RasterizerStatePtr& rasterizer,
	const BlendStatePtr& blend,
	const DepthStencilStatePtr& depth)
{
	return RenderStatesPtr(new RenderStates(rasterizer, blend, depth),
		[](RenderStates* obj) {delete obj; });
}

void RenderStates::SetForceIncrementalStencilState(bool set) {
	sForceIncrementalStencil = set;
}

RenderStates::RenderStates()
	: mImpl(new Impl)
{
}

RenderStates::RenderStates(const RenderStates& other)
	: mImpl(new Impl(*other.mImpl)){

}

RenderStates::RenderStates(
	const RasterizerStatePtr& rasterizer,
	const BlendStatePtr& blend,
	const DepthStencilStatePtr& depth)
	: mImpl(new Impl)
{
	mImpl->mRasterizerState = rasterizer;
	mImpl->mBlendState = blend;
	mImpl->mDepthStencilState = depth;
}

RenderStates::~RenderStates(){
}

bool RenderStates::operator==(const RenderStates& other) const {
	return mImpl == other.mImpl;
}

bool RenderStates::operator!=(const RenderStates& other) const {
	return mImpl != other.mImpl;
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

void RenderStates::Bind(int stencilRef) const{
	mImpl->Bind(stencilRef);
}

void RenderStates::DebugPrint() const{
	mImpl->DebugPrint();
}

}