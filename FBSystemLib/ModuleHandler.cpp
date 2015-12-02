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
#include "ModuleHandler.h"
#include "FBDebugLib/Logger.h"
namespace fb{
	// intended redundancy
	static std::string FormatString(const char* str, ...){
		static char buf[2048];
		va_list args;

		va_start(args, str);
		vsprintf_s(buf, str, args);
		va_end(args);

		return buf;
	}

	ModuleHandle ModuleHandler::LoadModule(const char* path){
#if defined(_PLATFORM_WINDOWS_)
		if (!path)
			return 0;

		auto dotPos = strchr(path, _T('.'));
		char buf[MAX_PATH] = { 0 };
		if (dotPos){
			strncpy_s(buf, MAX_PATH, path, dotPos - path);
		}
		else{
			strcpy_s(buf, MAX_PATH, path);
		}

#ifdef _DEBUG
		strcat_s(buf, MAX_PATH, "_Debug.dll");
#else
		strcat_s(buf, MAX_PATH, "_Release.dll");
#endif
		Logger::Log(FB_DEFAULT_LOG_ARG_NO_LINE, FormatString("Trying to load a module(%s)... ", buf).c_str());
		HMODULE module = LoadLibraryA(buf);
		if (!module) {
			Logger::Log("\tFailed.\n");
			strcpy_s(buf, MAX_PATH, path);
			strcat_s(buf, MAX_PATH, ".dll");
			Logger::Log(FB_DEFAULT_LOG_ARG_NO_LINE, FormatString("Trying to load a module(%s)... ", buf).c_str());
			module = LoadLibraryA(buf);
		}		
		
		if (!module) {
			Logger::Log("\tFailed.\n");
		}
		else{
			Logger::Log("\tSucceeded.\n");
		}
		return (intptr_t)module;
#else
		assert(0 && "Not implemented");
#endif
	}

	void ModuleHandler::UnloadModule(ModuleHandle handle){
#if defined(_PLATFORM_WINDOWS_)
		if (handle)
			FreeLibrary((HMODULE)handle);
#else
		assert(0 && "Not implemented");
#endif
	}

	FunctionHandle ModuleHandler::GetFunction(ModuleHandle module, const char* functionName){
		if (!module){
			Logger::Log(FB_DEFAULT_LOG_ARG, "ModuleHandler::GetFunction : invalid param");
			return 0;
		}
#if defined(_PLATFORM_WINDOWS_)
		return (FunctionHandle)GetProcAddress((HMODULE)module, functionName);
#else
		assert(0 && "Not implemented");
#endif
	}
}