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
#include "PixelFormats.h"
#include "PrimitiveTopology.h"
namespace fb
{
	enum BUFFER_USAGE
	{
		BUFFER_USAGE_DEFAULT,
		BUFFER_USAGE_IMMUTABLE,
		BUFFER_USAGE_DYNAMIC,
		BUFFER_USAGE_STAGING,

		BUFFER_USAGE_COUNT
	};

	enum BIND_FLAG
	{
		BIND_VERTEX_BUFFER	= 0x1L,
		BIND_INDEX_BUFFER	= 0x2L,
		BIND_CONSTANT_BUFFER	= 0x4L,
		BIND_SHADER_RESOURCE	= 0x8L,
		BIND_STREAM_OUTPUT	= 0x10L,
		BIND_RENDER_TARGET	= 0x20L,
		BIND_DEPTH_STENCIL	= 0x40L,
		BIND_UNORDERED_ACCESS	= 0x80L
	};

	enum BUFFER_CPU_ACCESS_FLAG
	{
		BUFFER_CPU_ACCESS_NONE = 0,
		BUFFER_CPU_ACCESS_WRITE = 0x10000L,
		BUFFER_CPU_ACCESS_READ = 0x20000L
	};

	enum MAP_TYPE
	{
		MAP_TYPE_READ = 1,
		MAP_TYPE_WRITE = 2,
		MAP_TYPE_READ_WRITE = 3,
		MAP_TYPE_WRITE_DISCARD = 4,
		MAP_TYPE_WRITE_NO_OVERWRITE = 5
	};

	enum MAP_FLAG
	{
		MAP_FLAG_NONE = 0,
		MAP_FLAG_DO_NOT_WAIT = 0x100000L
	};

	

	enum SHADER_TYPE : char
	{
		SHADER_TYPE_VS = 0x1,
		SHADER_TYPE_HS = 0x2,
		SHADER_TYPE_DS = 0x4,		
		SHADER_TYPE_GS = 0x8,
		SHADER_TYPE_PS = 0x10,
		SHADER_TYPE_CS = 0x20,
		SHADER_TYPE_COUNT = 6,
	};

	inline int ShaderIndex(SHADER_TYPE type) {
		switch (type) {
		case SHADER_TYPE_VS:
			return 0;
		case SHADER_TYPE_HS:
			return 1;
		case SHADER_TYPE_DS:
			return 2;
		case SHADER_TYPE_GS:
			return 3;
		case SHADER_TYPE_PS:
			return 4;
		case SHADER_TYPE_CS:
			return 5;
		}
		assert(0);
		return 0;
	}

	inline SHADER_TYPE ShaderType(int i) {
		if (i < 0 || i >= SHADER_TYPE_COUNT) {
			assert(0);
			return SHADER_TYPE_VS;
		}
		return (SHADER_TYPE)(1 << i);
	}

	SHADER_TYPE BindingShaderFromString(const char* szShader);

	enum FILL_MODE
	{
		FILL_MODE_WIREFRAME=2,
		FILL_MODE_SOLID=3,
	};
	FILL_MODE FillModeFromString(const char* str);

	enum CULL_MODE
	{
		CULL_MODE_NONE=1,
		CULL_MODE_FRONT=2,
		CULL_MODE_BACK=3
	};
	CULL_MODE CullModeFromString(const char* str);

	enum TEXTURE_TYPE
	{
		TEXTURE_TYPE_DEFAULT=0x0,
		TEXTURE_TYPE_COLOR_RAMP=0x1,
		TEXTURE_TYPE_RENDER_TARGET=0x2,
		TEXTURE_TYPE_RENDER_TARGET_SRV=0x4,
		TEXTURE_TYPE_DEPTH_STENCIL=0x8,
		TEXTURE_TYPE_DEPTH_STENCIL_SRV=0x10, //DXGI_FORMAT_R32_TYPELESS
		TEXTURE_TYPE_CUBE_MAP=0x20,
		TEXTURE_TYPE_MULTISAMPLE = 0x40,
		TEXTURE_TYPE_MIPS = 0x80, // for rendertarget && shader resource
		TEXTURE_TYPE_1D = 0x100,
		TEXTURE_TYPE_SECOND_DEVICE = 0x200, // for texture binding to ComputeShader.
		TEXTURE_TYPE_COUNT
	};

