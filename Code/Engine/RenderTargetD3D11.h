#pragma once
#include <Engine/RenderTarget.h>
#include <d3d11.h>
namespace fastbird
{
	class RenderTargetD3D11 : public RenderTarget
	{
	public:
		RenderTargetD3D11() {}
		virtual ~RenderTargetD3D11() {}

		virtual void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap);
		virtual void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap);

	private:

	};
}