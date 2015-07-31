#pragma once
namespace fastbird{
	class IWinBase;
	struct RegionTestParam{
		RegionTestParam()
			: mOnlyContainer(false)
			, mIgnoreScissor(false)
			, mTestChildren(false)
			, mNoRuntimeComp(false)
			, mCheckMouseEvent(false)
			, mHwndId(1)	
			, mRestrictToThisWnd(0)
		{}
		RegionTestParam(bool onlyContainer, bool ignoreScissor, bool testChildren, bool checkMouseEvent, HWND_ID hwndId)
			: mOnlyContainer(onlyContainer)
			, mIgnoreScissor(ignoreScissor)
			, mTestChildren(testChildren)
			, mCheckMouseEvent(checkMouseEvent)
			, mHwndId(hwndId)
			, mRestrictToThisWnd(0)
		{}
		bool mOnlyContainer;
		bool mIgnoreScissor;
		bool mTestChildren;
		bool mNoRuntimeComp;
		bool mCheckMouseEvent;
		IWinBase* mRestrictToThisWnd;
		HWND_ID mHwndId;
		std::vector<IWinBase*> mExceptions;
	};
}