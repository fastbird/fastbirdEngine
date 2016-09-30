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
#include "MusicPlayer.h"
#include "AudioManager.h"
#include "AudioSource.h"
#include "FBCommonHeaders/Helpers.h"
using namespace fb;
class MusicPlayer::Impl{
public:
	AudioId mCurrentAudioId;	
	std::string mCurrentAudioPath;
	float mStartNewAfter;		
	bool mLoop;

	//---------------------------------------------------------------------------
	Impl()
		: mCurrentAudioId(INVALID_AUDIO_ID)		
		, mStartNewAfter(0)		
		, mLoop(true)
	{

	}

	void PlayMusic(const char* path, float fadeOutOld){	
		if (!ValidCString(path))
			return;
		/*using namespace std::chrono;
		auto tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();*/

		auto& am = AudioManager::GetInstance();		
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (strstr(path, ".fbaudio")){
			mCurrentAudioId = am.PlayFBAudio(path, AudioSourceType::Music);
		}
		else{
			mCurrentAudioId = am.PlayAudio(path, AudioSourceType::Music);
		}
		
		mCurrentAudioPath = path;
		AudioManager::GetInstance().SetLoop(mCurrentAudioId, true);		
		mStartNewAfter = 0.f;

		/*auto elapsed = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() - tick;
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString(
			"(info) PlayMusic(%s) takes %u", path, elapsed).c_str());*/
	}

	void PlayMusic(const char* path, float fadeOutOld, bool loop){		
		PlayMusic(path, fadeOutOld);
		AudioManager::GetInstance().SetLoop(mCurrentAudioId, loop);		
	}

	void ChangeMusic(const char* path, float fadeOutOld, float startNewAfter, bool loop){
		if (!ValidCString(path))
			return;
		startNewAfter = std::max(0.f, startNewAfter);
		auto& am = AudioManager::GetInstance();
		if (mCurrentAudioId != INVALID_AUDIO_ID){
			am.SetLoop(mCurrentAudioId, false);
			am.StopWithFadeOut(mCurrentAudioId, fadeOutOld);
		}
		if (startNewAfter == 0.f){
			PlayMusic(path, fadeOutOld, loop);
		}
		else{
			mStartNewAfter = startNewAfter;
			mCurrentAudioPath = path;
			mLoop = loop;
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
				PlayMusic(mCurrentAudioPath.c_str(), 0.5f, mLoop);
			}
		}
	}

	bool IsPlaying(){
		return AudioManager::GetInstance().IsValidSource(mCurrentAudioId);
	}

	void SetGain(float gain){
		AudioSource::sMusicGain = gain;		
		AudioManager::GetInstance().OnGainOptionChanged();
	}

	float GetGain() const{
		return AudioSource::sMusicGain;
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

void MusicPlayer::ChangeMusic(const char* path, float fadeOutOld, float startNewAfter, bool loop){
	mImpl->ChangeMusic(path, fadeOutOld, startNewAfter, loop);
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