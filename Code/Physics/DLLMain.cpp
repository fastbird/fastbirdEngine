#include <Physics/StdAfx.h>

namespace fastbird
{
	void Log(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		OutputDebugString(buf);
		OutputDebugString("\n");
		std::cout << buf << std::endl;
	}
	void Error(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		OutputDebugString(buf);
		OutputDebugString("\n");
		std::cout << buf << std::endl;
	}
}