#pragma once

#define FB_DEFAULT_DEBUG_ARG "%s(%d): %s() - %s", __FILE__, __LINE__, __FUNCTION__
namespace fastbird
{
// this is only for printing a msg into the DebugOuputWindow.
// and CommonLib internal.
void DebugOutput(const char* fmt, ...);
}