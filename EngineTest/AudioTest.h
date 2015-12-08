#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioTest);
	class AudioTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioTest);
		AudioTest();
		~AudioTest();

	public:
		static AudioTestPtr Create();
	};
}