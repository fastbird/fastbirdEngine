#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/UIObject.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/IFont.h>
#include <Engine/IConsole.h>
#include <UI/ComponentType.h>


namespace fastbird
{

SmartPtr<IRasterizerState> UIObject::mRasterizerStateShared;

IUIObject* IUIObject::CreateUIObject(bool usingSmartPtr)
{
	static unsigned uinum = 0;
	IUIObject* p = FB_NEW(UIObject);
	if (!usingSmartPtr)
		p->AddRef();
	//p->SetDebugNumber(uinum++);
	return p;
}

// only not using smart ptr
void IUIObject::Delete()
{
	this->Release();
}

void UIObject::ClearSharedRS()
{
	mRasterizerStateShared = 0;
}

//---------------------------------------------------------------------------
UIObject::UIObject()
	: mAlpha(1.f)
	, mNDCPos(0, 0)
	, mNDCOffset(0, 0.0)
	, mTextNPos(0, 0)
	, mNOffset(0, 0.0f)
	, mNPos(0.5f, 0.5f)
	, mNSize(0.1f, 0.1f)
	, mDebugString("UIObject")
	, mNoDrawBackground(false)
	, mDirty(true)
	, mTextSize(30.0f)
	, mScissor(false)
	, mOut(false)
	, mAlphaBlending(false)
	, mAnimNDCOffset(0, 0)
	, mAnimNOffset(0, 0)
	, mSpecialOrder(0)
	, mMultiline(false)
{
	mObjectConstants.gWorld.MakeIdentity();
	mObjectConstants.gWorldViewProj.MakeIdentity();
	SetMaterial("es/materials/UI.material");
	mOwnerUI = 0;
	mTextColor = Color::White;

	BLEND_DESC bdesc;
	bdesc.RenderTarget[0].BlendEnable = true;
	bdesc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
	bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
	bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
	SetBlendState(bdesc);
	DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = false;
	desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	SetDepthStencilState(desc);
}
UIObject::~UIObject()
{
}

//---------------------------------------------------------------------------
void UIObject::SetVertices(const Vec3* ndcPoints, int num,
			const DWORD* colors/*=0*/,
			const Vec2* texcoords/*=0*/)
{
	mPositions.clear();
	mPositions.assign(ndcPoints, ndcPoints+num);

	mColors.clear();
	if (colors)
		mColors.assign(colors, colors+num);
	mTexcoords.clear();
	if (texcoords)
	{
		assert(num>=4);
		mTexcoords.assign(texcoords, texcoords+num);
		Vec2 step = texcoords[2] - texcoords[1];
		Vec4 val(step.x, step.y, texcoords[1].x, texcoords[1].y);
		mMaterial->SetMaterialParameters(0, val);
	}

	mDirty = true;
}

void UIObject::SetNSize(const Vec2& size) // in normalized space
{
	mNSize = size;
	UpdateRegion();	
	mDirty = true;
	Vec2 ndcSize = size * 2.f;
	// 1 3  
	// 0 2
	Vec3 positions[4] = 
	{
		Vec3(0.f, 0.0f - ndcSize.y, 0.f),
		Vec3(0.f, 0.0f, 0.f),
		Vec3(0.f+ndcSize.x, 0.0f - ndcSize.y, 0.f),
		Vec3(0.f+ndcSize.x, 0.0f, 0)
	};
	mPositions.assign(&positions[0], positions+4);
	mDirty = true;
}

void UIObject::SetNPos(const Vec2& npos)
{
	mNPos = npos;
	mNDCPos.x = npos.x*2.0f - 1.f;
	mNDCPos.y = -npos.y*2.0f + 1.f;

	UpdateRegion();
}

void UIObject::SetNPosOffset(const Vec2& nposOffset)
{
	mNOffset = nposOffset;
	mNDCOffset.x = nposOffset.x*2.f;
	mNDCOffset.y = -nposOffset.y*2.f;
	UpdateRegion();
}

void UIObject::SetAnimNPosOffset(const Vec2& nposOffset)
{
	mAnimNOffset = nposOffset;
	mAnimNDCOffset.x = nposOffset.x*2.f;
	mAnimNDCOffset.y = -nposOffset.y*2.f;
	UpdateRegion();
}

void UIObject::SetTexCoord(Vec2 coord[], DWORD num)
{
	mTexcoords.clear();
	if (coord)
		mTexcoords.assign(coord, coord+num);

	mDirty = true;
}

void UIObject::SetColors(DWORD colors[], DWORD num)
{
	mColors.clear();
	if (colors)
		mColors.assign(colors, colors+num);
	mDirty = true;
}

void UIObject::UpdateRegion()
{
	mRegion.left = Round((mNPos.x + mNOffset.x + mAnimNOffset.x) * gFBEnv->pRenderer->GetWidth());
	mRegion.top = Round((mNPos.y + mNOffset.y + mAnimNOffset.y) * gFBEnv->pRenderer->GetHeight());
	mRegion.right = mRegion.left + Round(mNSize.x * gFBEnv->pRenderer->GetWidth());
	mRegion.bottom = mRegion.top + Round(mNSize.y * gFBEnv->pRenderer->GetHeight());

	// ratio
	Vec4 val((mRegion.right - mRegion.left) / (float)(mRegion.bottom - mRegion.top), 
		// width, height
		(float)(mRegion.right - mRegion.left), (float)(mRegion.bottom - mRegion.top), 
		// empty
		0);
	mMaterial->SetMaterialParameters(0, val);
}

void UIObject::SetAlpha(float alpha)
{
	mAlpha = alpha;
}

void UIObject::SetText(const wchar_t* s)
{
	mText = s;
}

// 0~1
void UIObject::SetTextStartNPos(const Vec2& npos)
{
	mTextNPos = npos;
}

//---------------------------------------------------------------------------
void UIObject::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
{
	mMaterial = fastbird::IMaterial::CreateMaterial(name);
	assert(mMaterial);

	if (!mRasterizerStateShared)
	{
		RASTERIZER_DESC desc;
		mRasterizerStateShared = gFBEnv->pRenderer->CreateRasterizerState(desc);
	}

	if (mVBTexCoord)
	{
		Vec2 uvStep = mTexcoords[2] - mTexcoords[1];
		Vec4 val(uvStep.x, uvStep.y, mTexcoords[1].x, mTexcoords[1].y);
		mMaterial->SetMaterialParameters(0, val);
	}
}


//----------------------------------------------------------------------------
void UIObject::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	mOut = mScissor && 
		!IsOverlapped(mRegion, mScissorRect);
	if (mOut)
		return;

