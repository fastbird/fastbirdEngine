#pragma once

namespace fastbird
{
	class WinBase;
	class IUIAnimation;
	namespace Visibility
	{
		enum Enum
		{
			Hided,
			Showing,
			Shown,
			Hiding,
		};
	}
	namespace ReservedVisibility
	{
		enum Enum
		{
			None,
			Show,
			Hide,			
		};
	}
	class VisibleStatus
	{
		friend class WinBase;
		WinBase* mWinBase;
		Visibility::Enum mCurVisibility;
		std::vector<IUIAnimation*> mShowAnimations;
		std::vector<IUIAnimation*> mHideAnimations;
		float mHideTime;
		float mCurHideTime;
		float mShowTime;
		float mCurShowTime;
		ReservedVisibility::Enum mReservedVisibility;

	public:
		VisibleStatus();
		~VisibleStatus();
		void SetWinBase(WinBase* winBase) { mWinBase = winBase; }
		bool SetVisible(bool visible);
		void AddShowAnimation(IUIAnimation* showAnimCloned);
		void AddHideAnimation(IUIAnimation* hideAnimCloned);

		void Update(float dt);

		bool IsShowing() const { return mCurVisibility == Visibility::Showing; }
		bool IsHiding() const { return mCurVisibility == Visibility::Hiding; }
		bool IsVisible() const { return mCurVisibility == Visibility::Shown || IsShowing() || IsHiding(); }
		void Hided();
	};
}