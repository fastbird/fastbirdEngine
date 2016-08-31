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
#include "AudioSourceStatus.h"
#include "AudioSourceType.h"
namespace fb{
	struct AudioProperty;
	FB_DECLARE_SMART_PTR_STRUCT(AudioBuffer);
	FB_DECLARE_SMART_PTR_STRUCT(AudioSource);
	struct FB_DLL_AUDIOPLAYER AudioSource{
	private:
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioSource);
		AudioSource(AudioId audioId, AudioSourceType::Enum type);
		~AudioSource();

	public:
		static AudioSourcePtr Create(AudioId audioId, AudioSourceType::Enum type);
		static float sMasterGain;
		static float sSoundGain;
		static float sMusicGain;

		const AudioId& GetAudioId() const;
		void SetAudioBuffer(AudioBufferPtr buffer);
		AudioBufferPtr GetAudioBuffer() const;
		void SetALAudioSource(unsigned src);
		unsigned GetALAudioSource() const;
		void SetStatus(AudioSourceStatus::Enum status);
		AudioSourceStatus::Enum GetStatus();
		void SetProperty(const AudioProperty& prop);
		void ApplyProp();
		void ApplyRemainedTime();
		bool IsPlaying() const;
		bool Update(float dt);
		void SetPosition(float x, float y, float z);
		Vec3Tuple GetPosition() const;
		void SetRelative(bool relative);
		bool GetRelative() const;
		void SetReferenceDistance(float distance);
		float GetReferenceDistance();
		void SetRollOffFactor(float factor);
		void SetOffsetInSec(float sec);
		void SetGain(float gain);
		float GetGain() const;
		void OnGainOptionChanged();
		float GetLeftTime() const;
		float GetLength() const;
		void SetDistPerRef(float distPerRef);
		float GetDistPerRef() const;
		void SetLoop(bool loop);
		bool GetLoop() const;
		void SetMaxGain(float maxGain);
		float GetMaxGain() const;
		size_t GetNumSimultaneous() const;
		float GetSimultaneousCheckRange() const;
	};
}