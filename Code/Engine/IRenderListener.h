#pragma once
namespace fastbird{
	class IRenderListener
	{
	public:
		virtual void BeforeUIRendering(HWND_ID hwndId) {}
		virtual void AfterUIRendered(HWND_ID hwndId) {}
		virtual void BeforeDebugHudRendered(HWND_ID hwndId) {}
		virtual void AfterDebugHudRendered(HWND_ID hwndId) {}
		virtual void OnResolutionChanged(HWND_ID hwndId) {}
	};
}