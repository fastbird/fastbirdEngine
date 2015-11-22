#include <Engine/StdAfx.h>
#include <Engine/UIObject.h>
#include <Engine/GlobalEnv.h>
#include <Engine/Renderer.h>
#include <Engine/IFont.h>
#include <Engine/IConsole.h>
#include <Engine/EngineCommand.h>
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
, mTextSize(22.f)
, mScissor(false)
, mOut(false)
, mAnimNOffset(0, 0)
, mSpecialOrder(0)
, mMultiline(false)
, mDoNotDraw(false) // debugging purpose
, mScale(1, 1)
, mPivot(0, 0)
, mTextOffsetForCursor(0, 0)
, mNeedToUpdatePosVB(true)
, mNeedToUpdateColorVB(false)
, mNeedToUpdateTexcoordVB(false)
{
	SetMaterial("es/materials/UI.material");
	mOwnerUI = 0;
	mTextColor = Color(0.8f, 0.8f, 0.8f);

	mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(0,
		sizeof(Vec4), 4, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
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

	mNeedToUpdateTexcoordVB = true;
}

void UIObject::ClearTexCoord(unsigned index){
	assert(index < 2);
	mTexcoords[index].clear();
	mNeedToUpdateTexcoordVB = true;
}

void UIObject::SetColors(DWORD colors[], DWORD num)
{
	mColors.clear();
	if (colors)
		mColors.assign(colors, colors+num);
	mNeedToUpdateColorVB = true;
}


void UIObject::SetUIPos(const Vec2I& pos){
	if (mUIPos == pos)
		return;

	mUIPos = pos;
	mNeedToUpdatePosVB = true;	
}

const Vec2I& UIObject::GetUIPos() const{
	return mUIPos;
}

void UIObject::SetUISize(const Vec2I& size){
	if (mUISize == size)
		return;

	mUISize = size;
	mNeedToUpdatePosVB = true;	
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

void UIObject::SetTextOffsetForCursorMovement(const Vec2I& offset){
	mTextOffsetForCursor = offset;
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

	if (mNeedToUpdatePosVB)
		UpdateRegion();

	mOut = mScissor && 
		!IsOverlapped(mRegion, mScissorRect);
	if (mOut)
		return;

	if (mMaterial)
	{
		const Vec4& prev = mMaterial->GetMaterialParameters(4);
		mMaterial->SetMaterialParameters(4, Vec4(prev.x, prev.y, prev.z, mAlpha));
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

	mMaterial->Bind(true);
	if (!mNoDrawBackground)
	{
		PrepareVBs();
		
		//renderer->UpdateObjectConstantsBuffer(&mObjectConstants);		
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
		IFont* pFont = renderer->GetFont(mTextSize * mScale.x);
		if (pFont)
		{
			pFont->PrepareRenderResources();
			pFont->SetRenderStates(false, mScissor);			
			float x = float(mRegion.left + mTextOffset.x + mTextOffsetForCursor.x);
			float y = float(mRegion.top + mTextOffset.y + mTextOffsetForCursor.y);
			Color textColor = mTextColor;
			textColor.a() *= mAlpha;
			pFont->Write(x, y, 0.0f, textColor.Get4Byte(), (const char*)mText.c_str(), -1, FONT_ALIGN_LEFT);			
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
	if (mNeedToUpdatePosVB){
		mNeedToUpdatePosVB = false;
		auto mapData = gFBEnv->pRenderer->MapVertexBuffer(mVertexBuffer, 0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (mapData.pData){
			Vec4 positions[4] = {
				Vec4((float)mUIPos.x, (float)(mUIPos.y + mUISize.y), 0.f, 1.0f),
				Vec4((float)mUIPos.x, (float)mUIPos.y, 0.f, 1.0f),
				Vec4((float)(mUIPos.x + mUISize.x), (float)(mUIPos.y + mUISize.y), 0.f, 1.0f),
				Vec4((float)(mUIPos.x + mUISize.x), (float)mUIPos.y, 0.f, 1.0f),
			};
			Mat44 worldMat(2.f / mRenderTargetSize.x, 0, 0, -1.f,
				0.f, -2.f / mRenderTargetSize.y, 0, 1.f,
				0, 0, 1.f, 0.f,
				0, 0, 0, 1.f);
			for (auto& pos : positions){
				pos = worldMat * pos;
			}
			memcpy(mapData.pData, positions, sizeof(Vec4) * 4);
			gFBEnv->pRenderer->UnmapVertexBuffer(mVertexBuffer, 0);
		}
	}

	if (mNeedToUpdateColorVB){
		mNeedToUpdateColorVB = false;

		if (!mColors.empty())
		{
			mVBColor = gFBEnv->pRenderer->CreateVertexBuffer(&mColors[0],
				sizeof(DWORD), mColors.size(), BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

		}
		else
		{
			mVBColor = 0;
		}
	}

	if (mNeedToUpdateTexcoordVB){
		mNeedToUpdateTexcoordVB = false;
		for (int i = 0; i < 2; i++)
		{
			if (!mTexcoords[i].empty())
			{
				if (!mVBTexCoords[i]){
					mVBTexCoords[i] = gFBEnv->pRenderer->CreateVertexBuffer(&mTexcoords[i][0],
						sizeof(Vec2), mTexcoords[i].size(), BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
				}
				else{
					auto map = gFBEnv->pRenderer->MapVertexBuffer(mVBTexCoords[i], 0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
					if (map.pData){
						memcpy(map.pData, &mTexcoords[i][0], sizeof(Vec2)* mTexcoords[i].size());
						gFBEnv->pRenderer->UnmapVertexBuffer(mVBTexCoords[i], 0);
					}
				}				
			}
			else
			{
				mVBTexCoords[i] = 0;
			}
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
	mNeedToUpdatePosVB = true;
}

const Vec2I& UIObject::GetRenderTargetSize() const
{
	return mRenderTargetSize;
}


bool UIObject::HasTexCoord() const{
	return !mTexcoords[0].empty() || !mTexcoords[1].empty();
}

void UIObject::EnableLinearSampler(bool linear){
	if (mMaterial){
		if (!mMaterial->GetAdam())
			mMaterial = mMaterial->Clone();
		if (linear){
			mMaterial->AddShaderDefine("_USE_LINEAR_SAMPLER", "1");
			mMaterial->ApplyShaderDefines();
		}
		else{
			if (mMaterial->RemoveShaderDefine("_USE_LINEAR_SAMPLER"))
				mMaterial->ApplyShaderDefines();
		}
	}
}
}