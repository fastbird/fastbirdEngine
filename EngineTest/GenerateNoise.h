#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(GenerateNoise);
	class GenerateNoise {
		FB_DECLARE_PIMPL_NON_COPYABLE(GenerateNoise);
		GenerateNoise();
		~GenerateNoise();

	public:
		static GenerateNoisePtr Create();
	};
}