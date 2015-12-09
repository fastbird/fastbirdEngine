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

#include "stdafx.h"
#include "Animation.h"
#include "AnimationData.h"
#include "FBMathLib/Math.h"
#include "FBStringLib/StringLib.h"
#include "FBTimer/Timer.h"
#include "FBDebugLib/Logger.h"
using namespace fb;

class Animation::Impl{
public:
	AnimationDataPtr mAnimationData;
	const AnimationData::Action* mCurPlayingAction;
	const AnimationData::Action* mNextAction;
	TIME_PRECISION mPrevPlayingTime;
	TIME_PRECISION mPlayingTime;
	bool mCycled; // true when looped.
	bool mReverse;
	bool mNextReverse;

	Transformation mResult;
	unsigned mLastUpdatedFrame;
	bool mChanged;
	// Using Default Copy Constructor!

	//---------------------------------------------------------------------------
	Impl()
		: mCurPlayingAction(0)
		, mNextAction(0)
		, mPrevPlayingTime(-0.1f)
		, mPlayingTime(0)
		, mCycled(false)
		, mReverse(false)
		, mNextReverse(false)
		, mLastUpdatedFrame(0)
		, mChanged(false)
		
	{
	}

	// default copy ctor is fine.
	/*Impl(const Impl& other){
	}*/


	void PlayAction(const std::string& name, bool immediate, bool reverse){
		assert(mAnimationData);
		auto action = mAnimationData->GetAction(name.c_str());
		if (!action){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find an action (%s) in animation(%s)", name.c_str(), mAnimationData->GetName()).c_str());
			return;
		}
		if (immediate || !mCurPlayingAction ||
			(mReverse && mPlayingTime == 0.f) ||
			(!mReverse && mPlayingTime == mCurPlayingAction->mLength) ||
			mCurPlayingAction->mLoop
			)
		{
			mCurPlayingAction = action;
			mReverse = reverse;
			if (reverse)
			{
				mPlayingTime = mCurPlayingAction->mLength;
				mPrevPlayingTime = 0.0f;
			}
			else
			{
				mPlayingTime = 0.f;
				mPrevPlayingTime = mCurPlayingAction->mLength;
			}
			mNextAction = 0;
		}
		else
		{
			mNextAction = action;
			mNextReverse = reverse;
		}
	}

	bool IsActionDone(const char* action) const{
		if (mCurPlayingAction && strcmp(mCurPlayingAction->mName.c_str(), action) == 0)
		{
			if (mCurPlayingAction->mLoop)
				return true;
			if (mReverse)
			{
				return mPlayingTime == 0.0f;
			}
			else
			{
				return mPlayingTime == mCurPlayingAction->mLength;
			}
		}
		if (strcmp(mNextAction->mName.c_str(), action) == 0)
			return false;

		return true;
	}

	bool IsPlaying() const{
		if (mCurPlayingAction)
		{
			if (mCurPlayingAction->mLoop)
				return true;
			if (mReverse)
			{
				return mPlayingTime > 0.0f;
			}
			else
			{
				return mPlayingTime <= mCurPlayingAction->mLength;
			}
		}
		return false;
	}

	void Update(TIME_PRECISION dt){
		if (mCurPlayingAction)
		{
			if (mLastUpdatedFrame == gpTimer->GetFrame())
				return;

			mChanged = false;

			if (mPrevPlayingTime == mPlayingTime)
				return;

			TIME_PRECISION curTime = mPlayingTime;
			if (mReverse)
				mPlayingTime -= dt;
			else
				mPlayingTime += dt;

			mLastUpdatedFrame = gpTimer->GetFrame();
			// evaluate
			TIME_PRECISION normTime = curTime / mCurPlayingAction->mLength;
			bool cycled = mCycled;
			mCycled = false;
			if (mAnimationData->HasPosAnimation())
			{
				const Vec3 *p1 = 0, *p2 = 0;
				TIME_PRECISION interpol = 0;
				mAnimationData->PickPos(curTime, cycled, &p1, &p2, interpol);
				if (p1 && p2)
				{
					Vec3 pos = Lerp<Vec3>(*p1, *p2, interpol);
					mResult.SetTranslation(pos);
				}
			}

			if (mAnimationData->HasRotAnimation())
			{
				const Quat *r1 = 0, *r2 = 0;
				TIME_PRECISION interpol = 0;
				mAnimationData->PickRot(curTime, cycled, &r1, &r2, interpol);
				if (r1 && r2)
				{
					Quat rot = Slerp(*r1, *r2, interpol);
					mResult.SetRotation(rot);
				}
			}

			mPrevPlayingTime = curTime;
			mChanged = true;

			if ((!mReverse && mPlayingTime > mCurPlayingAction->mLength) ||
				(mReverse && mPlayingTime < 0))
			{
				if (mNextAction)
				{
					mCurPlayingAction = mNextAction;
					mReverse = mNextReverse;
					mNextAction = 0;
					if (mReverse)
					{
						mPlayingTime = mCurPlayingAction->mLength;
						mPrevPlayingTime = 0;
					}
					else
					{
						mPlayingTime = 0.f;
						mPrevPlayingTime = mCurPlayingAction->mLength;
					}
				}
				else
				{
					if (mReverse)
					{
						if (mCurPlayingAction->mLoop)
						{
							// mPlayingTime is negative
							mPlayingTime = mCurPlayingAction->mLength + mPlayingTime;
							mCycled = true;
						}
						else
						{
							mPlayingTime = 0.0f;
						}
					}
					else
					{
						if (mCurPlayingAction->mLoop)
						{
							mPlayingTime = mPlayingTime - mCurPlayingAction->mLength;
							mCycled = true;
						}
						else
						{
							mPlayingTime = mCurPlayingAction->mLength;
						}
					}

				}
			}
		}
	}

	const Transformation& GetResult() const { 
		return mResult; 
	}

	bool Changed() const { 
		return mChanged; 
	}

	void SetAnimationData(AnimationDataPtr data) { 
		mAnimationData = data; 
	}

	AnimationDataPtr GetAnimationData() const{
		return mAnimationData;
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(Animation);
AnimationPtr Animation::Create(const Animation& other){
	return AnimationPtr(new Animation(other), [](Animation* obj){ delete obj; });
}

Animation::Animation()
	: mImpl(new Impl){
}

Animation::Animation(const Animation& other)
	: mImpl(new Impl(*other.mImpl)){
}

Animation::~Animation(){
}

AnimationPtr Animation::Clone() const{
	auto cloned = Animation::Create(*this);
	return cloned;
}

void Animation::PlayAction(const std::string& name, bool immediate, bool reverse){
	mImpl->PlayAction(name, immediate, reverse);
}

bool Animation::IsActionDone(const char* action) const{
	return mImpl->IsActionDone(action);
}

bool Animation::IsPlaying() const{
	return mImpl->IsPlaying();
}

void Animation::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}

const Transformation& Animation::GetResult() const{
	return mImpl->GetResult();
}

bool Animation::Changed() const{
	return mImpl->Changed();
}

void Animation::SetAnimationData(AnimationDataPtr data){
	mImpl->SetAnimationData(data);
}

AnimationDataPtr Animation::GetAnimationData() const{
	return mImpl->GetAnimationData();
}