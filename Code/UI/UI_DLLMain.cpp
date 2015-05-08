#include <UI/StdAfx.h>
#include <UI/UIManager.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IScriptSystem.h>

fastbird::IUIManager* gFBUIManager = 0;

extern "C"
{
	//-------------------------------------------------------------------------
	__declspec(dllexport) fastbird::IUIManager* _cdecl Create_fastbird_UIManager(fastbird::GlobalEnv* genv)
	{
		assert(genv);
		if (!genv)
			return 0;

		gFBEnv = genv;
		if (gFBEnv->pUIManager)
			return gFBEnv->pUIManager;

		gFBUIManager = FB_NEW(fastbird::UIManager)(gFBEnv->pScriptSystem->GetLuaState());
		return gFBEnv->pUIManager;
	}

	//-------------------------------------------------------------------------
	__declspec(dllexport) void _cdecl Destroy_fastbird_UIManager()
	{
		FB_SAFE_DEL(gFBUIManager);
		gFBUIManager = 0;

#ifdef USING_FB_MEMORY_MANAGER
		fastbird::FBReportMemoryForModule();
#endif
	}
}