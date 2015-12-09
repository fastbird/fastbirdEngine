#include "stdafx.h"
#include "AudioFadeIn.h"
#include "AudioManager.h"
using namespace fb;
class AudioFadeIn::Impl{
public:
	AudioId mId;
	TIME_PRECISION mTime;
	TIME_PRECISION mRemainingTime;
	float mTargetGain;

	Impl(AudioId id, TIME_PRECISION time, float targetGain)
		: mId(id)
		, mTime(time)
		, mRemainingTime(time)
		, mTargetGain(targetGain)
	{

	}

	bool Update(TIME_PRECISION dt){
		mRemainingTime -= dt;
		auto& am = AudioManager::GetInstance();
		if (mRemainingTime <= 0){
			am.SetGain(mId, mTargetGain);
			return true; // Manipulator finished
		}
		else{
			bool success = am.SetGain(mId, mTargetGain * (1.0f - mRemainingTime / mTime));
			if (!success)
				return true; // Manipulator finished
		}
		return false;
	}
};

AudioFadeInPtr AudioFadeIn::Create(AudioId id, TIME_PRECISION time, float targetGain){
	return AudioFadeInPtr(new AudioFadeIn(id, time, targetGain), [](AudioFadeIn* obj){ delete obj; });
}

AudioFadeIn::AudioFadeIn(AudioId id, TIME_PRECISION time, float targetGain)
	: mImpl(new Impl(id, time, targetGain))
{

}

AudioFadeIn::~AudioFadeIn(){

}

bool AudioFadeIn::Update(TIME_PRECISION dt){
	return mImpl->Update(dt);
}