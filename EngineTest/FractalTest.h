#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBRenderer/IRendererObserver.h"
namespace fb {
	FB_DECLARE_SMART_PTR(FractalTest);
	class FractalTest : public IRendererObserver {
		FB_DECLARE_PIMPL_NON_COPYABLE(FractalTest);
		FractalTest();
		~FractalTest();

	public:
		static FractalTestPtr Create();

		virtual void BeforeUIRendering(HWindowId hwndId, HWindow hwnd) OVERRIDE;
		virtual void RenderUI(HWindowId hwndId, HWindow hwnd) OVERRIDE {}
		virtual void AfterUIRendered(HWindowId hwndId, HWindow hwnd) OVERRIDE {}
		virtual void BeforeDebugHudRendering() OVERRIDE {}
		virtual void AfterDebugHudRendered() OVERRIDE {}
		virtual void OnResolutionChanged(HWindowId hwndId, HWindow hwnd) OVERRIDE {}
	};
}