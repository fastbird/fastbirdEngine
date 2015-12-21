#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR_STRUCT(AudioBuffer);
	struct AudioBuffer{
		std::string mFilepath;
		INT64 mLastAccessed;
		unsigned mBuffer; // ALuint
		unsigned mReferences;
		TIME_PRECISION mLength;

		AudioBuffer();
		~AudioBuffer();
	};
}