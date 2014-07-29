#include <UI/StdAfx.h>
#include <UI/IUIManager.h>

namespace fastbird
{	
	void Log(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		gEnv->pEngine->Log(buf);
	}
	void Error(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		gEnv->pEngine->Error(buf);
	}
}