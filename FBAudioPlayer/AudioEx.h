#pragma once
#include "FBCommonHeaders/Types.h"
#include "AudioProperty.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioEx);
	class FB_DLL_AUDIOPLAYER AudioEx{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioEx);
		AudioEx(const AudioProperty& prop);
		~AudioEx();

	public:
		static AudioExPtr Create(const AudioProperty& prop);
		void SetStartLoopEnd(const char* start, const char* loop, const char* end);	
		void SetPosition(float x, float y, float z);
		void SetRelative(bool relative);
		void SetReferenceDistance(float referenceDistance);
		void SetRolloffFactor(float rolloffFactor);
		void Play(TIME_PRECISION forSec);
		void Stop(float fadeOutTime, bool playEnd);
		bool IsPlaying() const;
		// return true when finished.
		bool Update();

		// internal
		void OnFinish(AudioId id);
	};
}