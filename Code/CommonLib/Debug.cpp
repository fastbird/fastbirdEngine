#include <CommonLib/StdAfx.h>
#include "Timer.h"
namespace fastbird
{
void DebugOutput(const char* fmt, ...)
{
	static char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, 2048, fmt, args);
	va_end(args);
		
	strcat_s(buf, 2048, "\n");
		
	OutputDebugString(buf);
}

void FBDebugBreak(){
#ifdef _DEBUG
	DebugBreak();
#else
#endif
}
}