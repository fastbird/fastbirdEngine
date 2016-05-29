#include "stdafx.h"
#include "MusicPlayer.h"
#include "AudioManager.h"
#include "FBCommonHeaders/Helpers.h"
using namespace fb;
class MusicPlayer::Impl{
public:
	AudioId mCurrentAudioId;	
	std::string mCurrentAudioPath;
	float mStartNewAfter;
	float mMaxGain;
	float mGain;

	//---------------------------------------------------------------------------
	Impl()
		: mCurrentAudioId(INVALID_AUDIO_ID)		
		, mStartNewAfter(0)
		, mMaxGain(1)
		, mGain(1)
	{

	}

	void PlayMusic(const char* path, float fadeOutOld){
		if (!ValidCString(path))
			return;

		auto& am = AudioManager::GetInstance();		
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (strstr(path, ".fbaudio")){
			mCurrentAudioId = am.PlayFBAudio(path);
			mGain = am.GetGainFromFBAudio(path);
		}
		else{
			mCurrentAudioId = am.PlayAudio(path);
			mGain = 1.0f;
		}
		
		mCurrentAudioPath = path;
		AudioManager::GetInstance().SetLoop(mCurrentAudioId, true);
		AudioManager::GetInstance().SetGain(mCurrentAudioId, mGain * mMaxGain, true);
	}

	void PlayMusic(const char* path, float fadeOutOld, bool loop){		
		PlayMusic(path, fadeOutOld);
		AudioManager::GetInstance().SetLoop(mCurrentAudioId, loop);
	}

	void ChangeMusic(const char* path, float fadeOutOld, float startNewAfter){
		if (!ValidCString(path))
			return;
		startNewAfter = std::max(0.f, startNewAfter);
		auto& am = AudioManager::GetInstance();
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (startNewAfter == 0.f){
			PlayMusic(path, fadeOutOld);			
		}
		else{
			mStartNewAfter = startNewAfter;
			mCurrentAudioPath = path;
		}
	}

	void StopMusic(float fadeOut){
		if (mStartNewAfter > 0.f){
			mStartNewAfter = 0.f;
		}
		auto& am = AudioManager::GetInstance();
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOut);
		}

		mCurrentAudioId = INVALID_AUDIO_ID;
	}

	void Update(float dt){
		if (mStartNewAfter > 0.f){
			mStartNewAfter -= dt;
			if (mStartNewAfter <= 0.f){
				PlayMusic(mCurrentAudioPath.c_str(), 0.f);				
			}
		}
	}

	bool IsPlaying(){
		return AudioManager::GetInstance().IsValidSource(mCurrentAudioId);
	}

	void SetGain(float gain){
		mMaxGain = gain;
		if (mCurrentAudioId != INVALID_AUDIO_ID){				
			AudioManager::GetInstance().SetMaxGain(mCurrentAudioId, mMaxGain);
			AudioManager::GetInstance().SetGain(mCurrentAudioId, 
				mGain * mMaxGain, true);
		}
	}

	float GetGain() const{
		return mMaxGain;
	}

	void SetEnabled(bool enabled){
		if (enabled){
			if (!mCurrentAudioPath.empty())
				PlayMusic(mCurrentAudioPath.c_str(), 0.f);
		}
	}
};

//---------------------------------------------------------------------------
MusicPlayerWeakPtr sMusicPlayer;
MusicPlayer* sMusicPlayerRaw = 0;
MusicPlayerPtr MusicPlayer::Create(){
	if (sMusicPlayer.expired()){
		MusicPlayerPtr p(new MusicPlayer, [](MusicPlayer* obj){ delete obj; });
		sMusicPlayer = p;
		sMusicPlayerRaw = p.get();
		return p;
	}
	return sMusicPlayer.lock();
}
MusicPlayer& MusicPlayer::GetInstance() const{
	if (sMusicPlayer.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "MusicPlayer is deleted already. Program will crash.");
	}
	return *sMusicPlayerRaw;
}

MusicPlayer::MusicPlayer()
	: mImpl(new Impl)
{

}

MusicPlayer::~MusicPlayer(){

}

void MusicPlayer::PlayMusic(const char* path, float fadeOutOld){
	mImpl->PlayMusic(path, fadeOutOld);
}

void MusicPlayer::PlayMusic(const char* path, float fadeOutOld, bool loop){
	mImpl->PlayMusic(path, fadeOutOld, loop);
}

void MusicPlayer::ChangeMusic(const char* path, float fadeOutOld, float startNewAfter){
	mImpl->ChangeMusic(path, fadeOutOld, startNewAfter);
}

void MusicPlayer::StopMusic(float fadeOut){
	mImpl->StopMusic(fadeOut);
}

void MusicPlayer::Update(float dt){
	mImpl->Update(dt);
}

bool MusicPlayer::IsPlaying(){
	return mImpl->IsPlaying();
}

void MusicPlayer::SetGain(float gain){
	mImpl->SetGain(gain);
}
float MusicPlayer::GetGain() const{
	return mImpl->GetGain();
}

void MusicPlayer::SetEnabled(bool enabled){
	mImpl->SetEnabled(enabled);
}