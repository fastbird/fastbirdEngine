#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(VideoTest);
	class VideoTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(VideoTest);
		VideoTest();
		~VideoTest();

	public:
		static VideoTestPtr Create();
	};
}