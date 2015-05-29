#include <UI/StdAfx.h>
#include <UI/VisibleStatus.h>
#include <UI/WinBase.h>
#include <UI/ImageBox.h>
namespace fastbird
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
		for (auto it : mShowAnimations)
		{
			FB_DELETE(it);
		}
		for (auto it : mHideAnimations)
		{
			FB_DELETE(it);
		}
	}

	void VisibleStatus::Update(float dt)
	{
		if (mCurVisibility == Visibility::Showing)
		{
			mCurShowTime -= dt;
			if (mCurShowTime <= 0)
			{
				mCurShowTime = 0;
				mCurVisibility = Visibility::Shown;
			}
			mWinBase->TriggerRedraw();
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
			mWinBase->TriggerRedraw();
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
					mCurVisibility = Visibility::Showing;
					mCurShowTime = mShowTime;
					mWinBase->SetVisibleInternal(true);
					for (auto anim : mShowAnimations)
					{
						anim->SetActivated(true);
					}
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
					mCurVisibility = Visibility::Hiding;
					mCurHideTime = mHideTime;
					for (auto anim : mHideAnimations)
					{
						anim->SetActivated(true);
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

	void VisibleStatus::AddShowAnimation(IUIAnimation* showAnimCloned)
	{
		if (ValueNotExistInVector(mShowAnimations, showAnimCloned))
		{
			mShowAnimations.push_back(showAnimCloned);
			mShowTime = std::max(mShowTime, showAnimCloned->GetLength());
		}
	}

	void VisibleStatus::AddHideAnimation(IUIAnimation* hideAnimCloned)
	{
		if (ValueNotExistInVector(mHideAnimations, hideAnimCloned))
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