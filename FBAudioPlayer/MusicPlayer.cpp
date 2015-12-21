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

	//---------------------------------------------------------------------------
	Impl()
		: mCurrentAudioId(INVALID_AUDIO_ID)		
		, mStartNewAfter(0)
	{

	}

	void PlayMusic(const char* path, float fadeOutOld){
		if (!ValidCStringLength(path))
			return;

		auto& am = AudioManager::GetInstance();
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (_stricmp(path, ".fbaudio") == 0){
			mCurrentAudioId = am.PlayFBAudio(path);
		}
		else{
			mCurrentAudioId = am.PlayAudio(path);
		}
		
		mCurrentAudioPath = path;
	}

	void ChangeMusic(const char* path, float fadeOutOld, float startNewAfter){
		if (!ValidCStringLength(path))
			return;
		startNewAfter = std::max(0.f, startNewAfter);
		auto& am = AudioManager::GetInstance();
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (startNewAfter == 0.f){
			if (_stricmp(path, ".fbaudio") == 0){
				mCurrentAudioId = am.PlayAudio(path);
			}
			else{
				mCurrentAudioId = am.PlayFBAudio(path);
			}
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
				auto& am = AudioManager::GetInstance();
				if (_stricmp(mCurrentAudioPath.c_str(), ".fbaudio") == 0){
					mCurrentAudioId = am.PlayFBAudio(mCurrentAudioPath.c_str());
				}
				else{
					mCurrentAudioId = am.PlayAudio(mCurrentAudioPath.c_str());
				}
			}
		}
	}

	bool IsPlaying(){
		return AudioManager::GetInstance().IsValidSource(mCurrentAudioId);
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