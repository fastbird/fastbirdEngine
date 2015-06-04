#include <Engine/StdAfx.h>
#include <Engine/UIObject.h>
#include <Engine/GlobalEnv.h>
#include <Engine/Renderer.h>
#include <Engine/IFont.h>
#include <Engine/IConsole.h>
#include <UI/ComponentType.h>


namespace fastbird
{

SmartPtr<IRasterizerState> UIObject::mRasterizerStateShared;

IUIObject* IUIObject::CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize)
{
	static unsigned uinum = 0;
	IUIObject* p = FB_NEW(UIObject);
	if (!usingSmartPtr)
		p->AddRef();
	p->SetRenderTargetSize(renderTargetSize);
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
, mTextOffset(0, 22)
, mNOffset(0, 0.0f)
, mUIPos(0, 0)
, mUISize(100, 100)
, mDebugString("UIObject")
, mNoDrawBackground(false)
, mDirty(true)
, mTextSize(22.f)
, mScissor(false)
, mOut(false)
, mAnimNOffset(0, 0)
, mSpecialOrder(0)
, mMultiline(false)
, mDoNotDraw(false) // debugging purpose
, mScale(1, 1)
, mPivot(0, 0)
{
	mObjectConstants.gWorld.MakeIdentity();
	mObjectConstants.gWorldViewProj.MakeIdentity();
	SetMaterial("es/materials/UI.material");
	mOwnerUI = 0;
	mTextColor = Color(0.8f, 0.8f, 0.8f);
}
UIObject::~UIObject()
{
}

void UIObject::SetTexCoord(Vec2 coord[], DWORD num, unsigned index)
{
	if_assert_fail(index < 2)
		return;
	mTexcoords[index].clear();
	if (coord)
		mTexcoords[index].assign(coord, coord + num);

	mDirty = true;
}

void UIObject::ClearTexCoord(unsigned index){
	assert(index < 2);
	mTexcoords[index].clear();
	mDirty = true;
}

void UIObject::SetColors(DWORD colors[], DWORD num)
{
	mColors.clear();
	if (colors)
		mColors.assign(colors, colors+num);
	mDirty = true;
}


void UIObject::SetUIPos(const Vec2I& pos){
	if (mUIPos == pos)
		return;

	mUIPos = pos;
	mDirty = true;

}

const Vec2I& UIObject::GetUIPos() const{
	return mUIPos;
}

void UIObject::SetUISize(const Vec2I& size){
	if (mUISize == size)
		return;

	mUISize = size;
	mDirty = true;
}

const Vec2I& UIObject::GetUISize() const{
	return mUISize;
}


void UIObject::UpdateRegion()
{
	mRegion.left = mUIPos.x;
	mRegion.top = mUIPos.y;
	mRegion.right = mUIPos.x + mUISize.x;
	mRegion.bottom = mUIPos.y + mUISize.y;

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

void UIObject::SetTextOffset(const Vec2I& offset){
	mTextOffset = offset;
}

Vec2I UIObject::GetTextStartWPos() const{
	return Vec2I(mRegion.left, mRegion.top) + mTextOffset;
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

	if (mVBTexCoords[0])
	{
		Vec2 uvStep = mTexcoords[0][2] - mTexcoords[0][1];
		Vec4 val(uvStep.x, uvStep.y, mTexcoords[0][1].x, mTexcoords[0][1].y);
		mMaterial->SetMaterialParameters(0, val);
	}
}


//----------------------------------------------------------------------------
void UIObject::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE || mDoNotDraw)
		return;

	if (mLastPreRendered == gFBEnv->mFrameCounter)
		return;
	mLastPreRendered = gFBEnv->mFrameCounter;

	mOut = mScissor && 
		!IsOverlapped(mRegion, mScissorRect);
	if (mOut)
		return;

	if (mMaterial)
	{
		const Vec4& prev = mMaterial->GetMaterialParameters(4);
		mMaterial->SetMaterialParameters(4, Vec4(prev.x, prev.y, prev.z, mAlpha));
	}

	if (mDirty)
	{
		PrepareVBs();
		mDirty = false;
	}
}

