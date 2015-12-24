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
namespace fb{
	struct AudioProperty;
	FB_DECLARE_SMART_PTR(AudioExFacade);
	class FB_DLL_ENGINEFACADE AudioExFacade{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioExFacade);
		AudioExFacade();
		~AudioExFacade();

	public:
		static AudioExFacadePtr Create();

		void SetAudioExFile(const char* fbaudioEx);
		void SetAudio(const char* startPath, const char* loopPath, const char* endPath);
		void Play(const char* filepath, float forSec);
		void Play(const char* startPath, const char* loopPath, const char* endPath, float forSec);
		void Play(float forSec);
		/// Set play duration but do not play now.
		void SetRequestTime(float requestTime);
		void Stop();
		void Stop(float fadeOutTime, bool playEnd);

		void SetPosition(const Vec3& pos);
		void SetGain(float gain);
		void SetGainSmooth(float gain, float inSec);
		void SetReferenceDistance(float dist);
		bool IsPlaying() const;
	};
}