	int TextureTypeFromString(const char* str);
	
	enum TEXTURE_FILTER
	{
		TEXTURE_FILTER_MIN_MAG_MIP_POINT	= 0,
		TEXTURE_FILTER_MIN_MAG_POINT_MIP_LINEAR	= 0x1,
		TEXTURE_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x4,
		TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR	= 0x5,
		TEXTURE_FILTER_MIN_LINEAR_MAG_MIP_POINT	= 0x10,
		TEXTURE_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x11,
		TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT	= 0x14,
		TEXTURE_FILTER_MIN_MAG_MIP_LINEAR	= 0x15,
		TEXTURE_FILTER_ANISOTROPIC	= 0x55,
		TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_POINT	= 0x80,
		TEXTURE_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR	= 0x81,
		TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT	= 0x84,
		TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR	= 0x85,
		TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT	= 0x90,
		TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR	= 0x91,
		TEXTURE_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT	= 0x94,
		TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR	= 0x95,
		TEXTURE_FILTER_COMPARISON_ANISOTROPIC	= 0xd5
	};

	TEXTURE_FILTER FilterFromString(const char* sz);

	enum TEXTURE_ADDRESS_MODE
	{
		TEXTURE_ADDRESS_WRAP	= 1,
		TEXTURE_ADDRESS_MIRROR	= 2,
		TEXTURE_ADDRESS_CLAMP	= 3,
		TEXTURE_ADDRESS_BORDER	= 4,
		TEXTURE_ADDRESS_MIRROR_ONCE	= 5
	};

	TEXTURE_ADDRESS_MODE AddressModeFromString(const char* sz);

	enum COMPARISON_FUNC : int
	{
		COMPARISON_NEVER			= 1,
		COMPARISON_LESS				= 2,
		COMPARISON_EQUAL			= 3,
		COMPARISON_LESS_EQUAL		= 4,
		COMPARISON_GREATER			= 5,
		COMPARISON_NOT_EQUAL		= 6,
		COMPARISON_GREATER_EQUAL	= 7,
		COMPARISON_ALWAYS			= 8
	};

	static const char* STR_COMPARISON_FUNC[]=
	{
		"COMPARISON_NEVER",
		"COMPARISON_LESS",
		"COMPARISON_EQUAL",
		"COMPARISON_LESS_EQUAL",
		"COMPARISON_GREATER",
		"COMPARISON_NOT_EQUAL",
		"COMPARISON_GREATER_EQUAL",
		"COMPARISON_ALWAYS",
	};

	COMPARISON_FUNC ComparisonFuncFromString(const char* str);

	enum BLEND
	{
		BLEND_ZERO               = 1,
		BLEND_ONE                = 2,
		BLEND_SRC_COLOR          = 3,
		BLEND_INV_SRC_COLOR      = 4,
		BLEND_SRC_ALPHA          = 5,
		BLEND_INV_SRC_ALPHA      = 6,
		BLEND_DEST_ALPHA         = 7,
		BLEND_INV_DEST_ALPHA     = 8,
		BLEND_DEST_COLOR         = 9,
		BLEND_INV_DEST_COLOR     = 10,
		BLEND_SRC_ALPHA_SAT      = 11,
		BLEND_INVALID1			 = 12,
		BLEND_INVALID2			 = 13,
		BLEND_BLEND_FACTOR       = 14,
		BLEND_INV_BLEND_FACTOR   = 15,
		BLEND_SRC1_COLOR         = 16,
		BLEND_INV_SRC1_COLOR     = 17,
		BLEND_SRC1_ALPHA         = 18,
		BLEND_INV_SRC1_ALPHA     = 19 
	};

