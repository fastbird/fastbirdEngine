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
#include "VisibleStatus.h"
#include "WinBase.h"
#include "ImageBox.h"
#include "UIAnimation.h"
namespace fb
{
	VisibleStatus::VisibleStatus()
		: mHideTime(0)
		, mCurVisibility(Visibility::Hided)
		, mReservedVisibility(ReservedVisibility::None)
		, mCurHideTime(0)
		, mShowTime(0)
		, mCurShowTime(0)
	{
	}

	VisibleStatus::~VisibleStatus()
	{
	}

	void VisibleStatus::Update(float dt)
	{
		if (mCurVisibility == Visibility::Showing)
		{
			if (mCurShowTime > 0.f){
				mCurShowTime -= dt;
				if (mCurShowTime <= 0)
				{
					mCurShowTime = 0;
					mCurVisibility = Visibility::Shown;
				}				
			}
		}
		else if (mCurVisibility == Visibility::Hiding)
		{
			mCurHideTime -= dt;
			if (mCurHideTime <= 0)
			{
				mCurHideTime = 0;
				mCurVisibility = Visibility::Hided;
				mWinBase->SetVisibleInternal(false);
			}
		}
	}

	bool VisibleStatus::SetVisible(bool visible)
	{
		if (visible)
		{
			if (mCurVisibility == Visibility::Shown)
			{
				return false;
			}
			else
			{
				switch (mCurVisibility)
				{
				case Visibility::Hided:
				{					
					if (mShowTime > 0){
						mCurVisibility = Visibility::Showing;
						mCurShowTime = mShowTime;
						for (auto anim : mShowAnimations)
						{
							anim->SetActivated(true);
						}
					}
					else{
						mCurVisibility = Visibility::Shown;
					}
					mWinBase->SetVisibleInternal(true);						
					return true;
				}
				case Visibility::Hiding:
				{
					mCurVisibility = Visibility::Showing;
					mCurShowTime = mShowTime;
					for (auto anim : mHideAnimations)
					{
						anim->SetActivated(false);
					}
					for (auto anim : mShowAnimations)
					{
						anim->SetActivated(true);
					}
					return false;
				}

				case Visibility::Showing:
				default:
					break;
				}
				return false;
			}
		}
		else
		{
			if (mCurVisibility == Visibility::Hided)
			{
				return false;
			}
			else
			{
				switch (mCurVisibility)
				{
				case Visibility::Shown:
				{
					if (mHideTime > 0){
						mCurVisibility = Visibility::Hiding;
						mCurHideTime = mHideTime;
						for (auto anim : mHideAnimations)
						{
							anim->SetActivated(true);
						}
					}
					else{
						mCurVisibility = Visibility::Hided;
						mWinBase->SetVisibleInternal(false);
					}

					return false; // still visible.
				}
				case Visibility::Showing:
				{
					mCurVisibility = Visibility::Hiding;
					mCurHideTime = mHideTime;
					for (auto anim : mHideAnimations)
					{
						anim->SetActivated(true);
					}
					for (auto anim : mShowAnimations)
					{
						anim->SetActivated(false);
					}

					return false; // still visible.
				}
				case Visibility::Hiding:
				default:
					break;
				}
				return false;
			}
		}
	}

	void VisibleStatus::AddShowAnimation(UIAnimationPtr showAnimCloned)
	{
		if (!ValueExistsInVector(mShowAnimations, showAnimCloned))
		{
			mShowAnimations.push_back(showAnimCloned);
			mShowTime = std::max(mShowTime, showAnimCloned->GetLength());
		}
	}

	void VisibleStatus::AddHideAnimation(UIAnimationPtr hideAnimCloned)
	{
		if (!ValueExistsInVector(mHideAnimations, hideAnimCloned))
		{
			mHideAnimations.push_back(hideAnimCloned);
			mHideTime = std::max(mHideTime, hideAnimCloned->GetLength());			
		}
	}

	void VisibleStatus::Hided()
	{
		mCurVisibility = Visibility::Hided;
	}
}