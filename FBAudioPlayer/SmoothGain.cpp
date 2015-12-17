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
#include "SmoothGain.h"
#include "AudioManager.h"
using namespace fb;
class SmoothGain::Impl{
public:
	AudioId mId;
	float mGainPerSec;
	float mInitialGain;
	float mCurGain;
	float mTargetGain;

	Impl(AudioId id, float gainPerSec, float initialGain, float targetGain)
		: mId(id)
		, mGainPerSec(gainPerSec)
		, mInitialGain(initialGain)
		, mTargetGain(targetGain)
	{
		AudioManager::GetInstance().SetGain(id, initialGain, false);
		mCurGain = initialGain;
	}

	bool Update(TIME_PRECISION dt){
		if (mTargetGain > mInitialGain){
			mCurGain += mGainPerSec * dt;
			bool finished = false;
			if (mCurGain >= mTargetGain){
				mCurGain = mTargetGain;
				finished = true;
			}
			auto success = AudioManager::GetInstance().SetGain(mId, mCurGain, false);
			return !success || finished;
		}
		else{
			// if gain is reach to zero, audio will be stopped.
			mCurGain -= mGainPerSec * dt;
			bool finished = false;
			if (mCurGain <= mTargetGain){
				mCurGain = mTargetGain;
				finished = true;
			}
			auto success = AudioManager::GetInstance().SetGain(mId, mCurGain, false);
			return !success || finished;
		}		
	}
};

SmoothGainPtr SmoothGain::Create(AudioId id, float gainPerSec, float initialGain, float targetGain){
	return SmoothGainPtr(new SmoothGain(id, gainPerSec, initialGain, targetGain), [](SmoothGain* obj){ delete obj; });
}

SmoothGain::SmoothGain(AudioId id, float gainPerSec, float initialGain, float targetGain)
	: mImpl(new Impl(id, gainPerSec, initialGain, targetGain))
{

}

SmoothGain::~SmoothGain(){

}

bool SmoothGain::Update(TIME_PRECISION dt){
	return mImpl->Update(dt);
}

AudioId SmoothGain::GetAudioId() const{
	return mImpl->mId;
}

void SmoothGain::OnGainModified(float gain){
	mImpl->mTargetGain = gain;
	mImpl->mInitialGain = AudioManager::GetInstance().GetGain(mImpl->mId);
}

void SmoothGain::SetDuration(float duration){
	mImpl->mGainPerSec = std::abs(mImpl->mTargetGain - mImpl->mInitialGain) / duration;
}