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

/**
\file FBDebug.h
Debug utility.
\author Jungwan Byun
\defgroup FBDebugLib
A debug library
*/
#pragma once
#include "FBCommonHeaders/String.h"
#include <fstream>
#include <memory>
namespace fb{
	/** Collection of debug featreus.
	\ingroup FBDebug
	*/
	class Logger{
	public:
		/** Initialize the log file.
		Prepare the log file. Logs received before the initializing will be sent to
		the debug output widow rather than recorded into the log file.
		@param filepath The new log file path. ex)error.log		
		*/		
		static void Init(const char* filepath);
		static void Init(const WCHAR* filepath);
		/** Initialize the global error log file.
		Any log message containing (error)(info)(log) will be printed this file also.
		Do not need to call in the every modulues. call once.
		*/
		static void InitGlobalLog(const char* filepath);

		/** Close the log file
		Logs received after Debug is released, will be sent to the debug output.
		*/
		static void Release();

		/** Output to the log file created by \b CreateLogFile(). */		
		static void Log(const char* str, ...);		
		/** Check whether the log message is same with the previous frame
		You can prevent logging the same message every frame by passing \a frame arg.
		*/
		static void Log(FRAME_PRECISION curFrame, TIME_PRECISION curTime, const char* str, ...);

		/** Output to the log file specified as a parameter. */
		static void Log(std::ofstream& file, const char* str);

		/** Output to the debug window.	*/
		static void Output(const char* str, ...);
	};
}

#define FB_DEFAULT_LOG_ARG "%s:\n  %s\n", __FUNCTION__
#define FB_DEFAULT_LOG_ARG_NO_LINE "%s:\n  %s", __FUNCTION__
#define FB_ERROR_LOG_ARG "%s:\n  (error) %s\n", __FUNCTION__
#define FB_ERROR_LOG_ARG_NO_LINE "%s:\n  (error) %s", __FUNCTION__
#define FB_FRAME_TIME gpTimer->GetFrame(), gpTimer->GetTime()
