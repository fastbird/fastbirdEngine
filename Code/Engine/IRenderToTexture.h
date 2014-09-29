#pragma once
#include <CommonLib/SmartPtr.h>
#include <Engine/Renderer/RendererEnums.h>
namespace fastbird
{
	class ITexture;
	class IScene;
	class ICamera;
	class ILight;
	class IRenderToTexture : public ReferenceCounter
	{
	public:
		virtual void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap) = 0;
		virtual void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap) = 0;
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil) = 0;
		virtual IScene* GetScene() const = 0;
		virtual ICamera* GetCamera() const = 0;
		virtual ILight* GetLight() = 0;
		virtual ITexture* GetRenderTargetTexture() = 0;
		virtual ITexture* GetDepthStencilTexture() = 0;
		virtual void Render(size_t face = 0) = 0;
		// for every frame render
		virtual void SetEnable(bool enable) = 0;
		virtual bool GetEnable() const = 0;

		// simple camera operation
		virtual void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard) = 0;
	};
}