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

#pragma once
#include "FBCommonHeaders/Types.h"
#include "AudioProperty.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioEx);
	class FB_DLL_AUDIOPLAYER AudioEx{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioEx);
		AudioEx();
		~AudioEx();

	public:
		static AudioExPtr Create();
		void SetAudioExFile(const char* audioEx);
		void SetStartLoopEnd(const char* start, const char* loop, const char* end);	
		void SetPosition(float x, float y, float z);
		const Vec3Tuple& GetPosition() const;
		void SetRelative(bool relative);
		void SetReferenceDistance(float referenceDistance);
		void SetGain(float gain);
		void SetGainSmooth(float gain, float inSec);
		void SetRolloffFactor(float rolloffFactor);
		void Play(TIME_PRECISION forSec);
		void SetRequestTime(float requestTime);
		void ExtendTime(TIME_PRECISION forSec);
		void Stop(float fadeOutTime, bool playEnd);
		bool IsPlaying() const;
		// return true when finished.
		bool Update();

		// internal
		void OnFinish(AudioId id);
	};
}