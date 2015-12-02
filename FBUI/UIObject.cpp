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

#include "StdAfx.h"
#include "UIObject.h"
#include "ComponentType.h"

#include "FBRenderer/VertexBuffer.h"
using namespace fb;
class UIObject::Impl{
public:	
	WinBase* mOwnerUI;
	MaterialPtr mMaterial;
	VertexBufferPtr mVertexBuffer;

	std::wstring mText;
	Vec2 mNDCPos; // ndc pos
	Vec2 mNDCOffset;
	Vec2 mNOffset;
	Vec2 mAnimNDCOffset;
	Vec2 mAnimNOffset;
	Vec2 mScale;
	Vec2I mTextOffset;
	Vec2I mTextOffsetForCursor;
	Color mTextColor;
	float mTextSize;
	float mAlpha;
	Rect mRegion;
	Vec2I mUISize;
	Vec2I mUIPos;
	std::string mDebugString;
	bool mNoDrawBackground;
	std::vector<DWORD> mColors;
	std::vector<Vec2> mTexcoords[2];
	VertexBufferPtr mVBColor;
	VertexBufferPtr mVBTexCoords[2];
	Rect mScissorRect;
	int mSpecialOrder;
	bool mScissor;
	bool mOut;
	bool mMultiline;
	bool mNeedToUpdatePosVB;
	bool mNeedToUpdateColorVB;
	bool mNeedToUpdateTexcoordVB;
	bool mDoNotDraw;
	Vec2 mPivot;
	Vec2I mRenderTargetSize;
	unsigned mLastPreRendered;

	//---------------------------------------------------------------------------
	Impl()
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
		SetMaterial("EssentialEngineData/materials/UI.material");
		mOwnerUI = 0;
		mTextColor = Color(0.8f, 0.8f, 0.8f);

		mVertexBuffer = Renderer::GetInstance().CreateVertexBuffer(0,
			sizeof(Vec4f), 4, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	}
	void SetTexCoord(Vec2 coord[], DWORD num, unsigned index)
	{
		if_assert_fail(index < 2)
			return;
		mTexcoords[index].clear();
		if (coord)
			mTexcoords[index].assign(coord, coord + num);

		mNeedToUpdateTexcoordVB = true;
	}

	void ClearTexCoord(unsigned index){
		assert(index < 2);
		mTexcoords[index].clear();
		mNeedToUpdateTexcoordVB = true;
	}

	void SetColors(DWORD colors[], DWORD num)
	{
		mColors.clear();
		if (colors)
			mColors.assign(colors, colors + num);
		mNeedToUpdateColorVB = true;
	}


	void SetUIPos(const Vec2I& pos){
		if (mUIPos == pos)
			return;

		mUIPos = pos;
		mNeedToUpdatePosVB = true;
	}

	const Vec2I& GetUIPos() const{
		return mUIPos;
	}

	void SetUISize(const Vec2I& size){
		if (mUISize == size)
			return;

		mUISize = size;
		mNeedToUpdatePosVB = true;
	}

	const Vec2I& GetUISize() const{
		return mUISize;
	}


