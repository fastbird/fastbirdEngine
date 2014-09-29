#pragma once
namespace fastbird
{
	class IEngine;
	class IRenderer;
	class Renderer;
	class IConsole;
	class IScriptSystem;
	class Timer;

	struct GlobalEnv
	{
		GlobalEnv()
		{
			pEngine = 0;
			pRenderer = 0;
			_pInternalRenderer = 0;
			pConsole = 0;
			mFrameCounter = 0;
			pScriptSystem = 0;
			mExiting = false;
			pTimer = 0;
			mRenderPass = 0;
			mGodRayInScreen = false;
			mSilouetteRendered = false;
		}
		IEngine* pEngine;
		IRenderer* pRenderer;
		Renderer* _pInternalRenderer;
		IConsole* pConsole;
		IScriptSystem* pScriptSystem;
		unsigned mFrameCounter;
		bool mExiting;
		Timer* pTimer;
		int mRenderPass; // enum RENDER_PASS
		bool mGodRayInScreen;
		bool mSilouetteRendered;
	};
}

extern "C" CLASS_DECLSPEC_ENGINE fastbird::GlobalEnv* gFBEnv;
