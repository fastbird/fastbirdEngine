#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(LuaTest);
	class LuaTest {
		FB_DECLARE_PIMPL_NON_COPYABLE(LuaTest);
		LuaTest();
		~LuaTest();

	public:
		static LuaTestPtr Create();
	};
}

