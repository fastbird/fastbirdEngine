#include <Engine/StdAfx.h>
#include <Engine/DllMain.h>
#include <Engine/IEngine.h>
#include <Engine/GlobalEnv.h>

extern "C"
{
	fastbird::GlobalEnv* gFBEnv = 0;

	//-------------------------------------------------------------------------
	__declspec(dllexport) fastbird::IEngine* _cdecl Create_fastbird_Engine()
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
	__declspec(dllexport) void _cdecl Destroy_fastbird_Engine()
	{
		if (gFBEnv)
		{
			gFBEnv->mExiting = true;
			fastbird::IEngine::DeleteInstance(gFBEnv->pEngine);
		}
	}
}