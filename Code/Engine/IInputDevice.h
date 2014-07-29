#pragma once

#include <CommonLib/SmartPtr.h>

namespace fastbird
{
	class IInputDevice : public ReferenceCounter
	{
	public:
		virtual void EndFrame() = 0;
		virtual bool IsValid() const = 0;
		virtual void Invalidate() = 0;
	};
}