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
#include "BillboardQuad.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/RenderStates.h"
using namespace fb;
class BillboardQuad::Impl{
public:
	BillboardQuad* mSelf;
	Vec3 mWorldPos;
	Vec2 mSize;
	Vec2 mOffset;
	DWORD mColor;
	MaterialPtr mMaterial;
	RenderStatesPtr mRenderStates;

	//---------------------------------------------------------------------------
	Impl(BillboardQuad* self)
		: mSelf(self)
		, mWorldPos(0, 0, 0)
		, mSize(1, 1)
		, mOffset(0.5f, 0.5f)
	{
		mWorldPos = Vec3(0, 0, 0);
		mSize = Vec2(1, 1);
		mOffset = Vec2(0.5f, 0.5f);
		mSelf->GetBoundingVolumeWorld()->SetAlwaysPass(true);
		mRenderStates = RenderStates::Create();
	}

	void PreRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		if (param.mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;
		mRenderStates->Bind();
		Renderer::GetInstance().DrawBillboardWorldQuad(mWorldPos, mSize, mOffset, mColor, mMaterial);
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}


	void SetMaterial(MaterialPtr mat){
		mMaterial = mat;		
		if (mMaterial){
			mSelf->ModifyObjFlag(SceneObjectFlag::Transparent, mMaterial->IsTransparent());
		}
	}

	MaterialPtr GetMaterial() const{
		return mMaterial;
	}

	void SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color){
		mWorldPos = pos;
		mSelf->GetBoundingVolumeWorld()->SetCenter(pos);
		mSize = size;
		mSelf->GetBoundingVolumeWorld()->SetRadius(size.Length());
		mColor = color.Get4Byte();

		mOffset = offset;
	}

	void SetAlphaBlend(bool blend){
		BLEND_DESC desc;
		if (blend)
		{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;			
			mRenderStates->CreateBlendState(desc);
		}
		else
		{
			mRenderStates->CreateBlendState(desc);
		}
	}

};

FB_IMPLEMENT_STATIC_CREATE(BillboardQuad);
BillboardQuad::BillboardQuad()
	:mImpl(new Impl(this))
{
}
BillboardQuad::~BillboardQuad()
{
}

void BillboardQuad::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void BillboardQuad::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void BillboardQuad::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}

void BillboardQuad::SetMaterial(MaterialPtr mat) {
	mImpl->SetMaterial(mat);
}

MaterialPtr BillboardQuad::GetMaterial() const {
	return mImpl->GetMaterial();
}

void BillboardQuad::SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color) {
	mImpl->SetBillobardData(pos, size, offset, color);
}

void BillboardQuad::SetAlphaBlend(bool blend) {
	mImpl->SetAlphaBlend(blend);
}