	void UpdateRegion()
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
		mMaterial->SetMaterialParameter(0, val);
	}

	void SetAlpha(float alpha)
	{
		mAlpha = alpha;
	}

	void SetText(const wchar_t* s)
	{
		mText = s;
	}

	void SetTextOffset(const Vec2I& offset){
		mTextOffset = offset;
	}
	
	const Vec2I& GetTextOffset() const { 
		return mTextOffset; 
	}

	void SetTextOffsetForCursorMovement(const Vec2I& offset){
		mTextOffsetForCursor = offset;
	}

	Vec2I GetTextStartWPos() const{
		return Vec2I(mRegion.left, mRegion.top) + mTextOffset;
	}

	//---------------------------------------------------------------------------
	void SetMaterial(const char* name)
	{
		mMaterial = Renderer::GetInstance().CreateMaterial(name);			
		assert(mMaterial);

		if (mVBTexCoords[0])
		{
			Vec2 uvStep = mTexcoords[0][2] - mTexcoords[0][1];
			Vec4 val(uvStep.x, uvStep.y, mTexcoords[0][1].x, mTexcoords[0][1].y);
			mMaterial->SetMaterialParameter(0, val);
		}
	}
	
	MaterialPtr GetMaterial() const{
		return mMaterial;
	}


	//----------------------------------------------------------------------------
	void PreRender()
	{
		if (mDoNotDraw)
			return;

		if (mLastPreRendered == gpTimer->GetFrame())
			return;
		mLastPreRendered = gpTimer->GetFrame();

		if (mNeedToUpdatePosVB)
			UpdateRegion();

		mOut = mScissor &&
			!IsOverlapped(mRegion, mScissorRect);
		if (mOut)
			return;

		if (mMaterial)
		{
			const Vec4& prev = mMaterial->GetMaterialParameter(4);
			mMaterial->SetMaterialParameter(4, Vec4(prev.x, prev.y, prev.z, mAlpha));
		}
	}

	void Render()
	{
		if (mDoNotDraw)
			return;

		RenderEventMarker mark(mDebugString.c_str());
		if (!mMaterial || !mVertexBuffer || mOut)
			return;

		auto& renderer = Renderer::GetInstance();

		if (mScissor)
		{
			renderer.SetScissorRects(&mScissorRect, 1);
		}
		mMaterial->Bind(true);
		if (!mNoDrawBackground)
		{			
			PrepareVBs();

			//renderer.UpdateObjectConstantsBuffer(&mObjectConstants);		
			//mRasterizerStateShared->Bind();
			renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			const unsigned int numBuffers = 4;
			VertexBufferPtr buffers[numBuffers] = { mVertexBuffer, mVBColor, mVBTexCoords[0], mVBTexCoords[1] };
			unsigned int strides[numBuffers] = { mVertexBuffer->GetStride(),
				mVBColor ? mVBColor->GetStride() : 0,
				mVBTexCoords[0] ? mVBTexCoords[0]->GetStride() : 0,
				mVBTexCoords[1] ? mVBTexCoords[1]->GetStride() : 0
			};
			unsigned int offsets[numBuffers] = { 0, 0, 0, 0 };
			renderer.SetVertexBuffers(0, numBuffers, buffers, strides, offsets);

			renderer.Draw(mVertexBuffer->GetNumVertices(), 0);
		}

		if (!mText.empty())
		{
			FontPtr pFont = renderer.GetFont(mTextSize * mScale.x);
			if (pFont)
			{
				pFont->PrepareRenderResources();
				pFont->SetRenderStates(false, mScissor);
				float x = float(mRegion.left + mTextOffset.x + mTextOffsetForCursor.x);
				float y = float(mRegion.top + mTextOffset.y + mTextOffsetForCursor.y);
				Color textColor = mTextColor;
				textColor.a() *= mAlpha;
				pFont->Write(x, y, 0.0f, textColor.Get4Byte(), (const char*)mText.c_str(), -1, Font::FONT_ALIGN_LEFT);
			}
		}

		/*if (gFBEnv->pConsole->GetEngineCommand()->UI_Debug)
		{
		if (gFBEnv->pEngine->GetMouse()->IsIn(mRegion)){
		IFont* pFont = renderer.GetFont();
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

	void PrepareVBs()
	{
		auto& renderer = Renderer::GetInstance();
		if (mNeedToUpdatePosVB){
			mNeedToUpdatePosVB = false;
			auto mapData = mVertexBuffer->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
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
				mVertexBuffer->Unmap(0);
			}
		}

		if (mNeedToUpdateColorVB){
			mNeedToUpdateColorVB = false;

			if (!mColors.empty())
			{
				mVBColor = renderer.CreateVertexBuffer(&mColors[0],
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
						mVBTexCoords[i] = renderer.CreateVertexBuffer(&mTexcoords[i][0],
							sizeof(Vec2f), mTexcoords[i].size(), BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
					}
					else{
						auto map = mVBTexCoords[i]->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
						if (map.pData){
							memcpy(map.pData, &mTexcoords[i][0], sizeof(Vec2f)* mTexcoords[i].size());
							mVBTexCoords[i]->Unmap(0);
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

	void SetTextColor(const Color& c)
	{
		mTextColor = c;
	}

	void SetTextSize(float size)
	{
		mTextSize = size;
	}

	const Rect& GetRegion() const
	{
		return mRegion;
	}

	void SetDebugString(const char* string)
	{
		mDebugString = string;
	}

	void SetNoDrawBackground(bool flag)
	{
		mNoDrawBackground = flag;
	}

	bool GetNoDrawBackground() const { 
		return mNoDrawBackground; 
	}

	void SetUseScissor(bool use, const Rect& rect)
	{
		mScissor = use;


		if (!mMaterial)
		{
			Error(FB_DEFAULT_LOG_ARG, "Doesn't have material. Cannot change scissor mode.");
			return;
		}

		if (mScissor)
		{
			mScissorRect = rect;
			RASTERIZER_DESC rd;
			rd.ScissorEnable = true;			
			mMaterial->SetRasterizerState(rd);
		}
		else
		{
			mMaterial->ClearRasterizerState();
		}
		UpdateRegion();
	}

	void SetSpecialOrder(int order) { 
		mSpecialOrder = order; 
	}

	int GetSpecialOrder() const {
		return mSpecialOrder;
	}

	void SetMultiline(bool multiline){ 
		mMultiline = multiline; 
	}

	void SetDoNotDraw(bool doNotDraw)
	{
		mDoNotDraw = doNotDraw;
	}

	void SetRenderTargetSize(const Vec2I& rtSize)
	{
		mRenderTargetSize = rtSize;
		mNeedToUpdatePosVB = true;
	}

	const Vec2I& GetRenderTargetSize() const
	{
		return mRenderTargetSize;
	}


	bool HasTexCoord() const{
		return !mTexcoords[0].empty() || !mTexcoords[1].empty();
	}

	void EnableLinearSampler(bool linear){
		if (mMaterial){			
			if (linear){
				mMaterial->AddShaderDefine("_USE_LINEAR_SAMPLER", "1");
				//mMaterial->ApplyShaderDefines();
			}
			else{
				mMaterial->RemoveShaderDefine("_USE_LINEAR_SAMPLER");
					//mMaterial->ApplyShaderDefines();
			}
		}
	}
};

UIObjectPtr UIObject::Create(const Vec2I& renderTargetSize)
{
	static unsigned uinum = 0;
	UIObjectPtr p(new UIObject, [](UIObject* obj){ delete obj; });	
	p->SetRenderTargetSize(renderTargetSize);
	//p->SetDebugNumber(uinum++);
	return p;
}

//---------------------------------------------------------------------------
UIObject::UIObject()
	: mImpl(new Impl())
{

}
UIObject::~UIObject()
{
}

void UIObject::SetTexCoord(Vec2 coord[], DWORD num, unsigned index) {
	mImpl->SetTexCoord(coord, num, index);
}

void UIObject::ClearTexCoord(unsigned index) {
	mImpl->ClearTexCoord(index);
}

void UIObject::SetColors(DWORD colors[], DWORD num) {
	mImpl->SetColors(colors, num);
}

void UIObject::SetUIPos(const Vec2I& pos) {
	mImpl->SetUIPos(pos);
}

const Vec2I& UIObject::GetUIPos() const {
	return mImpl->GetUIPos();
}

void UIObject::SetUISize(const Vec2I& size) {
	mImpl->SetUISize(size);
}

const Vec2I& UIObject::GetUISize() const {
	return mImpl->GetUISize();
}

void UIObject::SetAlpha(float alpha) {
	mImpl->SetAlpha(alpha);
}

void UIObject::SetText(const wchar_t* s) {
	mImpl->SetText(s);
}

void UIObject::SetTextOffset(const Vec2I& offset) {
	mImpl->SetTextOffset(offset);
}

const Vec2I& UIObject::GetTextOffset() const {
	return mImpl->GetTextOffset();
}

void UIObject::SetTextOffsetForCursorMovement(const Vec2I& offset) {
	mImpl->SetTextOffsetForCursorMovement(offset);
}

Vec2I UIObject::GetTextStartWPos() const {
	return mImpl->GetTextStartWPos();
}

void UIObject::SetTextColor(const Color& c) {
	mImpl->SetTextColor(c);
}

void UIObject::SetTextSize(float size) {
	mImpl->SetTextSize(size);
}

const Rect& UIObject::GetRegion() const {
	return mImpl->GetRegion();
}

void UIObject::SetDebugString(const char* string) {
	mImpl->SetDebugString(string);
}

void UIObject::SetNoDrawBackground(bool flag) {
	mImpl->SetNoDrawBackground(flag);
}

bool UIObject::GetNoDrawBackground() const {
	return mImpl->GetNoDrawBackground();
}

void UIObject::SetUseScissor(bool use, const Rect& rect) {
	mImpl->SetUseScissor(use, rect);
}

void UIObject::SetSpecialOrder(int order) {
	mImpl->SetSpecialOrder(order);
}

int UIObject::GetSpecialOrder() const {
	return mImpl->GetSpecialOrder();
}

void UIObject::SetMultiline(bool multiline) {
	mImpl->SetMultiline(multiline);
}

void UIObject::SetDoNotDraw(bool doNotDraw) {
	mImpl->SetDoNotDraw(doNotDraw);
}

void UIObject::SetRenderTargetSize(const Vec2I& rtSize) {
	mImpl->SetRenderTargetSize(rtSize);
}

const Vec2I& UIObject::GetRenderTargetSize() const {
	return mImpl->GetRenderTargetSize();
}

bool UIObject::HasTexCoord() const {
	return mImpl->HasTexCoord();
}

void UIObject::EnableLinearSampler(bool linear) {
	mImpl->EnableLinearSampler(linear);
}

void UIObject::PreRender() {
	mImpl->PreRender();
}

void UIObject::Render() {
	mImpl->Render();
}

void UIObject::SetMaterial(const char* name) {
	mImpl->SetMaterial(name);
}

MaterialPtr UIObject::GetMaterial() const {
	return mImpl->GetMaterial();
}

void UIObject::UpdateRegion() {
	mImpl->UpdateRegion();
}

