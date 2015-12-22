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
#include "AudioManipulatorType.h"
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
	float mInitialGain;
	float mDurations[AudioType::Num];
	AudioType mCurPlaying;
	AudioId mCurAudioId;
	
	INT64 mStartTick;
	TIME_PRECISION mPlayingTime;
	TIME_PRECISION mInitialRequestedTime;
	TIME_PRECISION mRequestedTime;	
	TIME_PRECISION mLoopStartedTime;

	TIME_PRECISION mReservedRequestTime;

	AudioProperty mProperty;

	Impl()
		: mStartTick(0)
		, mPlayingTime(0)
		, mInitialRequestedTime(0)
		, mRequestedTime(0)
		, mCurPlaying(AudioType::Num)
		, mCurAudioId(INVALID_AUDIO_ID)		
		, mLoopStartedTime(0)
		, mReservedRequestTime(0)
		, mInitialGain(1.f)
	{
		for (int i = 0; i < AudioType::Num; ++i){
			mDurations[i] = 0.f;
		}
	}

	std::string VerifyTheFilepath(const char* audioEx, const char* filepath){
		if (strlen(filepath) == 0)
			return std::string();

		std::string audioPath = filepath;
		if (!FileSystem::Exists(audioPath.c_str())){
			std::string xmlpath = audioEx;
			auto onlyFilename = FileSystem::GetFileName(audioPath.c_str());
			audioPath = FileSystem::ReplaceFilename(xmlpath.c_str(), onlyFilename.c_str());
			if (!FileSystem::Exists(audioPath.c_str())){
				audioPath.clear();
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Cannot find a audio file(%s or %s)", filepath, audioPath.c_str()).c_str());
			}
		}
		return audioPath;
	}
	
	void SetAudioExFile(const char* audioEx){
		if (!ValidCStringLength(audioEx)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return;
		}

		tinyxml2::XMLDocument doc;
		auto err = doc.LoadFile(audioEx);
		if (err){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot parse fbaudioex(%s)", audioEx).c_str());
			return;
		}

		auto root = doc.RootElement();
		if (!root){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid format(%s)", audioEx);
			return;
		}
		auto& am = AudioManager::GetInstance();
		bool anyfileFound = false;
		auto filepath = root->Attribute("start");
		if (filepath){
			mAudioBuffers[AudioType::Start] = VerifyTheFilepath(audioEx, filepath);			
			if (!mAudioBuffers[AudioType::Start].empty()){
				mDurations[AudioType::Start] = am.GetAudioLength(mAudioBuffers[AudioType::Start].c_str());
				anyfileFound = true;
			}
		}
		filepath = root->Attribute("loop");
		if (filepath){
			mAudioBuffers[AudioType::Loop] = VerifyTheFilepath(audioEx, filepath);
			if (!mAudioBuffers[AudioType::Loop].empty()){
				mDurations[AudioType::Loop] = am.GetAudioLength(mAudioBuffers[AudioType::Loop].c_str());
				anyfileFound = true;
			}
		}
		filepath = root->Attribute("end");
		if (filepath){
			mAudioBuffers[AudioType::End] = VerifyTheFilepath(audioEx, filepath);
			if (!mAudioBuffers[AudioType::End].empty()){
				mDurations[AudioType::End] = am.GetAudioLength(mAudioBuffers[AudioType::End].c_str());
				anyfileFound = true;
			}
		}		
		if (!anyfileFound){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("FBAudioEx file(%s) doesn't have any audio file info.", audioEx).c_str());
		}

		auto sz = root->Attribute("playTime");
		if (sz){
			mRequestedTime = StringConverter::ParseReal(sz, mRequestedTime);
			mInitialRequestedTime = mRequestedTime;
		}

		sz = root->Attribute("gain");
		if (sz){
			mProperty.mGain = StringConverter::ParseReal(sz, mProperty.mGain);
			mInitialGain = mProperty.mGain;
		}

		sz = root->Attribute("relative");
		if (sz){
			mProperty.mRelative = StringConverter::ParseBool(sz, mProperty.mRelative);
		}

		sz = root->Attribute("rollOffFactor");
		if (sz){
			mProperty.mRolloffFactor = StringConverter::ParseReal(sz, mProperty.mRolloffFactor);
		}

		sz = root->Attribute("referenceDistance");
		if (sz){
			mProperty.mReferenceDistance = StringConverter::ParseReal(sz, mProperty.mReferenceDistance);
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
		if (!mAudioBuffers[AudioType::Start].empty()){
			mDurations[AudioType::Start] = am.GetAudioLength(start);
			if (_stricmp(FileSystem::GetExtension(start), ".fbaudio") == 0){
				mInitialGain = am.GetGainFromFBAudio(start);
			}
		}		
		if (!mAudioBuffers[AudioType::Loop].empty()){
			mDurations[AudioType::Loop] = am.GetAudioLength(loop);
			if (_stricmp(FileSystem::GetExtension(loop), ".fbaudio") == 0){
				mInitialGain = am.GetGainFromFBAudio(loop);
			}
		}		
		if (!mAudioBuffers[AudioType::End].empty())
		{
			mDurations[AudioType::End] = am.GetAudioLength(end);
			if (_stricmp(FileSystem::GetExtension(end), ".fbaudio") == 0){
				mInitialGain = am.GetGainFromFBAudio(end);
			}
		}		
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

	void SetGain(float gain){
		if (mCurPlaying == AudioType::Num)
			return;
		gain = std::max(0.f, gain);
		gain *= mInitialGain;
		float prevGain = mProperty.mGain;
		mProperty.mGain= gain;
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetGain(mCurAudioId, gain, true);
		}
		if (gain == 0.f){
			if (mCurAudioId != INVALID_AUDIO_ID){
				Stop(0.5f, false);
			}
		}
		else if (!IsPlaying() && mRequestedTime > 0.f){
			Play(mRequestedTime);
		}
	}

	void SetGainSmooth(float gain, float inSec){		
		gain = std::max(0.f, gain);
		gain *= mInitialGain;
		if (gain > 0.f && !IsPlaying()){
			mProperty.mGain = gain;
			Play(mRequestedTime);
		}
		else if (mCurAudioId != INVALID_AUDIO_ID){
			bool success = AudioManager::GetInstance().SetGainSmooth(mCurAudioId, gain, inSec);
			if (!success){
				auto reged = AudioManager::GetInstance().IsRegisteredAudioEx(mSelfPtr.lock());
				Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) audio ex is registered? : %d", reged ? 1 : 0).c_str());
			}
			mProperty.mGain = gain;
		}
	}

	void SetRolloffFactor(float rolloffFactor){
		mProperty.mRolloffFactor = rolloffFactor;
		if (mCurAudioId != INVALID_AUDIO_ID){
			AudioManager::GetInstance().SetReferenceDistance(mCurAudioId, rolloffFactor);
		}
	}

	void Play(TIME_PRECISION forSec){		
		auto& am = AudioManager::GetInstance();
		am.RegisterAudioEx(mSelfPtr.lock());
		if (!IsPlaying()){
			if (mCurAudioId != INVALID_AUDIO_ID){
				am.StopWithFadeOut(mCurAudioId, 0.5f);
				mCurAudioId = INVALID_AUDIO_ID;
			}			
			
			if (forSec > 0)
				mRequestedTime = forSec;
			else
				mRequestedTime = mInitialRequestedTime;
			if (mDurations[AudioType::Loop] == 0.f && forSec > mDurations[AudioType::Start] + mDurations[AudioType::End]){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("No loop sound is specified. Audio will finish quicker than"
					" you requested. start: %s, end: %s",
					mAudioBuffers[AudioType::Start].c_str(), mAudioBuffers[AudioType::End].c_str()).c_str());
			}
			mStartTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			mPlayingTime = 0;
			
			if (!mAudioBuffers[AudioType::Start].empty()){
				mCurPlaying = AudioType::Start;
				mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::Start].c_str(), mProperty);
			}
			else if (!mAudioBuffers[AudioType::Loop].empty()){
				mCurPlaying = AudioType::Loop;
				mCurAudioId = am.PlayAudioWithFadeIn(mAudioBuffers[AudioType::Loop].c_str(), mProperty, mDurations[AudioType::Loop] * 0.1f);
				mLoopStartedTime = 0;
			}
			else if (!mAudioBuffers[AudioType::End].empty()){
				mCurPlaying = AudioType::End;
				mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::End].c_str(), mProperty);
			}
		}
		else{
			Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioex already");
			mReservedRequestTime = forSec;
			// delete FadeOut manipulator
			am.SetGainSmooth(mCurAudioId, 1.0f, 0.5f);
		}
	}

	void SetRequestTime(float requestTime){
		mRequestedTime = requestTime;
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
		return mCurAudioId != INVALID_AUDIO_ID && mCurPlaying != AudioType::Num;
	}

	// audio thread
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
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType End");
				}
				else{
					mCurPlaying = AudioType::Num;
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType Num");
				}
			}
			// start finished case
			else if (mDurations[AudioType::Start] - mPlayingTime <= 0.05f){
				if (!mAudioBuffers[AudioType::Loop].empty()){
					mCurPlaying = AudioType::Loop;
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType Loop");
					mCurAudioId = am.PlayAudio(mAudioBuffers[AudioType::Loop].c_str(), mProperty);
					mLoopStartedTime = mPlayingTime;
				}
				else if (!mAudioBuffers[AudioType::End].empty()){
					mCurPlaying = AudioType::End;
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType End");
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
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType End");
				}
				else{
					mCurPlaying = AudioType::Num;
					Logger::Log(FB_DEFAULT_LOG_ARG, "(info) audioType Num");
				}
			}
			else{
				auto played = mPlayingTime - mLoopStartedTime;				
				if (played / mDurations[AudioType::Loop] > 0.9f){
					auto left = mDurations[AudioType::Loop] - played;					
					left = std::max(0.f, left);
					if (mCurAudioId != INVALID_AUDIO_ID)
						am.StopWithFadeOut(mCurAudioId, left);
					mCurAudioId = am.PlayAudioWithFadeIn(mAudioBuffers[AudioType::Loop].c_str(), mProperty, left);
					if (mCurAudioId == INVALID_AUDIO_ID){
						Logger::Log(FB_ERROR_LOG_ARG, "(info) Failed to play loop sound.");
					}
					if (mProperty.mGain < 0.5f){
						am.DeleteManipulator(mCurAudioId, AudioManipulatorType::SmoothGain);
						am.SetGain(mCurAudioId, mProperty.mGain, false);
					}
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

		return mCurAudioId == INVALID_AUDIO_ID && mCurPlaying==AudioType::Num; // finished.
	}

	// internal
	void OnFinish(AudioId id){
		if (mCurAudioId == id){			
			mCurAudioId = INVALID_AUDIO_ID;			
		}
	}
};
//---------------------------------------------------------------------------
AudioExPtr AudioEx::Create(){
	AudioExPtr p(new AudioEx(), [](AudioEx* obj){delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}

AudioEx::AudioEx()
	: mImpl(new Impl())
{

}

AudioEx::~AudioEx(){

}

void AudioEx::SetAudioExFile(const char* audioEx){
	mImpl->SetAudioExFile(audioEx);
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

void AudioEx::SetGain(float gain){
	mImpl->SetGain(gain);
}

void AudioEx::SetGainSmooth(float gain, float inSec){
	mImpl->SetGainSmooth(gain, inSec);
}

void AudioEx::SetRolloffFactor(float rolloffFactor) {
	mImpl->SetRolloffFactor(rolloffFactor);
}

void AudioEx::Play(TIME_PRECISION forSec) {
	mImpl->Play(forSec);
}

void AudioEx::SetRequestTime(float requestTime){
	mImpl->SetRequestTime(requestTime);
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

