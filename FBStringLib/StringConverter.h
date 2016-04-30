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
#include "FBCommonHeaders/String.h"
#include "FBCommonHeaders/Types.h"
namespace fb{
	class StringConverter{
	public:
		static TString ToString(Real val, unsigned short precision = 6,
			unsigned short width = 0, char fill = ' ', 
			std::ios::fmtflags flags = std::ios::fmtflags(0));
		static TString ToString(int val, unsigned short width = 0,
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));
		static TString ToString(size_t val, unsigned short width = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));		
		static TString ToString(unsigned long val, unsigned short width = 0, 
			char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));
		static TString ToString(UINT64 val);
		static TString ToStringK(unsigned val);		
		static TString ToString(long val, unsigned short width = 0, char fill = ' ',
			std::ios::fmtflags flags = std::ios::fmtflags(0));
		/** Converts a boolean to a String.
		\param \a yesNo If set to true, result is 'yes' or 'no' instead of 'true' or 'false'
		*/
		static TString ToString(bool val, bool yesNo = false);
		/// Should not contain a ' ' in the each string elements.
		static TString ToString(const TStringVector& val);		
		static TString ToHexa(unsigned val);

		static Real ParseReal(const TString& val, Real defaultValue = 0);
		static int ParseInt(const TString& val, int defaultValue = 0);
		static unsigned int ParseUnsignedInt(const TString& val, unsigned int defaultValue = 0);
		static unsigned int ParseHexa(const TString& val, unsigned int defaultValue = 0);
		static long ParseLong(const TString& val, long defaultValue = 0);
		static unsigned long ParseUnsignedLong(const TString& val, unsigned long defaultValue = 0);
		static unsigned long long ParseUnsignedLongLong(const TString& val, unsigned long long defaultValue = 0);
		static UINT64 ParseUINT64(const TString& val, unsigned long long defaultValue = 0);
		/** Converts a TString to a boolean.
		@remarks
		Returns true if case-insensitive match of the start of the string
		matches "true", "yes" or "1", false otherwise.
		*/
		static bool ParseBool(const TString& val, bool defaultValue = 0);

		/** Checks the TString is a valid number value. */
		static bool IsNumber(const TString& val);
		static TStringVector ParseStringVector(const TString& val, const std::string& delims = "\t\n, ",
			unsigned int maxSplits = 0, bool preserveDelims = false);
		
	};
}