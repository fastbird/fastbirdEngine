#include <Engine/StdAfx.h>
#include <Engine/DllMain.h>
#include <Engine/IEngine.h>
#include <Engine/GlobalEnv.h>

extern "C"
{
	CLASS_DECLSPEC_ENGINE fastbird::GlobalEnv* gFBEnv = 0;

	//-------------------------------------------------------------------------
	CLASS_DECLSPEC_ENGINE fastbird::IEngine* _cdecl Create_fastbird_Engine()
	{
		static bool engineCreated = false;
		if (engineCreated)
		{
			assert(0);
		}

		engineCreated = true;
		fastbird::IEngine* pEngine = fastbird::IEngine::CreateInstance();
		return pEngine;
	}
	
	//-------------------------------------------------------------------------
	CLASS_DECLSPEC_ENGINE void _cdecl Destroy_fastbird_Engine()
	{
		gFBEnv->mExiting = true;		
		if (gFBEnv)
			fastbird::IEngine::DeleteInstance(gFBEnv->pEngine);
	}

	//-------------------------------------------------------------------------
	CLASS_DECLSPEC_ENGINE void _cdecl OutputDebug(const char* szFmt, ...)
	{
		static char buf[2048];

		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		strcat_s(buf, 2048, "\n");

		fastbird::IEngine::Log(buf);
	}
}