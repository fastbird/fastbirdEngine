#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/BillboardQuad.h>

namespace fastbird
{
	IBillboardQuad* IBillboardQuad::CreateBillboardQuad()
	{
		return FB_NEW(BillboardQuad);
	}

	BillboardQuad::BillboardQuad()
	{
		mWorldPos = Vec3(0, 0, 0);
		mSize = Vec2(1, 1);
		mOffset = Vec2(0.5f, 0.5f);
		mBoundingVolumeWorld->SetAlwaysPass(true);
		SetDepthStencilState(DEPTH_STENCIL_DESC());
		SetBlendState(BLEND_DESC());
		SetRasterizerState(RASTERIZER_DESC());
	}

	BillboardQuad::~BillboardQuad()
	{

	}

	void BillboardQuad::SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color)
	{
		mWorldPos = pos;
		mBoundingVolumeWorld->SetCenter(pos);
		mSize = size;
		mBoundingVolumeWorld->SetRadius(size.Length());
		mColor = color.Get4Byte();

		mOffset = offset;
	}

	void BillboardQuad::PreRender()
	{

	}
	void BillboardQuad::Render()
	{
		if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;
		BindRenderStates();
		gFBEnv->pRenderer->DrawBillboardWorldQuad(mWorldPos, mSize, mOffset, mColor, mMaterial);
	}
	void BillboardQuad::PostRender()
	{

	}

	void BillboardQuad::SetMaterial(IMaterial* mat, int pass /*= RENDER_PASS::PASS_NORMAL*/)
	{
		mMaterial = mat;
	}

	IMaterial* BillboardQuad::GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
	{
		return mMaterial;
	}

	void BillboardQuad::SetAlphaBlend(bool blend)
	{
		BLEND_DESC desc;
		if (blend)
		{
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = BLEND_ONE;
			//desc.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
			//desc.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
			SetBlendState(desc);
		}
		else
		{
			SetBlendState(desc);
		}
		
	}
}