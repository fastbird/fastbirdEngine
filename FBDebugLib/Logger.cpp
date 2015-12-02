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

#include "Logger.h"
#include "FBCommonHeaders/platform.h"
#include "FBCommonHeaders/VectorMap.h"
#if defined(_PLATFORM_WINDOWS_)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#endif

#include <iostream>
#include <set>

using namespace fb;

static int sInitialized = false;
static std::shared_ptr<std::ofstream> sLogFile;
static std::streambuf* sOriginalErrorStream = 0;
static std::shared_ptr<std::ofstream> sGlobalErrorLog;

void Logger::Init(const char* filepath){	
	sLogFile = std::make_shared<std::ofstream>();
	sLogFile->open(filepath);
	
	sInitialized = true;
}

void Logger::Init(const WCHAR* filepath){
	sLogFile->open(filepath);
	auto errStream = std::cerr.rdbuf(sLogFile->rdbuf());
	if (!sOriginalErrorStream){
		sOriginalErrorStream = errStream;
	}
	sInitialized = true;
}

void Logger::InitGlobalLog(const char* filepath){
	sGlobalErrorLog = std::make_shared<std::ofstream>();
	sGlobalErrorLog->open(filepath);
	auto errStream = std::cerr.rdbuf(sGlobalErrorLog->rdbuf());
	if (!sOriginalErrorStream){
		sOriginalErrorStream = errStream;
	}
}

void Logger::Release(){
	sInitialized = false;
	if (sOriginalErrorStream){
		std::cerr.rdbuf(sOriginalErrorStream);
		sOriginalErrorStream = 0;
	}
	sLogFile->close();	
	if (sGlobalErrorLog){
		sGlobalErrorLog->close();
	}
}

void Logger::Log(const char* str, ...){
	static const unsigned BUFFER_SIZE = 2048;	
	if (!str) return;
	auto length = strlen(str);
	if (length == 0) return;
	if (length >= BUFFER_SIZE){
		std::cerr << "Log message is too long to print and truncated. Maximum 2047 characters are supported.\n";
	}
	char buffer[BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsprintf_s(buffer, str, args);
	va_end(args);

	//if (strstr(buffer, "(error)") || strstr(buffer, "(log)") || strstr(buffer, "(info)"))
	std::cerr << buffer;

	if (sLogFile && sLogFile->is_open()){
		*sLogFile << buffer;
		sLogFile->flush();
#if defined(_PLATFORM_WINDOWS_)
		OutputDebugStringA(buffer);
#else
#endif
	}
	else{
		// fallback
		Output(buffer);
	}
}

struct PreventedMessage{
	FRAME_PRECISION mFrame;
	TIME_PRECISION mTime;
	std::string mMessage;

	PreventedMessage(FRAME_PRECISION frame, TIME_PRECISION time, std::string&& msg)
		: mFrame(frame), mTime(time), mMessage(msg)
	{
	}

	bool operator < (const PreventedMessage& other) const{
		return mMessage < other.mMessage;
	}
};
static std::set<PreventedMessage> sPreventedMessage;
static VectorMap<FRAME_PRECISION, std::set< std::string > > sMessages;
void Logger::Log(FRAME_PRECISION curFrame, TIME_PRECISION curTime, const char* str, ...){
	static const unsigned BUFFER_SIZE = 2048;
	if (!str) return;
	auto length = strlen(str);
	if (length == 0) return;
	if (length >= BUFFER_SIZE){
		std::cerr << "Log message is too long to print and truncated. Maximum 2047 characters are supported.\n";
	}
	char buffer[BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsprintf_s(buffer, str, args);
	va_end(args);

	static const TIME_PRECISION PREVENT_IN = 5.f; // Do not print the same log in 5 secons.
	if (curFrame > 1){
		PreventedMessage currentMsg(curFrame, curTime, std::string(buffer));
		auto itPrevented = sPreventedMessage.find(currentMsg);
		if (itPrevented != sPreventedMessage.end()){
			TIME_PRECISION elapsed = curTime - itPrevented->mTime;
			if (elapsed < PREVENT_IN)
				return; // block the message
			else{
				sPreventedMessage.insert(currentMsg); // update the time.
			}
		}

		//  check whether the last frame has the same message
		auto it = sMessages.Find(curFrame - 1);
		if (it != sMessages.end()){
			
			if (it->second.find(currentMsg.mMessage) != it->second.end()){
				sPreventedMessage.insert(currentMsg); // found. insert it to prevented and return.
				return;
			}
		}

		// so far so good.
		// delete  <= curFrame-2 data if exists
		for (auto it = sMessages.begin(); it != sMessages.end(); ){
			auto curIt = it;
			++it;
			if (curIt->first <= curFrame - 2){
				sMessages.erase(curIt);
			}
			else{
				break;
			}
		}
	}

	if (sLogFile && sLogFile->is_open()){
		*sLogFile << buffer;
		sLogFile->flush();
	}
	else{
		// fallback
		Output(buffer);
	}
}

void Logger::Log(std::ofstream& file, const char* str){
	if (file.is_open()){
		file << str;
	}
	else{
		// fallback
		Output(str);
	}
}

void Logger::Output(const char* str, ...){
	static const unsigned BUFFER_SIZE = 2048;
	if (!str) return;
	auto length = strlen(str);
	if (length == 0) return;
	if (length >= BUFFER_SIZE){
		std::cerr << "Log message is too long to print and truncated. Maximum 2047 characters are supported.\n";
	}
	char buffer[BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	vsprintf_s(buffer, str, args);
	va_end(args);
#if defined(_PLATFORM_WINDOWS_)
	OutputDebugStringA(buffer);
	std::cerr << buffer;
#else
	std::cerr << str;
#endif
}
