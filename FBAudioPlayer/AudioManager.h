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
#include "AudioManipulatorType.h"
#include "AudioSourceStatus.h"
#include "AudioSourceType.h"
#include "FBCommonHeaders/VectorMap.h"

typedef unsigned int ALuint;
namespace fb{
	FB_DECLARE_SMART_PTR_STRUCT(AudioSource);
	FB_DECLARE_SMART_PTR(AudioEx);
	FB_DECLARE_SMART_PTR(AudioManager);
	class FB_DLL_AUDIOPLAYER AudioManager{
		friend void eos_callback(void *userData, ALuint source);
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioManager);		
		AudioManager();
		~AudioManager();

	public:
		typedef std::function< void(AudioId) > CallbackFunction;
		static AudioManagerPtr Create();
		static AudioManager& GetInstance();
		bool Init();
		void Deinit();

		void Update(TIME_PRECISION dt);
		
		// fb audio
		AudioId PlayFBAudio(const char* fbAudioPath, AudioSourceType::Enum sourceType = AudioSourceType::Sound);
		AudioId PlayFBAudio(const char* fbAudioPath, const Vec3Tuple& pos);

		// no position.
		AudioId PlayAudio(const char* path, AudioSourceType::Enum sourceType = AudioSourceType::Sound);
		// position or no position
		AudioId PlayAudio(const char* path, const AudioProperty& prop);
		// position
		AudioId PlayAudio(const char* path, const Vec3Tuple& pos);		
		
		/** Start an audio smoothly.
		if \a sec is longer than the audio length, \a sec will be replaced by the audio length.
		*/
		AudioId PlayAudioWithFadeIn(const char* path, const AudioProperty& prop, TIME_PRECISION inSec);
		bool StopAudio(AudioId id);
		TIME_PRECISION GetAudioLength(const char* path);
		TIME_PRECISION GetAudioLength(AudioId id);
		FunctionId RegisterEndCallback(AudioId id, CallbackFunction callback);
		void UnregisterEndCallback(AudioId id);
		/** Stop the audio smoothly.
		if the remaining time of the audio is less than \a sec, the audio will stop earlier than requested time.
		*/
		bool StopWithFadeOut(AudioId id, TIME_PRECISION sec);
		TIME_PRECISION GetAudioLeftTime(AudioId id);
		bool SetPosition(AudioId id, float x, float y, float z);
		/// if true, the audio position is relative to the listener.
		/// Audio which has {0, 0, 0} position will be played always at the listner position if 
		/// this flag is set.
		/// default is false.
		bool SetRelative(AudioId id, bool relative);
		void SetListenerPosition(const Vec3Tuple& pos);
		bool SetReferenceDistance(AudioId id, float distance);
		bool SetRolloffFactor(AudioId id, float factor);
		bool SetOffsetInSec(AudioId id, float sec);
		/// \a checkManipulator when true, if fade-in or fade-out manipulator found, this function
		/// will set the gain value to the manipulator.
		bool SetGain(AudioId id, float gain, bool checkManipulator);
		bool SetGainSmooth(AudioId id, float gain, float inSec);
		float GetGain(AudioId id) const;
		float GetGainFromFBAudio(const char* fbaudioPath);

		bool SetLoop(AudioId id, bool loop);
		bool GetLoop(AudioId id) const;
		bool SetMaxGain(AudioId id, float maxGain);
		float GetMaxGain(AudioId id);

		void RegisterAudioEx(AudioExPtr audioex);
		bool IsRegisteredAudioEx(AudioExPtr audioex);

		void UnregisterEndCallbackForAudio(AudioId id);
		void UnregisterEndCallbackFunc(FunctionId funcId);

		void DeleteManipulator(AudioId id, AudioManipulatorType::Enum type);

		typedef std::unordered_map<AudioId, AudioSourcePtr> AudioSources;
		/// only for FBAudioDebuffer
		struct AudioDebugData{
			float mDistPerRef;
			float mGain;
			Vec3Tuple mPosition;
			AudioSourceStatus::Enum mStatus;
			std::string mFilePath;
		};
		void GetAudioList(std::vector<AudioDebugData>& list) const;

		unsigned GetNumGenerated() const;
		bool IsValidSource(AudioId id) const;

		/// Internal only.
		bool AudioThreadFunc();

		void SetMasterGain(float gain);
		float GetMasterGain() const;

		void OnGainOptionChanged();

		void SetSoundGain(float gain);
		float GetSoundGain() const;

		void SetEnabled(bool enabled);		
	};
}