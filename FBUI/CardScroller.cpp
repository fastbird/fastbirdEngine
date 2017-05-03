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
#include "CardScroller.h"
#include "Scroller.h"
#include "CardData.h"
#include "Wnd.h"
#include "UIManager.h"
#include "ImageBox.h"
#include "NamedPortrait.h"
#include "Button.h"
#include "UIObject.h"

using namespace fb;

CardScrollerPtr CardScroller::Create(){
	CardScrollerPtr p(new CardScroller, [](CardScroller* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

CardScroller::CardScroller()
: mRatio(1)
, mNYOffset(0.f)
, mCardOffsetY(2)
, mCardSizeY(100)
, mCardSizeX(200)
, mStartIndex(0)
, mEndIndex(10)
{
	mUIObject = UIObject::Create(GetRenderTargetSize(), this);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUseScrollerV = true;
	mUIObject->SetNoDrawBackground(true);
	mCardData = FB_NEW(CardData);
}

CardScroller::~CardScroller()
{
	FB_DELETE(mCardData);
}

void CardScroller::OnSizeChanged()
{
	__super::OnSizeChanged();
	
	//mWidth = 1.0f - (4 / (float)GetParentSize().x);
	const auto& size = GetFinalSize();	
	mWidth = mCardSizeX / (float)size.x;
	mHeight = mCardSizeY / (float)size.y;
}

bool CardScroller::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::CARD_SIZEX:
	{
		mCardSizeX = StringConverter::ParseInt(val);
		SetCardSizeX(mCardSizeX);
		return true;
	}
	case UIProperty::CARD_SIZEY:
	{
		mCardSizeY = StringConverter::ParseInt(val);
		SetCardSizeY(mCardSizeY);
		return true;
	}
	case UIProperty::CARD_OFFSETY:
	{
		mCardOffsetY = StringConverter::ParseInt(val);
		SetCardOffset(mCardOffsetY);
		return true;
	}
	}
	return __super::SetProperty(prop, val);
}

bool CardScroller::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::CARD_SIZEX:
	{
		if (notDefaultOnly)
		{
			if (mCardSizeX == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mCardSizeX);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::CARD_SIZEY:
	{
		if (notDefaultOnly)
		{
			if (mCardSizeY == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mCardSizeY);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::CARD_OFFSETY:
	{
		if (notDefaultOnly)
		{
			if (mCardOffsetY == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::ToString(mCardOffsetY);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void CardScroller::SetCardSize_Offset(const Vec2& x_ratio, int offset)
{
	mWidth = x_ratio.x;
	mRatio = x_ratio.y;
	mCardOffsetY = offset;
	auto localRT = GetParentSize();
	mNYOffset = offset / (float)localRT.y;

	float iWidth = localRT.x * mWidth;
	mCardSizeX = Round(iWidth);
	float iHeight = iWidth / mRatio;
	mCardSizeY = Round(iHeight);
	mHeight = iHeight / (float)localRT.y;
}

void CardScroller::SetCardSize(const Vec2I& size)
{
	auto localRT = GetParentSize();
	mWidth = size.x / (float)localRT.x;
	mHeight = size.y / (float)localRT.y;	
	
	mCardSizeX = size.x;
	mCardSizeY = size.y;
}

void CardScroller::SetCardSizeNX(float nx)
{
	mWidth = nx;
	mCardSizeX = Round(nx * GetParentSize().x);
}

void CardScroller::SetCardSizeX(int x)
{
	mCardSizeX = x;
	const auto& rtSize = GetFinalSize();
	mWidth = x / (float)rtSize.x;
}

void CardScroller::SetCardSizeY(int y)
{
	mCardSizeY = y;
	const auto& rtSize = GetFinalSize();
	mHeight = y / (float)rtSize.y;
}

void CardScroller::SetCardOffset(int offset)
{
	mCardOffsetY = offset;
	const auto& rtSize = GetFinalSize();
	mNYOffset = offset / (float)rtSize.y;
}

unsigned CardScroller::AddCard(unsigned key, LuaObject& data)
{
	unsigned index = mCardData->AddData(key, data);
	if (mItems.size() <= index){
		mItems.push_back(WndWeakPtr());
	}
	VisualizeData(index);
	return index;
}

void CardScroller::DeleteCard(unsigned key)
{
	unsigned idx = mCardData->DeleteData(key);
	if (idx >= mStartIndex && idx <= mEndIndex){
		for (unsigned i = idx; i <= mEndIndex; i++)
			VisualizeData(i);
	}
}

void CardScroller::DeleteCardWithIndex(unsigned index){
	mCardData->DeleteDataWithIndex(index);
}

void CardScroller::DeleteAllCard(){
	mCardData->Clear();
	VisualizeData(0);
	mRecycleBin.clear();
}

void CardScroller::SetTexture(unsigned key, const char* comp, TexturePtr texture){
	mCardData->SetTexture(key, comp, texture);
	unsigned index = mCardData->GetIndex(key);
	if (index != -1)
		RefreshTextures(index);
}

void CardScroller::VisualizeData(unsigned index){
	auto numData = mCardData->GetNumData();
	if ( numData == 0){
		RemoveAllChildren();
		mItems.clear();
		return;
	}
	if (numData <= index){
		MoveToRecycle(index);
		return;
	}
	unsigned key = mCardData->GetKey(index);
	
	if (index < mStartIndex || index > mEndIndex)
	{
		if (!mItems[index].expired())
		{
			MoveToRecycle(index);
		}
	}

	int hgap = mCardSizeY + mCardOffsetY;
	Vec2 offset(0, 0);
	auto scrollerV = mScrollerV.lock();
	if (scrollerV)
	{
		offset = scrollerV->GetOffset();
	}

	const auto data = mCardData->GetDataWithIndex(index);
	if (!data.IsValid())
	{
		MoveToRecycle(index);
		return;
	}

	if (!mItems[index].expired())
	{
		auto item = mItems[index].lock();
		auto it = mKeys.find(index);
		if (it == mKeys.end() || it->second != key){
			item->SetVisible(false);
			item->RemoveAllChildren(true);
			item->ParseLua(data);
			item->SetVisible(true);
			RefreshTextures(index);
			RefreshProps(index);
		}
	}
	else
	{
		if (!mRecycleBin.empty())
		{
			auto item = mRecycleBin.back();
			int y = hgap * index + mCardOffsetY;
			item->SetPosY(y);
			mChildren.push_back(item);
			mItems[index] = item;
			mRecycleBin.pop_back();			
			item->ParseLua(data);	
			item->SetVisible(true);
			RefreshTextures(index);
			RefreshProps(index);
			item->SetWNScrollingOffset(offset);
			
		}
		else
		{
			int y = hgap * index + mCardOffsetY;
			auto item = CreateNewCard(index);
			mItems[index] = item;
			item->ParseLua(data);
			RefreshTextures(index);
			RefreshProps(index);
			item->SetWNScrollingOffset(offset);
		}
	}
	mKeys[index] = key;
}

Vec2 CardScroller::Scrolled()
{
	unsigned prevStart = mStartIndex;
	unsigned prevEnd = mEndIndex;
	int hgap = mCardSizeY + mCardOffsetY;
	auto scrollerV = mScrollerV.lock();
	if (scrollerV)
	{
		Vec2 offset = scrollerV->GetOffset();
		int scrolledLen = -Round(offset.y * GetRenderTargetSize().y) - mCardOffsetY;
		int topToBottom = mSize.y + scrolledLen - mCardOffsetY;

		// decide visual index range		
		mStartIndex = scrolledLen / hgap;
		mEndIndex = topToBottom / hgap;
		int remain = topToBottom % hgap;
		if (remain > 0)
			mEndIndex += 1;
	}
	else
	{
		// decide visual index range
		int topToBottom = mSize.y;
		mStartIndex = 0;
		mEndIndex = topToBottom / hgap;
		int remain = topToBottom % hgap;
		if (remain > 0)
			mEndIndex += 1;
	}

	// to recycle
	while (prevStart < mStartIndex)
	{
		unsigned index = prevStart++;
		MoveToRecycle(index);
	}

	while (prevEnd > mEndIndex)
	{
		unsigned index = prevEnd--;
		MoveToRecycle(index);
	}

	//to visual
	while (prevStart > mStartIndex)
	{
		--prevStart;
		unsigned visualIndex = prevStart;
		VisualizeData(visualIndex);
	}

	while (prevEnd < mEndIndex)
	{
		++prevEnd;
		unsigned visualIndex = prevEnd;
		VisualizeData(visualIndex);
	}

	return __super::Scrolled();
}

void CardScroller::MoveToRecycle(unsigned index){
	if (index < mItems.size())
	{
		auto target = mItems[index].lock();
		if (target)
		{
			if (target->IsKeyboardFocused())
				return;
			
			target->RemoveAllChildren(true);
			mRecycleBin.push_back(target);
			RemoveChild(target);
			mItems[index].reset();
			UIManager::GetInstance().DirtyRenderList(mHwndId);
		}
	}
}

WndPtr CardScroller::CreateNewCard(unsigned index){
	int hgap = mCardSizeY + mCardOffsetY;
	int x = 0;
	int y = hgap * index + mCardOffsetY;
	
	auto item = std::static_pointer_cast<Wnd>(AddChild(Vec2I(x, y), Vec2I(mCardSizeX, mCardSizeY), ComponentType::Window));
	item->SetRuntimeChild(true);
	item->SetRuntimeChildRecursive(true);
	return item;
}

bool CardScroller::IsExisting(unsigned key){
	auto data = mCardData->GetData(key);
	return data.IsValid();
}

void CardScroller::RefreshTextures(unsigned index){
	auto key = mCardData->GetKey(index);
	std::vector<CardData::TextureData> textures;
	mCardData->GetTextures(key, textures);
	for (auto& t : textures){		
		auto root = GetRootWnd();		
		auto comp = root->GetChild(t.compName.c_str(), true);
		if (comp){
			auto texture = t.texture;
			if (comp->GetType() == ComponentType::ImageBox)
			{
				auto imgBox = std::static_pointer_cast<ImageBox>(comp);
				imgBox->SetTexture(texture);

			}
			else if (comp->GetType() == ComponentType::NamedPortrait)
			{
				auto portrait = std::static_pointer_cast<NamedPortrait>(comp);
				portrait->SetTexture(texture);
			}
			else if (comp->GetType() == ComponentType::Button)
			{
				auto btn = std::static_pointer_cast<Button>(comp);
				btn->SetTexture(ButtonImages::Image, texture, true);
			}
		}
	}
}

void CardScroller::RefreshProps(unsigned index){
	auto key = mCardData->GetKey(index);
	auto it = mItemProps.find(key);
	if (it == mItemProps.end())
		return;

	auto& props = it->second;
	for (auto p : props){
		auto comp = GetChild(p.compName.c_str(), true);
		if (comp){
			comp->SetProperty(p.prop, p.val.c_str());
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "No component found in CardScroller.");
		}
		
	}
}

void CardScroller::SetItemProperty(unsigned key, const char* comp, const char* prop, const char* val){
	auto& props = mItemProps[key];
	UIProperty::Enum p = UIProperty::ConvertToEnum(prop);
	bool updated = false;
	for (auto it = props.begin(); it != props.end(); ++it){
		if (it->compName == comp && it->prop == p){
			it->val = val;
			updated = true;
			break;
		}
	}
	if (!updated){
		props.push_back(Props(comp, p, val));
	}

	unsigned idx = mCardData->GetIndex(key);
	if (idx != -1){		
		if (!mItems[idx].expired()){
			auto item = mItems[idx].lock();
			auto c = item->GetChild(comp, true);
			if (c)
				c->SetProperty(p, val);
			else
				Logger::Log(FB_ERROR_LOG_ARG, 
					"Cannot find a component in CardScroller.");
		}
	}
}