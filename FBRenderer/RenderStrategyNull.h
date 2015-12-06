#pragma once
#include "FBRenderer/IRenderStrategy.h"

#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(RenderStrategyNull);
	class RenderStrategyNull : public IRenderStrategy{
		FB_DECLARE_PIMPL_NON_COPYABLE(RenderStrategyNull);
		RenderStrategyNull();
		~RenderStrategyNull();

	public:
		static RenderStrategyNullPtr Create();

		//-------------------------------------------------------------------
		// IRenderStrategyNull
		//-------------------------------------------------------------------
		void SetScene(IScenePtr scene);
		void SetRenderTarget(RenderTargetPtr renderTarget);
		void UpdateLightCamera();
		void Render(size_t face);
		bool IsHDR() const;
		bool IsGlowSupported();
		CameraPtr GetLightCamera() const;
		bool SetHDRTarget();
		bool SetSmallSilouetteBuffer();
		bool SetBigSilouetteBuffer();
		void GlowRenderTarget(bool bind);
		void DepthTexture(bool bind);
		void OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName);
		TexturePtr GetShadowMap();
	};
}