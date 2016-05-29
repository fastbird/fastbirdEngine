/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include "FBRenderer/RendererEnums.h"
namespace fb
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

	inline PIXEL_FORMAT ConvertEnumFB(DXGI_FORMAT format)
	{
		return PIXEL_FORMAT(format);
	}

	inline DXGI_FORMAT ConvertEnumD3D11(INDEXBUFFER_FORMAT format)
	{
		switch (format)
		{
		case INDEXBUFFER_FORMAT_16BIT:
			return DXGI_FORMAT_R16_UINT;
		case INDEXBUFFER_FORMAT_32BIT:
			return DXGI_FORMAT_R32_UINT;
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Unknown index buffer format.");
			return DXGI_FORMAT_R32_UINT;
		}
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
			Logger::Log(FB_ERROR_LOG_ARG, "Undefined primitive topology!");
			assert(0);
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}
}