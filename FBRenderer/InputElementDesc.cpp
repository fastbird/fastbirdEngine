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
#include "InputElementDesc.h"
#include "FBStringLib/StringLib.h"
namespace fb{

	INPUT_ELEMENT_FORMAT InputElementFromString(const char* sz)
	{
		if (!sz)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid INPUT_ELEMENT_FORMAT", sz).c_str());
			assert(0);
			return INPUT_ELEMET_FORMAT_NUM;
		}

		if (_stricmp(STR_INPUT_ELEMENT_FORMAT[INPUT_ELEMENT_FORMAT_FLOAT4], sz) == 0)
		{
			return INPUT_ELEMENT_FORMAT_FLOAT4;
		}
		if (_stricmp(STR_INPUT_ELEMENT_FORMAT[INPUT_ELEMENT_FORMAT_FLOAT3], sz) == 0)
		{
			return INPUT_ELEMENT_FORMAT_FLOAT3;
		}
		if (_stricmp(STR_INPUT_ELEMENT_FORMAT[INPUT_ELEMENT_FORMAT_UBYTE4], sz) == 0)
		{
			return INPUT_ELEMENT_FORMAT_UBYTE4;
		}
		if (_stricmp(STR_INPUT_ELEMENT_FORMAT[INPUT_ELEMENT_FORMAT_FLOAT2], sz) == 0)
		{
			return INPUT_ELEMENT_FORMAT_FLOAT2;
		}
		if (_stricmp(STR_INPUT_ELEMENT_FORMAT[INPUT_ELEMET_FORMAT_INT4], sz) == 0)
		{
			return INPUT_ELEMET_FORMAT_INT4;
		}

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid INPUT_ELEMENT_FORMAT", sz).c_str());
		assert(0);
		return INPUT_ELEMET_FORMAT_NUM;
	}

	INPUT_CLASSIFICATION InputClassificationFromString(const char* sz)
	{
		if (!sz)
			return INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		if (_stricmp(sz, "VERTEX") == 0)
			return INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		if (_stricmp(sz, "INSTANCE") == 0)
			return INPUT_CLASSIFICATION_PER_INSTANCE_DATA;

		Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s is not valid INPUT_CLASSIFICATION", sz).c_str());
		return INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
}