#pragma once
#include <CommonLib/Timer.h>
namespace fastbird
{

struct BlockFrequent
{
	BlockFrequent(float* pLastTime, float elapsed)
		: mLastTime(pLastTime)
		, mBlockElapsed(elapsed)
	{
	}

	bool IsBlocked()
	{
		if (gpTimer)
		{
			float curTime = gpTimer->GetTime();
			if (*mLastTime !=0 && 
				curTime - *mLastTime < mBlockElapsed)
				return true;
		}
		return false;
	}

	~BlockFrequent()
	{
		if (gpTimer)
			*mLastTime = gpTimer->GetTime();
	}

	float* mLastTime;
	float mBlockElapsed;
};

#define BLOCK_FREQUENT_CALL(time, ret)  \
	static float lastTime = 0;\
	fastbird::BlockFrequent block(&lastTime, (time));\
	if (block.IsBlocked())\
		##ret;
}