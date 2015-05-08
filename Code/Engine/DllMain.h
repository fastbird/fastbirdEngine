#pragma once

#include <Engine/IEngine.h>

namespace fastbird
{
	class IEngine;
}
extern "C"
{
	__declspec(dllexport) fastbird::IEngine* _cdecl Create_fastbird_Engine();
	__declspec(dllexport) void _cdecl Destroy_fastbird_Engine();
}