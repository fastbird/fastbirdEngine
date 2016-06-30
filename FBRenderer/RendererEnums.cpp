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

#include "stdafx.h"
#include "RendererEnums.h"
#include "FBStringLib/StringLib.h"
namespace fb
{
	SHADER_TYPE BindingShaderFromString(const char* szShader)
	{
		if (!szShader)
			return SHADER_TYPE_PS;
		if (_stricmp(szShader, "vs") == 0)
		{
			return SHADER_TYPE_VS;
		}
		if (_stricmp(szShader, "ps") == 0)
		{
			return SHADER_TYPE_PS;
		}
		if (_stricmp(szShader, "ds") == 0)
		{
			return SHADER_TYPE_DS;
		}
		if (_stricmp(szShader, "hs") == 0)
		{
			return SHADER_TYPE_HS;
		}
		if (_stricmp(szShader, "gs") == 0)
		{
			return SHADER_TYPE_GS;
		}
		if (_stricmp(szShader, "cs") == 0)
		{
			return SHADER_TYPE_CS;
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid SHADER_TYPE", szShader).c_str());
		return SHADER_TYPE_PS;
	}

	FILL_MODE FillModeFromString(const char* str)
	{
		if (!str)
			return FILL_MODE_SOLID;
		if (_stricmp(str, "FILL_MODE_WIREFRAME") == 0)
		{
			return FILL_MODE_WIREFRAME;
		}
		if (_stricmp(str, "FILL_MODE_SOLID") == 0)
		{
			return FILL_MODE_SOLID;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not vaild FILL_MODE", str).c_str());
		return FILL_MODE_SOLID;
	}

	CULL_MODE CullModeFromString(const char* str)
	{
		if (!str)
			return CULL_MODE_BACK;
		if (_stricmp(str, "CULL_MODE_NONE") == 0)
		{
			return CULL_MODE_NONE;
		}
		if (_stricmp(str, "CULL_MODE_FRONT") == 0)
		{
			return CULL_MODE_FRONT;
		}
		if (_stricmp(str, "CULL_MODE_BACK") == 0)
		{
			return CULL_MODE_BACK;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not vaild CULL_MODE", str).c_str());
		return CULL_MODE_BACK;
	}

	int TextureTypeFromString(const char* str)
	{
		if (!str)
			return TEXTURE_TYPE_DEFAULT;
		if (_stricmp(str, "ColorRamp") == 0)
			return TEXTURE_TYPE_COLOR_RAMP;
		if (_stricmp(str, "Default") == 0)
			return TEXTURE_TYPE_DEFAULT;
		if (_stricmp(str, "RenderTarget") == 0)
			return TEXTURE_TYPE_RENDER_TARGET;
		if (_stricmp(str, "DepthStencil") == 0)
			return TEXTURE_TYPE_DEPTH_STENCIL;
		if (_stricmp(str, "1D") == 0) {
			return TEXTURE_TYPE_DEFAULT | TEXTURE_TYPE_1D;
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not vaild TEXTURE_TYPE", str).c_str());
		return TEXTURE_TYPE_DEFAULT;
	}

	TEXTURE_FILTER FilterFromString(const char* sz)
	{
		if (!sz)
			return TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
		if (_stricmp(sz, "MIN_MAG_MIP_POINT") == 0)
			return TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		if (_stricmp(sz, "MIN_MAG_POINT_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		if (_stricmp(sz, "MIN_POINT_MAG_LINEAR_MIP_POINT") == 0)
			return TEXTURE_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		if (_stricmp(sz, "MIN_POINT_MAG_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		if (_stricmp(sz, "MIN_LINEAR_MAG_MIP_POINT") == 0)
			return TEXTURE_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		if (_stricmp(sz, "MIN_LINEAR_MAG_POINT_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		if (_stricmp(sz, "MIN_MAG_LINEAR_MIP_POINT") == 0)
			return TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		if (_stricmp(sz, "MIN_MAG_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
		if (_stricmp(sz, "ANISOTROPIC") == 0)
			return TEXTURE_FILTER_ANISOTROPIC;
		if (_stricmp(sz, "COMPARISON_MIN_MAG_MIP_POINT") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		if (_stricmp(sz, "COMPARISON_MIN_MAG_POINT_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		if (_stricmp(sz, "COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		if (_stricmp(sz, "COMPARISON_MIN_POINT_MAG_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		if (_stricmp(sz, "COMPARISON_MIN_LINEAR_MAG_MIP_POINT") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		if (_stricmp(sz, "COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		if (_stricmp(sz, "COMPARISON_MIN_MAG_LINEAR_MIP_POINT") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		if (_stricmp(sz, "COMPARISON_MIN_MAG_MIP_LINEAR") == 0)
			return TEXTURE_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		if (_stricmp(sz, "COMPARISON_ANISOTROPIC") == 0)
			return TEXTURE_FILTER_COMPARISON_ANISOTROPIC;

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not vaild TEXTURE_FILTER", sz).c_str());
		return TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
	}

	TEXTURE_ADDRESS_MODE AddressModeFromString(const char* sz)
	{
		if (!sz)
			return TEXTURE_ADDRESS_CLAMP;
		if (_stricmp(sz, "Wrap") == 0)
			return TEXTURE_ADDRESS_WRAP;
		if (_stricmp(sz, "Mirror") == 0)
			return TEXTURE_ADDRESS_MIRROR;
		if (_stricmp(sz, "Clamp") == 0)
			return TEXTURE_ADDRESS_CLAMP;
		if (_stricmp(sz, "Border") == 0)
			return TEXTURE_ADDRESS_BORDER;
		if (_stricmp(sz, "Mirror_Once") == 0)
			return TEXTURE_ADDRESS_MIRROR_ONCE;

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid TEXTURE_ADDRESS_MODE", sz).c_str());
		return TEXTURE_ADDRESS_CLAMP;
	}

	COMPARISON_FUNC ComparisonFuncFromString(const char* str)
	{
		if (!str)
			return COMPARISON_LESS;
		for (int i = 1; i <= 8; ++i)
		{
			if (_stricmp(str, STR_COMPARISON_FUNC[i - 1]) == 0)
			{
				return COMPARISON_FUNC(i);
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid COMPARISON_FUNC", str).c_str());
		return COMPARISON_LESS;
	}

	BLEND BlendFromString(const char* str)
	{
		if (!str)
			return BLEND_ONE;
		for (int i = 1; i <= 19; i++)
		{
			if (_stricmp(str, STR_BLEND[i - 1]) == 0)
			{
				return BLEND(i);
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid Blend.", str).c_str());
		return BLEND_ONE;
	}

	BLEND_OP BlendOpFromString(const char* str)
	{
		if (!str)
			return BLEND_OP_ADD;
		for (int i = 1; i <= 5; i++)
		{
			if (_stricmp(str, STR_BLEND_OP[i - 1]) == 0)
			{
				return BLEND_OP(i);
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid BlendOp.", str).c_str());
		return BLEND_OP_ADD;
	}

	COLOR_WRITE_MASK ColorWriteMaskFromString(const char* str)
	{
		if (!str)
			return COLOR_WRITE_MASK_ALL;

		if (_stricmp(str, "COLOR_WRITE_MASK_RED") == 0)
		{
			return COLOR_WRITE_MASK_RED;
		}
		if (_stricmp(str, "COLOR_WRITE_MASK_GREEN") == 0)
		{
			return COLOR_WRITE_MASK_GREEN;
		}
		if (_stricmp(str, "COLOR_WRITE_MASK_BLUE") == 0)
		{
			return COLOR_WRITE_MASK_BLUE;
		}
		if (_stricmp(str, "COLOR_WRITE_MASK_ALPHA") == 0)
		{
			return COLOR_WRITE_MASK_ALPHA;
		}
		if (_stricmp(str, "0") == 0)
			return COLOR_WRITE_MASK_NONE;

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid COLOR_WRITE_MASK", str).c_str());
		return COLOR_WRITE_MASK_ALL;
	}

	DEPTH_WRITE_MASK DepthWriteMaskFromString(const char* str)
	{
		if (!str)
			return DEPTH_WRITE_MASK_ALL;
		if (_stricmp(str, "DEPTH_WRITE_MASK_ZERO") == 0)
		{
			return DEPTH_WRITE_MASK_ZERO;
		}

		if (_stricmp(str, "DEPTH_WRITE_MASK_ALL") == 0)
		{
			return DEPTH_WRITE_MASK_ALL;
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid DepthWriteMask", str).c_str());
		return DEPTH_WRITE_MASK_ALL;
	}

	STENCIL_OP StencilOpFromString(const char* str)
	{
		if (!str)
			return STENCIL_OP_KEEP;

		for (int i = 1; i <= 8; ++i)
		{
			if (_stricmp(str, STR_STENCIL_OP[i - 1]) == 0)
				return STENCIL_OP(i);
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid StencilOp", str).c_str());
		return STENCIL_OP_KEEP;
	}

	RENDER_PASS RenderPassFromString(const char* str)
	{
		if (!str)
			return PASS_NORMAL;
		if (_stricmp(str, "NORMAL") == 0)
			return PASS_NORMAL;
		if (_stricmp(str, "GODRAY_OCC_PRE") == 0)
			return PASS_GODRAY_OCC_PRE;
		if (_stricmp(str, "GLOW") == 0)
			return PASS_GLOW;
		if (_stricmp(str, "DEPTH") == 0)
			return PASS_DEPTH;
		if (_stricmp(str, "DEPTH_ONLY") == 0)
			return PASS_DEPTH_ONLY;
		if (_stricmp(str, "SHADOW") == 0)
			return PASS_SHADOW;
		
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid RENDER_PASS", str).c_str());
		return PASS_NORMAL;
	}

}