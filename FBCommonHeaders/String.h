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
#include "platform.h"
#include <vector>
#include <string>
#include <cstdarg>

typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character

#if defined(_PLATFORM_MAC_)
static inline int sprintf(TCHAR* str, size_t size, LPCTSTR format, ...){
	int ret = -1;
	va_list args;
	va_start(args, format);
	ret = _tvsnprintf(str, format, args);
	va_end(args);
	return ret;
}
#endif

inline int FB_WCSNCPY(WCHAR* dest, unsigned bufSize, const wchar_t* src, unsigned maxCount){
#if defined(_PLATFORM_WINDOWS_)
	return wcsncpy_s(dest, bufSize, src, maxCount);
#else
	return wcsncpy(dest, src, maxCount);
#endif
}

inline int FB_WCSCAT(WCHAR* dest, unsigned size, const wchar_t* src){
#if defined(_PLATFORM_WINDOWS_)
	return wcscat_s(dest, size, src);
#else
	return wcscat(dest, src);	
#endif
}

inline int FB_VSNPRINTF(char* dest, unsigned size, unsigned count, const char* format, ...)
{
	int ret = -1;
	va_list args;
	va_start(args, format);
#if defined(_PLATFORM_WINDOWS_)
	ret = vsnprintf_s(dest, size, count, format, args);
#else
	ret = vsnprintf(dest, size, format, args);
#endif
	va_end(args);
	return ret;
}


#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define __WFUNCTION__ WIDE1(__FUNCTION__)

#if defined(UNICODE)
	typedef wchar_t TCHAR;
	typedef const WCHAR* LPCTSTR;	
	typedef std::vector<std::wstring> TStringVector;
	typedef std::wstring TString;
	#define __T(x)      L ## x
	#define _tstrlen wcslen
	#define _tstrchr wcschr
	#define _tstrncpy FB_WCSNCPY
	#define _tstrcpy wcscpy
	#define _tstrncat wcsncat
	#define _tstrcat FB_WCSCAT
	#define _tisspace iswspace
	#define _tisdigit iswdigit
	#define _ttolower towlower
	#define _ttoupper towupper
	#define _tvsnprintf vswprintf
	#define _tsprintf swprintf
	#define _tofstream wofstream
	#define _tgeneric_string generic_wstring
	#define _tcout wcout
	#define _tcerr wcerr	
	#define _tofstream wofstream
	#define _tifstream wifstream
	#define _tstreambuf wstreambuf
	#define _tstringstream wstringstream
	#define __TFILE__ WFILE
	#define __TFUNCTION__ __WFUNCTION__
#else
	typedef const char* LPCTSTR;
	typedef char TCHAR;
	typedef std::vector<std::string> TStringVector;
    typedef std::string TString;
	#define __T(x)      x
	#define _tstrlen strlen	
	#define _tstrchr strchr
	#define _tstrncpy strncpy
	#define _tstrcpy strcpy
	#define _tstrncat strncat
	#define _tstrcat strcat
	#define _tisspace isspace
	#define _tisdigit isdigit
	#define _ttolower tolower
	#define _ttoupper toupper
	#define _tvsnprintf FB_VSNPRINTF
	#define _tsprintf sprintf
	#define _tofstream ofstream
	#define _tgeneric_string generic_string
	#define _tcout cout
	#define _tcerr cerr	
	#define _tofstream ofstream
	#define _tifstream ifstream
	#define _tstreambuf streambuf
	#define _tstringstream stringstream
	#define __TFILE__ __FILE__
	#define __TFUNCTION__ __FUNCTION__
#endif

#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)