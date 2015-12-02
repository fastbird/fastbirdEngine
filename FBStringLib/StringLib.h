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
\file FBStringLib.h
Provide string manipulators
\author Jungwan Byun
\defgroup FBStringLib
Provide string manipulators
*/
#pragma once
#include "FBCommonHeaders/String.h"
#include <vector>
namespace fb{
	///@addtogroup FBStringLib
	///@{

	/// Replace all occurences of \a target in \a s to \a replace
	void ReplaceCharacter(char* s, char target, char replace);	
	char* StripRight(char* s);
	char* StripLeft(char* s);
	std::string StripBoth(const char* s);	
	const char* FindLastOf(const char* s, char ch);
	void StepToDigit(char** ppch);
	StringVector Split(const std::string& str, const std::string& delims = "\t\n, ",
		unsigned int maxSplits = 0, bool preserveDelims = false);
	/** Compare two strings. This function ignores the case.
	@return 0 if two strings are same. 1 if \b lhs > \b rhs. -1 if \nlhs < \b rhs
	*/
	int StringCompareNoCase(const std::string& lhs, const std::string& rhs);
	/** Check whether the \b str is start with the \b pattern.
	@param lowerCase if true, the \b str will be lowered before checking.
	*/
	bool StartsWith(const std::string& str, const std::string& pattern, bool lowerCase = true);
	
	void ToLowerCase(std::string& str);	
	void ToLowerCaseFirst(std::string& str);
	void ToUpperCase(std::string& str);
	void ToUpperCase(std::wstring& str);
	
	bool IsNumeric(const char* str);
	std::wstring FormatString(const WCHAR* str, ...);	
	std::string FormatString(const char* str, ...);

	//-----------------------------------------------------------------------
	// UNICODE conversion
	enum EUnicodeByteOrder	{
		FB_LITTLE_ENDIAN,
		FB_BIG_ENDIAN,
	};

	/// This function will attempt to decode a UTF-8 encoded character in the buffer.
	/// If the encoding is invalid, the function returns -1.
	int DecodeUTF8(const unsigned char *encodedBuffer, unsigned int *outCharLength);

	/// This function will encode the value into the buffer.
	/// If the value is invalid, the function returns -1, else the encoded length.
	int EncodeUTF8(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength);

	/// This function will attempt to decode a UTF-16 encoded character in the buffer.
	/// If the encoding is invalid, the function returns -1.
	int DecodeUTF16(const unsigned char *encodedBuffer, unsigned int *outCharLength, EUnicodeByteOrder byteOrder = FB_LITTLE_ENDIAN);

	/// This function will encode the value into the buffer.
	/// If the value is invalid, the function returns -1, else the encoded length.
	int EncodeUTF16(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength, EUnicodeByteOrder byteOrder = FB_LITTLE_ENDIAN);

	/// return data is temporary data. save it to other memory if you need
	unsigned char* AnsiToUTF8(const char* source);

	/// return data is temporary data. save it to other memory if you need
	WCHAR* AnsiToWide(const char* source, int size);
	WCHAR* AnsiToWide(const char* source);
	WCHAR AnsiToWide(const char source);

	// return data is temporary data. save it to other memory if you need
	WCHAR* UTF8ToWide(const unsigned char* source);

	const char* WideToAnsi(const WCHAR* source);	
	///@}


	class HashedString
	{
		void *             mIdent;
		std::string		   mIdentStr;

	public:
		explicit HashedString(char const * const pIdentString);
		unsigned long getHashValue(void) const;
		const std::string & getStr() const;
		static void* hash_name(char const *  pIdentStr);
		bool operator< (HashedString const & o) const;
		bool operator== (HashedString const & o) const;
	};
}