	mObjectConstants.gWorld.SetTranslation( Vec3(mNDCPos+mNDCOffset+mAnimNDCOffset, 0.f ));
	mObjectConstants.gWorld[0][0] = mAlpha;

	if (mDirty)
	{
		PrepareVBs();
		mDirty = false;
	}
}

void UIObject::Render()
{
	if (gFBEnv->pConsole->GetEngineCommand()->r_UI == 0)
		return;
	D3DEventMarker mark(mDebugString.c_str());

	if (mObjFlag & IObject::OF_HIDE)
		return;

	if (!mMaterial || !mVertexBuffer || mOut)
		return;

	if (gFBEnv->pConsole->GetEngineCommand()->UI_Debug)
	{
		IFont* pFont = gFBEnv->pRenderer->GetFont();
		if (pFont)
		{
			pFont->PrepareRenderResources();
			pFont->SetDefaultConstants();
			pFont->SetRenderStates();
			pFont->SetHeight(30.0f);
			std::wstringstream ss;
			if (mTypeString)
				ss << L"UI Type: " << AnsiToWide(mTypeString, strlen(mTypeString)) << ", ";
			ss << mNPos.x << "," << mNPos.y;
			pFont->Write((float)(mRegion.left + (mRegion.right - mRegion.left)/2), 
				(float)(mRegion.top + (mRegion.bottom - mRegion.top)/2),
				0.f, Color::Blue.Get4Byte(), (const char*)ss.str().c_str(), -1, FONT_ALIGN_LEFT);
			pFont->SetBackToOrigHeight();
		}
	}

	if (mScissor)
	{
		gFBEnv->pRenderer->SetScissorRects(&mScissorRect, 1);
	}

	if (!mNoDrawBackground)
	{
		gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);
		mMaterial->Bind(true);
		mRasterizerStateShared->Bind();
		gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		BindRenderStates();
	
		const unsigned int numBuffers = 3;
		IVertexBuffer* buffers[numBuffers] = {mVertexBuffer, mVBColor, mVBTexCoord};
		unsigned int strides[numBuffers] = {mVertexBuffer->GetStride(), 
			mVBColor ? mVBColor->GetStride() : 0,
			mVBTexCoord ? mVBTexCoord->GetStride(): 0};
		unsigned int offsets[numBuffers] = {0, 0, 0};
		gFBEnv->pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
		gFBEnv->pRenderer->Draw(mVertexBuffer->GetNumVertices(), 0);
	}

	if (!mText.empty())
	{
		IFont* pFont = gFBEnv->pRenderer->GetFont();
		if (pFont)
		{
			gFBEnv->pRenderer->SetAlphaBlendState();
			pFont->PrepareRenderResources();
			pFont->SetDefaultConstants();
			pFont->SetRenderStates(false, mScissor);
			pFont->SetHeight(mTextSize);
			pFont->Write(
				(mTextNPos.x + mNOffset.x + mAnimNOffset.x) * gFBEnv->pRenderer->GetWidth(),
				(mTextNPos.y + mNOffset.y + mAnimNOffset.y) * gFBEnv->pRenderer->GetHeight(),
				0.0f, mTextColor.Get4Byte(), (const char*)mText.c_str(), -1, FONT_ALIGN_LEFT);
			pFont->SetBackToOrigHeight();
		}
	}

	if (mScissor)
	{
		gFBEnv->pRenderer->RestoreScissorRects();
	}
}

