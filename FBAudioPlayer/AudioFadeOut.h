#pragma once
#include "IAudioManipulator.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioFadeOut);
	class AudioFadeOut : public IAudioManipulator{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioFadeOut);
		AudioFadeOut(AudioId id, TIME_PRECISION time);
		~AudioFadeOut();

	public:
		static AudioFadeOutPtr Create(AudioId id, TIME_PRECISION time);

		//---------------------------------------------------------------------------
		// IAudioManipulator
		//---------------------------------------------------------------------------
		/// returns true when faded out completely.
		bool Update(TIME_PRECISION dt);
	};
}