	static const char* STR_BLEND[] =
	{
		"BLEND_ZERO",
		"BLEND_ONE",
		"BLEND_SRC_COLOR",
		"BLEND_INV_SRC_COLOR",
		"BLEND_SRC_ALPHA",
		"BLEND_INV_SRC_ALPHA",
		"BLEND_DEST_ALPHA",
		"BLEND_INV_DEST_ALPHA",
		"BLEND_DEST_COLOR",
		"BLEND_INV_DEST_COLOR",
		"BLEND_SRC_ALPHA_SAT",
		"BLEND_INVALID1",
		"BLEND_INVALID2",
		"BLEND_BLEND_FACTOR",
		"BLEND_INV_BLEND_FACTOR",
		"BLEND_SRC1_COLOR",
		"BLEND_INV_SRC1_COLOR",
		"BLEND_SRC1_ALPHA",
		"BLEND_INV_SRC1_ALPHA",
	};

	BLEND BlendFromString(const char* str);

	enum BLEND_OP
	{
		BLEND_OP_ADD            = 1,
		BLEND_OP_SUBTRACT       = 2,
		BLEND_OP_REV_SUBTRACT   = 3,
		BLEND_OP_MIN            = 4,
		BLEND_OP_MAX            = 5
	};

	static const char* STR_BLEND_OP[] = {
		"BLEND_OP_ADD",
		"BLEND_OP_SUBTRACT",
		"BLEND_OP_REV_SUBTRACT",
		"BLEND_OP_MIN",
		"BLEND_OP_MAX",
	};

	BLEND_OP BlendOpFromString(const char* str);

	enum COLOR_WRITE_MASK
    {
		COLOR_WRITE_MASK_NONE = 0,
		COLOR_WRITE_MASK_RED		= 1,
		COLOR_WRITE_MASK_GREEN		= 2,
		COLOR_WRITE_MASK_BLUE		= 4,
		COLOR_WRITE_MASK_ALPHA		= 8,
		COLOR_WRITE_MASK_ALL	= ( ( ( COLOR_WRITE_MASK_RED | COLOR_WRITE_MASK_GREEN )  | COLOR_WRITE_MASK_BLUE )  | COLOR_WRITE_MASK_ALPHA ) 
    };

	COLOR_WRITE_MASK ColorWriteMaskFromString(const char* str);

	enum DEPTH_WRITE_MASK : int
	{
		DEPTH_WRITE_MASK_ZERO 	=0,
		DEPTH_WRITE_MASK_ALL  	=1,		
	};

	DEPTH_WRITE_MASK DepthWriteMaskFromString(const char* str);

	enum STENCIL_OP : int
    {	
		STENCIL_OP_KEEP			= 1,
		STENCIL_OP_ZERO			= 2,
		STENCIL_OP_REPLACE		= 3,
		STENCIL_OP_INCR_SAT		= 4,
		STENCIL_OP_DECR_SAT		= 5,
		STENCIL_OP_INVERT		= 6,
		STENCIL_OP_INCR			= 7,
		STENCIL_OP_DECR			= 8
	};
	static const char* STR_STENCIL_OP[] = {
		"STENCIL_OP_KEEP",
		"STENCIL_OP_ZERO",
		"STENCIL_OP_REPLACE",
		"STENCIL_OP_INCR_SAT",
		"STENCIL_OP_DECR_SAT",
		"STENCIL_OP_INVERT",
		"STENCIL_OP_INCR",
		"STENCIL_OP_DECR",
	};
	STENCIL_OP StencilOpFromString(const char* str);

}
#include "RenderPass.h"
namespace fb{
	RENDER_PASS RenderPassFromString(const char* str);
}