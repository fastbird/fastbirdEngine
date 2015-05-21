#pragma once
namespace fastbird{
	struct RegionTestParam{
		RegionTestParam()
			: mOnlyContainer(false)
			, mIgnoreScissor(false)
			, mTestChildren(false)
		{}
		RegionTestParam(bool onlyContainer, bool ignoreScissor, bool testChildren)
			: mOnlyContainer(onlyContainer)
			, mIgnoreScissor(ignoreScissor)
			, mTestChildren(testChildren)
		{}
		bool mOnlyContainer;
		bool mIgnoreScissor;
		bool mTestChildren;
	};
}