void UIObject::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
}

void UIObject::PrepareVBs()
{
	if (!mPositions.empty())
	{
		mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(&mPositions[0],
			sizeof(Vec3), mPositions.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	}
	if (!mColors.empty())
	{
		mVBColor = gFBEnv->pRenderer->CreateVertexBuffer(&mColors[0],
			sizeof(DWORD), mColors.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);

	}
	else
	{
		mVBColor = 0;
	}

	if (!mTexcoords.empty())
	{
		mVBTexCoord = gFBEnv->pRenderer->CreateVertexBuffer(&mTexcoords[0],
			sizeof(Vec2), mTexcoords.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	}
	else
	{
		mVBTexCoord = 0;
	}
}

void UIObject::SetTextColor(const Color& c)
{
	mTextColor = c;
}

void UIObject::SetTextSize(float size)
{
	mTextSize = size;
}

const RECT& UIObject::GetRegion() const
{

	return mRegion;
}

void UIObject::SetDebugString(const char* string)
{
	mDebugString = string;
}

void UIObject::SetNoDrawBackground(bool flag)
{
	mNoDrawBackground = flag;
}

void UIObject::SetUseScissor(bool use, const RECT& rect)
{
	mScissor = use;
	if (mScissor)
	{
		mScissorRect = rect;
		if (!mRasterizerState)
		{
			RASTERIZER_DESC rd;
			rd.ScissorEnable = true;
			mRasterizerState = gFBEnv->pRenderer->CreateRasterizerState(rd);
		}
	}
	else
	{
		mRasterizerState = 0;
	}
}

void UIObject::SetAlphaBlending(bool set)
{
	/*
	mAlphaBlending = set;
	if (set)
	{
		BLEND_DESC desc;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
		SetBlendState(desc);

		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthEnable = false;
		SetDepthStencilState(ddesc);
	}
	else
	{
		SetBlendState(BLEND_DESC());
		SetDepthStencilState(DEPTH_STENCIL_DESC());
	}
	*/
}

bool UIObject::GetAlphaBlending() const
{
	return mAlphaBlending;
}
}