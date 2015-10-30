#pragma once
#include <CommonLib/SmartPtr.h>
#include <Engine/RendererEnums.h>
#include <Engine/RenderPipeline.h>
namespace fastbird
{

	typedef unsigned RenderTargetId;

	class ITexture;
	class IScene;
	class ICamera;
	class ILight;
	class IMouse;
	class IKeyboard;
	class IRenderTargetListener;

	class IRenderTarget : public ReferenceCounter
	{
	public:
		virtual const Vec2I& GetSize() const = 0;
		virtual bool CheckOptions(const RenderTargetParam& param) = 0;
		virtual RenderPipeline& GetRenderPipeline() const = 0;

		virtual void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap) = 0;
		virtual void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap) = 0;
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil) = 0;
		virtual void SetClearColor(const Color& color) = 0;
		virtual void SetClearDepthStencil(float z, UINT8 stencil) = 0;
		virtual void SetScene(IScene*) = 0;
		virtual void ReplaceCamera(ICamera* cam) = 0;
		virtual IScene* GetScene() const = 0;
		virtual IScene* GetOriginalScene() const = 0;
		virtual void SetSceneOverride(IScene* scene) = 0;
		virtual IScene* GetSceneOverride() const = 0;
		virtual void LockSceneOverride(bool lock) = 0;

		virtual IScene* CreateScene() = 0;
		virtual ICamera* GetCamera() const = 0;
		virtual ICamera* GetOrCreateOverridingCamera() = 0;
		virtual void RemoveOverridingCamera() = 0;
		virtual ITexture* GetRenderTargetTexture() = 0;
		virtual ITexture* GetDepthStencilTexture() = 0;
		virtual void Bind(size_t face = 0) = 0;
		virtual void BindTargetOnly(bool hdr) = 0;
		virtual bool Render(size_t face = 0) = 0;
		virtual void Unbind() = 0;
		// for every frame render
		virtual void SetEnable(bool enable) = 0;
		virtual bool GetEnable() const = 0;

		virtual void SetEnvTexture(ITexture* texture) = 0;
		virtual void SetUsePool(bool usePool) = 0;
		virtual bool GetUsePool() const = 0;

		// simple camera operation
		virtual void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard) = 0;

		virtual ICamera* GetLightCamera() const = 0;

		//-------------------------------------------------------------------
		// Post processors
		//-------------------------------------------------------------------
		virtual void SetGlowRenderTarget()=0;
		virtual void UnSetGlowRenderTarget()=0;
		virtual void BindDepthTexture(bool set) = 0;
		virtual void SetHDRTarget() = 0;
		virtual void SetSmallSilouetteBuffer() = 0;
		virtual void SetBigSilouetteBuffer() = 0;

		virtual void DrawOnEvent(bool set) = 0;
		virtual void TriggerDrawEvent() = 0;

		virtual void AddListener(IRenderTargetListener* listener) = 0;
		virtual void RemoveListener(IRenderTargetListener* listener) = 0;
	};
}