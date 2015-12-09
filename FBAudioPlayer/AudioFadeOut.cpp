#include "stdafx.h"
#include "AudioFadeOut.h"
#include "AudioManager.h"
using namespace fb;
class AudioFadeOut::Impl{
public:
	AudioId mId;
	TIME_PRECISION mTime;
	TIME_PRECISION mRemainingTime;
	float mInitialGain;

	Impl(AudioId id, TIME_PRECISION time)
		: mId(id)
		, mTime(time)
		, mRemainingTime(time)
	{
		auto& am = AudioManager::GetInstance();
		mInitialGain = am.GetGain(id);
	}

	bool Update(TIME_PRECISION dt){
		mRemainingTime -= dt;
		auto& am = AudioManager::GetInstance();
		if (mRemainingTime <= 0){
			am.StopAudio(mId);
			return true; // audio finished
		}
		else{
			bool success = am.SetGain(mId, mInitialGain * mRemainingTime / mTime);
			if (!success)
				return true; // audio finished
		}
		return false;
	}
};

AudioFadeOutPtr AudioFadeOut::Create(AudioId id, TIME_PRECISION time){
	return AudioFadeOutPtr(new AudioFadeOut(id, time), [](AudioFadeOut* obj){ delete obj; });
}

AudioFadeOut::AudioFadeOut(AudioId id, TIME_PRECISION time)
	: mImpl(new Impl(id, time))
{

}

AudioFadeOut::~AudioFadeOut(){

}

bool AudioFadeOut::Update(TIME_PRECISION dt){
	return mImpl->Update(dt);
}