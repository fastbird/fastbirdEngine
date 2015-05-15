#pragma once
namespace fastbird{
	class IRenderListener
	{
	public:
		virtual void BeforeUIRendering(HWND_ID hwndId) {}
		virtual void AfterUIRendered(HWND_ID hwndId) {}
	};
}