#pragma once

namespace fastbird
{
	namespace ObjectEvent
	{
		enum Enum
		{
			InterpolationDone,
			AfterInterpolation,
		};
	}
	class IObjectEventListener
	{
		// object type
		// SkySphere : SKSP
		virtual bool OnEvent(ObjectEvent::Enum e, DWORD objectType, void*)=0;
	};
}