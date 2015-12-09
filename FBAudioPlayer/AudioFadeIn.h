#pragma once
#include "IAudioManipulator.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioFadeIn);
	class AudioFadeIn : public IAudioManipulator{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioFadeIn);
		AudioFadeIn(AudioId id, TIME_PRECISION time, float targetGain);
		~AudioFadeIn();

	public:
		static AudioFadeInPtr Create(AudioId id, TIME_PRECISION time, float targetGain);

		//---------------------------------------------------------------------------
		// IAudioManipulator
		//---------------------------------------------------------------------------
		/// returns true when faded in completely.
		bool Update(TIME_PRECISION dt);
	};
}