#pragma once
#include <Engine/Animation/AnimationData.h>
namespace fastbird
{
	class Animation
	{
		AnimationData* mAnimationData;
		AnimationData::Action* mCurPlayingAction;
		AnimationData::Action* mNextAction;
		float mPrevPlayingTime;
		float mPlayingTime;
		bool mCycled; // true when looped.
		bool mReverse;
		bool mNextReverse;

		Transformation mResult;
		unsigned mLastUpdatedFrame;
		bool mChanged;

	public:
		Animation();		

		virtual void PlayAction(const std::string& name, bool immediate, bool reverse);
		virtual bool IsActionDone(const char* action) const;
		void Update(float dt);
		const Transformation& GetResult() const { return mResult; }
		bool Changed() const { return mChanged; }
		void SetAnimationData(AnimationData* data) { mAnimationData = data; }		
	};
}