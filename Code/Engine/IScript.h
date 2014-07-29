#pragma once
#include <CommonLib/SmartPtr.h>
namespace fastbird
{
	class IScript : public ReferenceCounter
	{
		void RegisterFunction();
	};
}