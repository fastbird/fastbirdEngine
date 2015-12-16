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
#include "AudioEx.h"
#include "AudioManager.h"
#include "FBCommonHeaders/Helpers.h"
using namespace fb;
using namespace std::chrono;
class AudioEx::Impl{
public:
	enum AudioType{
		Start,
		Loop,
		End,
		Num
	};
	AudioExWeakPtr mSelfPtr;
	std::string mAudioBuffers[AudioType::Num];
	float mDurations[AudioType::Num];
	AudioType mCurPlaying;
	AudioId mCurAudioId;
	
	INT64 mStartTick;
	TIME_PRECISION mPlayingTime;
	TIME_PRECISION mRequestedTime;
	TIME_PRECISION mLoopStartedTime;

	TIME_PRECISION mReservedRequestTime;

	AudioProperty mProperty;

	Impl(const AudioProperty& prop)
		: mStartTick(0)
		, mPlayingTime(0)
		, mRequestedTime(0)
		, mCurPlaying(AudioType::Num)
		, mCurAudioId(INVALID_AUDIO_ID)
		, mProperty(prop)
		, mLoopStartedTime(0)
		, mReservedRequestTime(0)
	{
		for (int i = 0; i < AudioType::Num; ++i){
			mDurations[i] = 0.f;
		}
	}
	

	void SetStartLoopEnd(const char* start, const char* loop, const char* end){
		if (!start || !loop || !end){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return;
		}

		mAudioBuffers[AudioType::Start] = start;
		mAudioBuffers[AudioType::Loop] = loop;
		mAudioBuffers[AudioType::End] = end;
		auto& am = AudioManager::GetInstance();
		if (!mAudioBuffers[AudioType::Start].empty())
			mDurations[AudioType::Start] = am.GetAudioLength(start);
		if (!mAudioBuffers[AudioType::Loop].empty())
			mDurations[AudioType::Loop] = am.GetAudioLength(loop);
		if (!mAudioBuffers[AudioType::End].empty())
			mDurations[AudioType::End] = am.GetAudioLength(end);
	}

