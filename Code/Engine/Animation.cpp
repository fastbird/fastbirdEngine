#include <Engine/StdAfx.h>
#include <Engine/Animation.h>

namespace fastbird
{

	Animation::Animation()
		:mAnimationData(0)
	{
		mPlayingTime = 0.f;
		mPrevPlayingTime = -0.1f;
		mCurPlayingAction = 0;
		mLastUpdatedFrame = 0;
		mNextAction = 0;
		mReverse = false;
		mNextReverse = false;
		mCycled = false;
		mChanged = false;
	}

	void Animation::PlayAction(const std::string& name, bool immediate, bool reverse)
	{
		assert(mAnimationData);
		auto it = mAnimationData->mActions.Find(name);
		if (it == mAnimationData->mActions.end())
		 {
			Log("Cannot find an action (%s) in animation(%s)", name.c_str(), mAnimationData->mName.c_str());
			 return;
		 }
		 if (immediate || !mCurPlayingAction ||
			 (mReverse && mPlayingTime == 0.f) ||
			 (!mReverse && mPlayingTime == mCurPlayingAction->mLength) ||
			 mCurPlayingAction->mLoop
			 )
		 {
			 mCurPlayingAction = &it->second;
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
			 mNextAction = &it->second;
			 mNextReverse = reverse;
		 }
	}

	void Animation::Update(float dt)
	{
		if (mCurPlayingAction)
		{
			if (mLastUpdatedFrame == gFBEnv->mFrameCounter)
				return;

			mChanged = false;

			if (mPrevPlayingTime == mPlayingTime)
				return;

			float curTime = mPlayingTime;
			if (mReverse)
				mPlayingTime -= dt;
			else
				mPlayingTime += dt;

			mLastUpdatedFrame = gFBEnv->mFrameCounter;
			// evaluate
			float normTime = curTime / mCurPlayingAction->mLength;
			bool cycled = mCycled;
			mCycled = false;
			if (mAnimationData->HasPosAnimation())
			{
				const Vec3 *p1 = 0, *p2 = 0;
				float interpol = 0;
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
				float interpol = 0;
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

	

	bool Animation::IsActionDone(const char* action) const
	{
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
		if (strcmp(mNextAction->mName.c_str(), action)==0)
			return false;

		return true;
	}
}