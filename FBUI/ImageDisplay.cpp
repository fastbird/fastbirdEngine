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

