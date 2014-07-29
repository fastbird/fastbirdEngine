#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3DEventMarker.h>
#include <CommonLib/Unicode.h>
#include <d3d9.h>

using namespace fastbird;

D3DEventMarker::D3DEventMarker(const char* name)
{
	D3DPERF_BeginEvent(0xffffffff, AnsiToWide(name, strlen(name)));
}

D3DEventMarker::~D3DEventMarker()
{
	D3DPERF_EndEvent();
}