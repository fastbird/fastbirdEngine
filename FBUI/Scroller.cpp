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
#include "Scroller.h"
#include "Container.h"
#include "UIObject.h"
namespace fb
{
ScrollerPtr Scroller::Create(){
	ScrollerPtr p(new Scroller, [](Scroller* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

Scroller::Scroller()
	: mScrollAmount(0.0015f)
	, mOffset(0, 0)
	, mMaxOffset(0, 0)	
	, mDestOffset(0)
	, mCurOffset(0)
	, mScrollAcc(10)
	, mCurScrollSpeed(0)
	, mMaxScrollSpeed(10)
	, mScrollAmountScale(1.f)
	, mLastWheel(0)
{
	mUIObject = UIObject::Create(GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

Scroller::~Scroller()
{

}

void Scroller::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject.get());	
}

bool Scroller::OnInputFromHandler(IInputInjectorPtr injector)
{
	auto parent = GetParent();
	if (!parent || !mVisibility.IsVisible())
		return false;

	bool isIn = __super::OnInputFromHandler(injector);
	long wheel = injector->GetWheel();
	if (wheel && parent->IsIn(injector))
	{
		injector->PopWheel();
		if ((gpTimer->GetTime() - mLastScrollTime )< 0.5f && Sign((float)mLastWheel) == Sign((float)wheel)){
			mScrollAmountScale += 0.3f;			
		}
		else {
			mScrollAmountScale = 1.0f;
		}
		mLastScrollTime = gpTimer->GetTime();
		mLastWheel = wheel;
		float wheelSens = injector->GetWheelSensitivity();
		float wheelSensitivity = wheelSens * (float)injector->GetNumLinesWheelScroll();

		float prevDestOffset = mDestOffset;
		mDestOffset += wheel * wheelSensitivity * (mScrollAmount * mScrollAmountScale);
		mDestOffset = std::min(0.f, mDestOffset);
		mDestOffset = std::max(-mMaxOffset.y, mDestOffset);
		if (prevDestOffset != mDestOffset || mDestOffset != mCurOffset)
			injector->ClearWheel();
	}
	return isIn;
}

void Scroller::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);
	if (mDestOffset == mCurOffset)
		return;

	float gap = mDestOffset - mCurOffset;
	mCurScrollSpeed += mScrollAcc*elapsedTime;
	mCurScrollSpeed = std::min(mCurScrollSpeed, mMaxScrollSpeed);

	mCurOffset += mCurScrollSpeed * elapsedTime * gap;
	if (gap < 0 && mDestOffset >= mCurOffset)
	{
		mCurOffset = mDestOffset;
		mCurScrollSpeed = 0.f;
	}
	else if (gap > 0 && mDestOffset <= mCurOffset)
	{
		mCurOffset = mDestOffset;
		mCurScrollSpeed = 0.f;
	}
	mOffset.y = mCurOffset;
	auto parent = GetParent();
	if (parent)
		parent->Scrolled();
}
void Scroller::ResetScroller()
{
	mOffset = Vec2(0, 0);
	mCurOffset = 0;
	mDestOffset = 0;

}

void Scroller::SetMaxOffset(const Vec2& maxOffset)
{
	if (mMaxOffset != maxOffset)
	{
		mMaxOffset = maxOffset;
		if (mOffset.y < -mMaxOffset.y)
		{
			mOffset.y = -mMaxOffset.y;
			mCurOffset = mOffset.y;
			mDestOffset = mCurOffset;
			GetParent()->Scrolled();
		}

		mScrollAmount = 22 / (float)GetRenderTargetSize().y;
			
		
	}
}

void Scroller::SetOffset(const Vec2& offset)
{
	mOffset = offset;
	if (mOffset.y < -mMaxOffset.y)
	{
		mOffset.y = -mMaxOffset.y;
	}
	mCurOffset = mOffset.y;
	mDestOffset = mCurOffset;
	GetParent()->Scrolled();
}

}