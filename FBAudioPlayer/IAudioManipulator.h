#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(IAudioManipulator);
	class IAudioManipulator{
	public:
		/// returns true when finished.
		virtual bool Update(TIME_PRECISION dt) = 0;
	};
}