#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(ComputeShaderTest);
	class ComputeShaderTest {
		FB_DECLARE_PIMPL_NON_COPYABLE(ComputeShaderTest);
		ComputeShaderTest();
		~ComputeShaderTest();

	public:
		static ComputeShaderTestPtr Create();
	};
}
