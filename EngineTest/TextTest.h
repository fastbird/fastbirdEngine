#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(TextTest);
	class TextTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(TextTest);
		TextTest();
		~TextTest();

	public:
		static TextTestPtr Create();
		void Update();
	};
}