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
#include "AudioExFacade.h"
#include "FBAudioPlayer/AudioEx.h"
using namespace fb;
class AudioExFacade::Impl{
public:
	AudioExPtr mAudio;

	Impl(const AudioProperty& prop){
		mAudio = AudioEx::Create(prop);
	}

	void SetAudio(const char* startPath, const char* loopPath, const char* endPath){
		mAudio->SetStartLoopEnd(startPath, loopPath, endPath);
	}

	void Play(const char* startPath, const char* loopPath, const char* endPath, float forSec){
		mAudio->SetStartLoopEnd(startPath, loopPath, endPath);
		mAudio->Play(forSec);
	}

	void Play(float forSec){
		mAudio->Play(forSec);
	}

	void SetRequestTime(float requestTime){
		mAudio->SetRequestTime(requestTime);
	}

	void Stop(){
		mAudio->Stop(0.5f, true);
	}

	void Stop(float fadeOutTime, bool playEnd){
		mAudio->Stop(fadeOutTime, playEnd);
	}

	void SetPosition(const Vec3& pos){
		mAudio->SetPosition(pos.x, pos.y, pos.z);
	}

	void SetGain(float gain){
		mAudio->SetGain(gain);
	}

	void SetGainSmooth(float gain, float inSec){
		mAudio->SetGainSmooth(gain, inSec);
	}

	void SetReferenceDistance(float dist){
		mAudio->SetReferenceDistance(dist);
	}
};

//---------------------------------------------------------------------------
AudioExFacadePtr AudioExFacade::Create(const AudioProperty& prop){
	return AudioExFacadePtr(new AudioExFacade(prop), [](AudioExFacade* obj){delete obj; });		
}

AudioExFacade::AudioExFacade(const AudioProperty& prop)
	: mImpl(new Impl(prop)){

}

AudioExFacade::~AudioExFacade(){

}

void AudioExFacade::SetAudio(const char* startPath, const char* loopPath, const char* endPath){
	mImpl->SetAudio(startPath, loopPath, endPath);
}

void AudioExFacade::Play(const char* filepath, float forSec){
	mImpl->Play("", filepath, "", forSec);
}

void AudioExFacade::Play(const char* startPath, const char* loopPath, const char* endPath, float forSec){
	mImpl->Play(startPath, loopPath, endPath, forSec);
}

void AudioExFacade::Play(float forSec){
	mImpl->Play(forSec);
}

void AudioExFacade::SetRequestTime(float requestTime){
	mImpl->SetRequestTime(requestTime);
}

void AudioExFacade::Stop(){
	mImpl->Stop();
}

void AudioExFacade::Stop(float fadeOutTime, bool playEnd){
	mImpl->Stop(fadeOutTime, playEnd);
}

void AudioExFacade::SetPosition(const Vec3& pos){
	mImpl->SetPosition(pos);
}

void AudioExFacade::SetGain(float gain){
	mImpl->SetGain(gain);
}

void AudioExFacade::SetGainSmooth(float gain, float inSec){
	mImpl->SetGainSmooth(gain, inSec);
}

void AudioExFacade::SetReferenceDistance(float dist){
	mImpl->SetReferenceDistance(dist);
}