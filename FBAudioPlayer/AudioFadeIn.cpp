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