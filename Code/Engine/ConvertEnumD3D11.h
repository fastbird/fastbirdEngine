#pragma once
#include <Engine/RendererEnums.h>
#include <d3d11.h>
namespace fastbird
{
	//------------------------------------------------------------------------
	inline D3D11_USAGE ConvertEnumD3D11(BUFFER_USAGE usage)
	{
		return D3D11_USAGE(usage);
	}

	//------------------------------------------------------------------------
	inline D3D11_CPU_ACCESS_FLAG ConvertEnumD3D11(BUFFER_CPU_ACCESS_FLAG flag)
	{
		return D3D11_CPU_ACCESS_FLAG(flag);
	}

	//------------------------------------------------------------------------
	inline unsigned int ConvertEnumD3D11(MAP_FLAG flag)
	{
		switch(flag)
		{
		case 0:
			return 0;

		case MAP_FLAG_DO_NOT_WAIT:
			return D3D11_MAP_FLAG_DO_NOT_WAIT;

		default:
			assert(0);
			return 0;
		}
	}

	//------------------------------------------------------------------------
	inline D3D11_MAP ConvertEnumD3D11(MAP_TYPE type)
	{
		return D3D11_MAP(type);
	}

	//------------------------------------------------------------------------
	inline D3D11_FILL_MODE ConvertEnumD3D11(FILL_MODE mode)
	{
		return D3D11_FILL_MODE(mode);
	}

	//------------------------------------------------------------------------
	inline D3D11_CULL_MODE ConvertEnumD3D11(CULL_MODE mode)
	{
		return D3D11_CULL_MODE(mode);
	}

	//------------------------------------------------------------------------
	inline D3D11_FILTER ConvertEnumD3D11(TEXTURE_FILTER filter)
	{
		return D3D11_FILTER(filter);
	}

	//------------------------------------------------------------------------
	inline D3D11_TEXTURE_ADDRESS_MODE ConvertEnumD3D11(TEXTURE_ADDRESS_MODE mode)
	{
		return D3D11_TEXTURE_ADDRESS_MODE(mode);
	}

	//------------------------------------------------------------------------
	inline D3D11_COMPARISON_FUNC ConvertEnumD3D11(COMPARISON_FUNC func)
	{
		return D3D11_COMPARISON_FUNC(func);
	}

	//------------------------------------------------------------------------
	inline D3D11_BLEND ConvertEnumD3D11(BLEND b)
	{
		return D3D11_BLEND(b);
	}

	//------------------------------------------------------------------------
	inline D3D11_BLEND_OP ConvertEnumD3D11(BLEND_OP bo)
	{
		return D3D11_BLEND_OP(bo);
	}

	//------------------------------------------------------------------------
	inline D3D11_COLOR_WRITE_ENABLE ConvertEnumD3D11(COLOR_WRITE_MASK c)
	{
		return D3D11_COLOR_WRITE_ENABLE(c);
	}

	//------------------------------------------------------------------------
	inline D3D11_DEPTH_WRITE_MASK ConvertEnumD3D11(DEPTH_WRITE_MASK mask)
	{
		return D3D11_DEPTH_WRITE_MASK(mask);
	}

	//------------------------------------------------------------------------
	inline D3D11_STENCIL_OP ConvertEnumD3D11(STENCIL_OP op)
	{
		return D3D11_STENCIL_OP(op);
	}

	//------------------------------------------------------------------------
	inline DXGI_FORMAT ConvertEnumD3D11(PIXEL_FORMAT format)
	{
		return DXGI_FORMAT(format);
	}

	inline int PixelFormat2Bytes(PIXEL_FORMAT format)
	{
		if (format>=PIXEL_FORMAT_R32G32B32A32_TYPELESS && format <= PIXEL_FORMAT_R32G32B32A32_SINT)
			return 16;
		else if (format>=PIXEL_FORMAT_R32G32B32_TYPELESS && format <= PIXEL_FORMAT_R32G32B32_SINT)
			return 12;
		else if (format>=PIXEL_FORMAT_R16G16B16A16_TYPELESS && format <= PIXEL_FORMAT_X32_TYPELESS_G8X24_UINT)
			return 8;
		else if (format>=PIXEL_FORMAT_R10G10B10A2_TYPELESS && format <= PIXEL_FORMAT_X24_TYPELESS_G8_UINT)
			return 4;
		else if (format >= PIXEL_FORMAT_R8G8_TYPELESS && format <= PIXEL_FORMAT_R16_SINT)
			return 2;
		else if (format >= PIXEL_FORMAT_R8_TYPELESS && format <= PIXEL_FORMAT_A8_UNORM)
			return 1;
		else if (format >=PIXEL_FORMAT_R9G9B9E5_SHAREDEXP && format <= DXGI_FORMAT_G8R8_G8B8_UNORM )
			return 4;
		else if (format >=PIXEL_FORMAT_B5G6R5_UNORM  && format <= PIXEL_FORMAT_B5G5R5A1_UNORM)
			return 2;
		else if (format >= PIXEL_FORMAT_B8G8R8A8_UNORM && format <= PIXEL_FORMAT_B8G8R8X8_UNORM_SRGB)
			return 4;
		assert(0 && "Unknown bytes.");
		return 0;
	}

	inline D3D11_PRIMITIVE_TOPOLOGY ConvertEnumD3D11(PRIMITIVE_TOPOLOGY topology){
		switch (topology){
		case PRIMITIVE_TOPOLOGY_POINTLIST:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PRIMITIVE_TOPOLOGY_LINELIST:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;			
		case PRIMITIVE_TOPOLOGY_LINESTRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		case PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:
			gFBEnv->pEngine->Log(FB_DEFAULT_DEBUG_ARG, "Undefined primitive topology!");
			assert(0);
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}
}