	void SetPosition(float x, float y, float z){
		mProperty.mPosition = std::make_tuple(x, y, z);
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetPosition(mCurAudioId, x, y, z);
		}
	}

	void SetRelative(bool relative){
		mProperty.mRelative = relative;
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetRelative(mCurAudioId, relative);
		}
	}
	void SetReferenceDistance(float referenceDistance){
		mProperty.mReferenceDistance = referenceDistance;
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetReferenceDistance(mCurAudioId, referenceDistance);
		}
	}

	void SetRolloffFactor(float rolloffFactor){
		mProperty.mRolloffFactor = rolloffFactor;
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetReferenceDistance(mCurAudioId, rolloffFactor);
		}
	}

	void Play(TIME_PRECISION forSec){
		if (mCurPlaying == AudioType::Num){			
			mRequestedTime = forSec;
			if (mDurations[AudioType::Loop] == 0.f && forSec > mDurations[AudioType::Start] + mDurations[AudioType::End]){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("No loop sound is specified. Audio will finish quicker than"
					" you requested. start: %s, end: %s",
					mAudioBuffers[AudioType::Start].c_str(), mAudioBuffers[AudioType::End].c_str()).c_str());
			}
			mStartTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			mPlayingTime = 0;
			auto& am = AudioManager::GetInstance();
			if (!mAudioBuffers[AudioType::Start].empty()){
				mCurPlaying = AudioType::Start;
				mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::Start].c_str(), mProperty);
			}
			else if (!mAudioBuffers[AudioType::Loop].empty()){
				mCurPlaying = AudioType::Loop;
				mCurAudioId = am.PlayAudioWithFadeIn(mAudioBuffers[AudioType::Loop].c_str(), mProperty, mDurations[AudioType::Loop] * 0.1f);
			}
			else if (!mAudioBuffers[AudioType::End].empty()){
				mCurPlaying = AudioType::End;
				mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
			}
		}
		else{
			mReservedRequestTime = forSec;
		}
	}

	void ExtendTime(TIME_PRECISION forSec){
		if (mCurPlaying != AudioType::Num){
			mRequestedTime += forSec;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot extend time for AudioEx(%s, %s, %s)", 
				mAudioBuffers[AudioType::Start].c_str(),
				mAudioBuffers[AudioType::Loop].c_str(),
				mAudioBuffers[AudioType::End].c_str()).c_str());
		}
	}

	void Stop(float fadeOutTime, bool playEnd){
		auto& am = AudioManager::GetInstance();
		if (mCurAudioId){
			am.StopWithFadeOut(mCurAudioId, fadeOutTime);
			mCurAudioId = INVALID_AUDIO_ID;
		}
		if (playEnd && mDurations[AudioType::End] != 0){
			am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
		}
		mCurPlaying = AudioType::Num;
	}

	bool IsPlaying() const{
		return mCurAudioId != INVALID_AUDIO_ID;
	}

	bool Update(){
		auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
		mPlayingTime = (curTick - mStartTick) / (TIME_PRECISION)std::milli::den;
		auto& am = AudioManager::GetInstance();
		auto leftTime = mRequestedTime - mPlayingTime;
		if (mCurPlaying == AudioType::Start){
			// jump to end case
			if (mDurations[AudioType::End] >= leftTime){
				am.StopWithFadeOut(mCurAudioId, 0.2f);
				if (!mAudioBuffers[AudioType::End].empty()){
					mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
					mCurPlaying = AudioType::End;
				}
				else{
					mCurPlaying = AudioType::Num;
				}
			}
			// start finished case
			else if (mDurations[AudioType::Start] - mPlayingTime <= 0.05f){
				if (!mAudioBuffers[AudioType::Loop].empty()){
					mCurPlaying = AudioType::Loop;
					mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::Loop].c_str(), mProperty);
					mLoopStartedTime = mPlayingTime;
				}
				else if (!mAudioBuffers[AudioType::End].empty()){
					mCurPlaying = AudioType::End;
					mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
				}
			}
		}
		else if (mCurPlaying == AudioType::Loop){
			// jump to end case
			if (mDurations[AudioType::End] >= leftTime){
				am.StopWithFadeOut(mCurAudioId, 0.2f);
				if (!mAudioBuffers[AudioType::End].empty()){
					mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
					mCurPlaying = AudioType::End;
				}
				else{
					mCurPlaying = AudioType::Num;
				}
			}
			else{
				auto played = mPlayingTime - mLoopStartedTime;				
				if (played / mDurations[AudioType::Loop] > 0.9f){
					auto left = mDurations[AudioType::Loop] - played;					
					am.StopWithFadeOut(mCurAudioId, left);
					mCurAudioId = am.PlayAudioWithFadeIn(mAudioBuffers[AudioType::Loop].c_str(), mProperty, left*.6f);
					mLoopStartedTime = mPlayingTime;					
				}
				if (mDurations[AudioType::End] == 0.f && leftTime <= 0.1f){
					am.StopWithFadeOut(mCurAudioId, 0.1f);
					return true;
				}
			}
		}
		if (mCurPlaying == AudioType::Num && mReservedRequestTime > 0){
			Play(mReservedRequestTime);
			mReservedRequestTime = 0;
			return false;
		}

		return mCurAudioId == INVALID_AUDIO_ID; // finished.
	}

	// internal
	void OnFinish(AudioId id){
		if (mCurAudioId == id){
			mCurAudioId = INVALID_AUDIO_ID;			
		}
	}
};
//---------------------------------------------------------------------------
AudioExPtr AudioEx::Create(const AudioProperty& prop){
	AudioExPtr p(new AudioEx(prop), [](AudioEx* obj){delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}

AudioEx::AudioEx(const AudioProperty& prop)
	: mImpl(new Impl(prop))
{

}

AudioEx::~AudioEx(){

}
void AudioEx::SetStartLoopEnd(const char* start, const char* loop, const char* end) {
	mImpl->SetStartLoopEnd(start, loop, end);
}

void AudioEx::SetPosition(float x, float y, float z) {
	mImpl->SetPosition(x, y, z);
}

void AudioEx::SetRelative(bool relative) {
	mImpl->SetRelative(relative);
}

void AudioEx::SetReferenceDistance(float referenceDistance) {
	mImpl->SetReferenceDistance(referenceDistance);
}

void AudioEx::SetRolloffFactor(float rolloffFactor) {
	mImpl->SetRolloffFactor(rolloffFactor);
}

void AudioEx::Play(TIME_PRECISION forSec) {
	mImpl->Play(forSec);
	AudioManager::GetInstance().RegisterAudioEx(mImpl->mSelfPtr.lock());
}

void AudioEx::ExtendTime(TIME_PRECISION forSec){
	mImpl->ExtendTime(forSec);
}

void AudioEx::Stop(float fadeOutTime, bool playEnd) {
	mImpl->Stop(fadeOutTime, playEnd);
}

bool AudioEx::IsPlaying() const {
	return mImpl->IsPlaying();
}

bool AudioEx::Update() {
	return mImpl->Update();
}

void AudioEx::OnFinish(AudioId id) {
	mImpl->OnFinish(id);
}

