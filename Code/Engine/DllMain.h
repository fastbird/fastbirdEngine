#pragma once

#include <Engine/IEngine.h>

namespace fastbird
{
	class IEngine;
}
extern "C"
{
	CLASS_DECLSPEC_ENGINE fastbird::IEngine* _cdecl Create_fastbird_Engine();
	CLASS_DECLSPEC_ENGINE void _cdecl Destroy_fastbird_Engine();
}