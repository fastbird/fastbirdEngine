#pragma once
#include <Engine/RenderToTexture.h>
#include <d3d11.h>
namespace fastbird
{
	class RenderToTextureD3D11 : public RenderToTexture
	{
	public:
		RenderToTextureD3D11() {}
		virtual ~RenderToTextureD3D11() {}

		virtual void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap);
		virtual void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap);

	private:

	};
}