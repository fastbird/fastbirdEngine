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

#include "StdAfx.h"
#include "ImageDisplay.h"

namespace fb{
namespace ImageDisplay{
	const char* strings[] = { 
		"FreeScaleImageMatchAll", 
		"KeepImageRatioMatchWidth", 
		"KeepImageRatioMatchHeight", 
		"FreeScaleUIMatchAll",
		"KeepUIRatioMatchWidth",
		"KeepUIRatioMatchHeight",
		"NoScale",
		
		"Num" };

	const char* ConvertToString(Enum display){
		if (display >= Num){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid ImageDisplay enum found.").c_str());
			return strings[0];
		}
		return strings[display];
	}

	Enum ConvertToEnum(const char* sz){
		for (int i = 0; i < Num; ++i){
			if (_stricmp(sz, strings[i]) == 0)
				return (Enum)i;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid ImageDisplay string(%s) found", sz).c_str());
		return FreeScaleImageMatchAll;
	}
}
}

