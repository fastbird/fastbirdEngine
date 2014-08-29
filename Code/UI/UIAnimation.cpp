#include <UI/StdAfx.h>
#include <UI/UIAnimation.h>

namespace fastbird
{

//---------------------------------------------------------------------------
UIAnimation::UIAnimation()
: mLength(1.f)
, mCurTime(0)
, mLoop(true)
, mEnd(false)
, mCurPos(0, 0)
{
}

//---------------------------------------------------------------------------
UIAnimation::~UIAnimation()
{

}

//---------------------------------------------------------------------------
void UIAnimation::SetLength(float seconds)
{
	mLength = seconds;
}

//---------------------------------------------------------------------------
void UIAnimation::AddPos(float time, const Vec2& pos)
{
	if (mKeyPos.empty())
		mKeyPos[0.0f] = Vec2::ZERO;

	mKeyPos[time] = pos;
}

//---------------------------------------------------------------------------
void UIAnimation::Update(float deltaTime)
{
	if (mEnd)
		return;

	mCurTime += deltaTime;
	if (mCurTime > mLength)
	{
		if (mLoop)
		{
			mCurTime =  mCurTime - mLength;
		}
		else
		{
			mEnd = true;
			mCurTime = mLength;
		}
	}

	float normTime = mCurTime / mLength;
	if (!mKeyPos.empty())
	{
		auto it = mKeyPos.begin();
		auto itEnd = mKeyPos.end();
		for (; it != itEnd; ++it)
		{
			if (normTime <= it->first)
			{
				if (it == mKeyPos.begin())
				{
					mCurPos = it->second;
				}
				auto prevIt = it - 1;
				float l = SmoothStep(prevIt->first, it->first, normTime);
				mCurPos = Lerp(prevIt->second, it->second, l);
				break;
			}
		}
	}
}

//---------------------------------------------------------------------------
const Vec2 UIAnimation::GetCurrentPos()
{
	return mCurPos;
}

}