#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(SkyBoxTest);
	class SkyBoxTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(SkyBoxTest);
		SkyBoxTest();
		~SkyBoxTest();

	public:
		static SkyBoxTestPtr Create();
	};
}