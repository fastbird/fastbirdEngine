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
typedef unsigned int ALuint;
namespace fb{
	

	FB_DECLARE_SMART_PTR(AudioManager);
	class FB_DLL_AUDIOPLAYER AudioManager{
		friend void eos_callback(void *userData, ALuint source);
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioManager);		
		AudioManager();
		~AudioManager();

		
	public:
		static AudioManagerPtr Create();
		static AudioManager& GetInstance();
		bool Init();
		void Deinit();

		void Update(TIME_PRECISION dt);
		AudioId PlayAudio(const char* path);
		AudioId PlayAudio(const char* path, float x, float y, float z);
		AudioId PlayAudio(const char* path, const AudioProperty& property);
		bool SetPosition(AudioId id, float x, float y, float z);
		/// if true, the audio position is relative to the listener.
		/// Audio which has {0, 0, 0} position will be played always at the listner position if 
		/// this flag is set.
		/// default is false.
		bool SetRelative(AudioId id, bool relative);
		void SetListenerPosition(float x, float y, float z);
		bool SetMaxDistance(AudioId id, float distance);
	};
}