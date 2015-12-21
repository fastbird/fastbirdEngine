#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(AudioDebugger);
	class FB_DLL_AUDIODEBUGGER AudioDebugger{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioDebugger);
		AudioDebugger();
		~AudioDebugger();

	public:
		static AudioDebuggerPtr Create();
		void Render();
	};
}