void UIObject::Render()
{
	if (gFBEnv->pConsole->GetEngineCommand()->r_UI == 0 || mDoNotDraw)
		return;
	D3DEventMarker mark(mDebugString.c_str());

	if (mObjFlag & IObject::OF_HIDE)
		return;

	if (!mMaterial || !mVertexBuffer || mOut)
		return;

	auto const renderer = gFBEnv->_pInternalRenderer;

	if (mScissor)
	{
		renderer->SetScissorRects(&mScissorRect, 1);
	}

	if (!mNoDrawBackground)
	{
		renderer->UpdateObjectConstantsBuffer(&mObjectConstants);
		mMaterial->Bind(true);
		//mRasterizerStateShared->Bind();
		renderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
		const unsigned int numBuffers = 4;
		IVertexBuffer* buffers[numBuffers] = { mVertexBuffer, mVBColor, mVBTexCoords[0], mVBTexCoords[1] };
		unsigned int strides[numBuffers] = { mVertexBuffer->GetStride(),
			mVBColor ? mVBColor->GetStride() : 0,
			mVBTexCoords[0] ? mVBTexCoords[0]->GetStride() : 0,
			mVBTexCoords[1] ? mVBTexCoords[1]->GetStride() : 0
		};
		unsigned int offsets[numBuffers] = { 0, 0, 0, 0 };
		renderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);
		
		renderer->Draw(mVertexBuffer->GetNumVertices(), 0);
	}

	if (!mText.empty())
	{
		IFont* pFont = renderer->GetFont();
		if (pFont)
		{
			pFont->PrepareRenderResources();
			pFont->SetRenderStates(false, mScissor);
			pFont->SetHeight(mTextSize * mScale.x);
			float x = float(mRegion.left + mTextOffset.x);
			float y = float(mRegion.top + mTextOffset.y);
			Color textColor = mTextColor;
			textColor.a() *= mAlpha;
			pFont->Write(x, y, 0.0f, textColor.Get4Byte(), (const char*)mText.c_str(), -1, FONT_ALIGN_LEFT);
			pFont->SetBackToOrigHeight();
		}
	}

	/*if (gFBEnv->pConsole->GetEngineCommand()->UI_Debug)
	{
		if (gFBEnv->pEngine->GetMouse()->IsIn(mRegion)){
			IFont* pFont = renderer->GetFont();
			if (pFont)
			{
				pFont->PrepareRenderResources();
				pFont->SetRenderStates();
				pFont->SetHeight(30.0f);
				std::wstringstream ss;
				if (mTypeString)
					ss << L"UI Type: " << AnsiToWide(mTypeString, strlen(mTypeString)) << ", ";
				ss << mUIPos.x << "," << mUIPos.y;

				pFont->Write((float)(mRegion.left + (mRegion.right - mRegion.left) / 2),
					(float)(mRegion.top + (mRegion.bottom - mRegion.top) / 2),
					0.f, Color::Blue.Get4Byte(), (const char*)ss.str().c_str(), -1, FONT_ALIGN_LEFT);
				pFont->SetBackToOrigHeight();
			}
		}
	}*/
}

void UIObject::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
}

void UIObject::PrepareVBs()
{
	mPositions.clear();
	mPositions.reserve(4);
	mPositions.push_back(Vec3((float)mUIPos.x, (float)(mUIPos.y + mUISize.y), 0.f));
	mPositions.push_back(Vec3(Vec2(mUIPos), 0.f));
	mPositions.push_back(Vec3((float)(mUIPos.x + mUISize.x), (float)(mUIPos.y + mUISize.y), 0.f));
	mPositions.push_back(Vec3((float)(mUIPos.x + mUISize.x), (float)mUIPos.y, 0.f));	
	

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

	for (int i = 0; i < 2; i++)
	{
		if (!mTexcoords[i].empty())
		{
			mVBTexCoords[i] = gFBEnv->pRenderer->CreateVertexBuffer(&mTexcoords[i][0],
				sizeof(Vec2), mTexcoords[i].size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
		}
		else
		{
			mVBTexCoords[i] = 0;
		}
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
	

	if (!mMaterial)
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Doesn't have material. Cannot change scissor mode.");
		return;
	}
		
	if (mScissor)
	{
		mScissorRect = rect;
		RASTERIZER_DESC rd;
		rd.ScissorEnable = true;
		mMaterial->CloneRenderStates();
		mMaterial->SetRasterizerState(rd);
	}
	else
	{
		mMaterial->ClearRasterizerState();
	}
	UpdateRegion();
}

void UIObject::SetDoNotDraw(bool doNotDraw)
{
	mDoNotDraw = doNotDraw;
}

void UIObject::SetRenderTargetSize(const Vec2I& rtSize)
{
	mRenderTargetSize = rtSize;
	mObjectConstants = {
		Mat44(2.f / rtSize.x, 0, 0, -1.f,
		0.f, -2.f / rtSize.y, 0, 1.f,
		0, 0, 1.f, 0.f,
		0, 0, 0, 1.f),
		Mat44(),
		Mat44(),
	};
}

const Vec2I& UIObject::GetRenderTargetSize() const
{
	return mRenderTargetSize;
}


bool UIObject::HasTexCoord() const{
	return !mTexcoords[0].empty() || !mTexcoords[1].empty();
}

}