#pragma once
namespace fastbird
{
	class IEngine;
	class IRenderer;
	class IConsole;
	class IScriptSystem;
	class Timer;

	struct GlobalEnv
	{
		GlobalEnv()
		{
			pEngine = 0;
			pRenderer = 0;
			pConsole = 0;
			mFrameCounter = 0;
			pScriptSystem = 0;
			mExiting = false;
			pTimer = 0;
		}
		IEngine* pEngine;
		IRenderer* pRenderer;
		IConsole* pConsole;
		IScriptSystem* pScriptSystem;
		unsigned mFrameCounter;
		bool mExiting;
		Timer* pTimer;
	};
}

extern "C" CLASS_DECLSPEC_ENGINE fastbird::GlobalEnv* gFBEnv;
