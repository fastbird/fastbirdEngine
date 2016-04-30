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
#include "StringLib.h"
#include "FBCommonHeaders/String.h"
#include "FBCommonHeaders/Helpers.h"


namespace fb{

	void RemoveNewLine(std::string& str)
	{
		for (auto rit = str.rbegin(); rit != str.rend(); ) {
			auto c = *rit;
			if (isspace(*rit)) {
				++rit;
				str.erase(rit.base());
			}
			else {
				++rit;
			}
		}
	}

	std::string RemoveNewLine(const char* szstr)
	{
		if (!ValidCStringLength(szstr))
			return{};
		std::string str(szstr);
		for (auto rit = str.rbegin(); rit != str.rend(); ) {
			auto c = *rit;
			if (isspace(*rit)) {
				++rit;
				str.erase(rit.base());
			}
			else {
				++rit;
			}
		}
		return str;
	}

	void ReplaceCharacter(TCHAR* s, TCHAR target, TCHAR replace){
		auto size = _tstrlen(s);
		for (unsigned i = 0; i < size; i++)
		{
			if (s[i] == target)
				s[i] = replace;
		}
	}

	void ReplaceCharacter(char* s, const std::string& target, char replace) {
		auto size = _tstrlen(s);
		for (unsigned i = 0; i < size; i++)
		{
			for (auto c : target) {
				if (s[i] == c)
					s[i] = replace;
			}
		}
	}

	void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
	}

	TCHAR* StripRight(TCHAR* s){
		TCHAR* p = s + _tstrlen(s);
		while (p > s && _tisspace((*--p)))
			*p = '\0';
		return s;
	}

	TCHAR* StripLeft(TCHAR* s){
		while (*s && _tisspace((*s)))
			s++;
		return s;
	}

	void StripBoth(std::string& str) {
		str = StripBoth(str.c_str());
	}

	std::string StripBoth(LPCTSTR s){
		if (s == 0)
		{
			assert(0);
			return std::string();
		}
		LPCTSTR b = s;
		LPCTSTR e = s + _tstrlen(s);
		while (b != e && _tisspace(*b))
		{
			b++;
		}
		while (e != b && _tisspace(*(e - 1)))
		{
			e--;
		}
		return std::string(b, e - b);
	}	

	LPCTSTR FindLastOf(LPCTSTR s, TCHAR ch){
		int len = (int)_tstrlen(s);
		LPCTSTR it = s + len - 1;
		for (; it >= s; --it)
		{
			if (*it == ch)
			{
				return it;
			}
		}
		return 0;
	}

	void StepToDigit(TCHAR** ppch){
		if (*ppch == 0)
			return;

		auto len = _tstrlen(*ppch);
		for (unsigned i = 0; i<len; i++)
		{
			if (_tisdigit(**ppch))
				return;
			*ppch += 1;
		}

		*ppch = 0;
	}

	StringVector Split(const std::string& str, const std::string& delims /*= "\t\n, "*/,
		unsigned int maxSplits /*= 0*/, bool preserveDelims /*= false*/)
	{
		StringVector ret;
		ret.reserve(maxSplits ? maxSplits + 1 : 10);
		unsigned int numSplits = 0;
		// Use STL methods 
		size_t start, pos;
		start = 0;
		do
		{
			pos = str.find_first_of(delims, start);
			if (pos == start)
			{
				// Do nothing
				ret.push_back(std::string());
				if (pos != std::string::npos)
					start = pos + 1;
			}
			else if (pos == std::string::npos || (maxSplits && numSplits == maxSplits))
			{
				// Copy the rest of the string
				ret.push_back(str.substr(start));
				break;
			}
			else
			{
				// Copy up to delimiter
				ret.push_back(str.substr(start, pos - start));

				if (preserveDelims)
				{
					// Sometimes there could be more than one delimiter in a row.
					// Loop until we don't find any more delims
					size_t delimStart = pos, delimPos;
					delimPos = str.find_first_not_of(delims, delimStart);
					if (delimPos == std::string::npos)
					{
						// Copy the rest of the string
						ret.push_back(str.substr(delimStart));
					}
					else
					{
						ret.push_back(str.substr(delimStart, delimPos - delimStart));
					}
				}

				start = pos + 1;
			}
			// parse up to next float data
			start = str.find_first_not_of(delims, start);
			++numSplits;

		} while (pos != std::string::npos);

		return ret;
	}

	int StringCompareNoCase(const std::string& lhs, const std::string& rhs){
		auto size = lhs.size();
		auto size2 = rhs.size();
		for (unsigned i = 0; i < size; ++i){
			auto l = tolower(lhs[i]);
			auto r = tolower(rhs[i]);
			if (l == r)
				continue;
			return l>r ? 1 : -1;
		}
		if (size == size2){
			return 0;
		}
		else{
			return size > size2 ? 1 : -1;
		}

	}

	bool StringContainNoCase(const std::string& string, const std::string& find) {
		auto stringSize = (int)string.size();
		auto findSize = (int)find.size();
		if (findSize > stringSize)
			return false;

		for (int si = 0; si <= stringSize - findSize; ++si) {
			bool found = true;
			for (int fi = 0; fi < findSize; ++fi) {
				if (tolower(string[si + fi]) != tolower(find[fi])) {
					found = false;
					break;
				}
			}		
			if (found)
				return true;
		}
		return false;
	}

	bool StartsWith(const std::string& str, const std::string& pattern, bool lowerCase){
		auto thisLen = str.length();
		auto patternLen = pattern.length();
		std::string transformedPatten = pattern;
		if (lowerCase) {
			ToLowerCase(transformedPatten);
		}
		if (thisLen < patternLen || patternLen == 0)
			return false;

		std::string startOfThis = str.substr(0, patternLen);
		if (lowerCase)
			ToLowerCase(startOfThis);

		return (startOfThis == transformedPatten);
	}
	
	bool EndsWith(const std::string& str, const char* pattern) {		
		auto it = str.find_last_of(pattern);
		if (it == str.size() - strlen(pattern))
			return true;

		return false;
	}

	void ToLowerCase(std::string& str){
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			tolower);
	}
	
	std::string ToLowerCase(const char* sz){
		if (!sz)
			return std::string();
		std::string lowered(sz);
		std::transform(
			lowered.begin(),
			lowered.end(),
			lowered.begin(),
			tolower);
		return lowered;
	}

	void ToLowerCaseFirst(std::string& str){
		std::transform(
			str.begin(), str.begin() + 1, str.begin(), tolower);
	}

	void ToUpperCase(std::string& str){
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			toupper);
	}

	void ToUpperCase(std::wstring& str){
		std::transform(
			str.begin(),
			str.end(),
			str.begin(),
			toupper);
	}

	bool IsNumeric(LPCTSTR str){
		auto len = _tstrlen(str);
		for (size_t i = 0; i < len; i++)
		{
			if (!_tisdigit(str[i]))
				return false;
		}
		return true;
	}

	std::wstring FormatString(const WCHAR* str, ...){
		static WCHAR buf[2048];
		va_list args;
		
        va_start(args, str);
		vswprintf(buf, 2048, str, args);
		va_end(args);

		return buf;
	}

	std::string FormatString(const char* str, ...){
		char buf[2048];
		va_list args;

		va_start(args, str);
		vsprintf_s(buf, str, args);
		va_end(args);

		return std::string(buf);
	}

	size_t FindLastIndexOf(const char* sz, char c) {
		if (!ValidCStringLength(sz))
			return -1;
		auto len = strlen(sz);
		return FindLastIndexOf(sz, c, len - 1);
	}

	size_t FindLastIndexOf(const char* sz, char c, size_t startIndex) {
		if (!ValidCStringLength(sz))
			return -1;
		auto len = strlen(sz);
		if (startIndex >= len)
			startIndex = len - 1;
		for (int i = startIndex; i >= 0; --i) {
			if (sz[i] == c)
				return i;
		}
		return -1;


	}

	std::string SubString(const char* sz, size_t s, size_t e) {
		if (!ValidCStringLength(sz))
			return std::string();

		auto len = strlen(sz);
		if (e > len)
			e = len;
		if (s > len)
			s = len;
		if (s == e)
			return std::string();
		return std::string(sz + s, sz + e);
	}

	std::string GetFileSuffix(const char* filePath) {
		if (!ValidCStringLength(filePath))
		{
			return std::string();
		}

		int len = strlen(filePath);
		int p = FindLastIndexOf(filePath, '.');
		auto suffix = (p >= 0 && p + 1 < len) ? SubString(filePath, p+1, len) : std::string();

		// handle .bil.gz extensions
		if (!suffix.empty() && p > 0 && suffix == "gz")
		{
			int idx = FindLastIndexOf(filePath, '.', p-1);
			suffix = (idx >= 0 && idx + 1 < len) ? SubString(filePath, idx + 1, len) : std::string();
		}

		return suffix;
	}

	//-----------------------------------------------------------------------
	// UNICODE conversion
	int DecodeUTF8(const unsigned char *encodedBuffer, unsigned int *outLength)
	{
		const unsigned char *buf = (const unsigned char*)encodedBuffer;

		int value = 0;
		int length = -1;
		unsigned char byte = buf[0];
		if ((byte & 0x80) == 0)
		{
			// This is the only byte
			if (outLength) *outLength = 1;
			return byte;
		}
		else if ((byte & 0xE0) == 0xC0)
		{
			// There is one more byte
			value = int(byte & 0x1F);
			length = 2;

			// The value at this moment must not be less than 2, because 
			// that should have been encoded with one byte only.
			if (value < 2)
				length = -1;
		}
		else if ((byte & 0xF0) == 0xE0)
		{
			// There are two more bytes
			value = int(byte & 0x0F);
			length = 3;
		}
		else if ((byte & 0xF8) == 0xF0)
		{
			// There are three more bytes
			value = int(byte & 0x07);
			length = 4;
		}

		int n = 1;
		for (; n < length; n++)
		{
			byte = buf[n];
			if ((byte & 0xC0) == 0x80)
				value = (value << 6) + int(byte & 0x3F);
			else
				break;
		}

		if (n == length)
		{
			if (outLength) *outLength = (unsigned)length;
			return value;
		}

		// The byte sequence isn't a valid UTF-8 byte sequence.
		return -1;
	}

	int EncodeUTF8(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outLength)
	{
		unsigned char *buf = (unsigned char*)outEncodedBuffer;

		int length = -1;

		if (value <= 0x7F)
		{
			buf[0] = value;
			if (outLength) *outLength = 1;
			return 1;
		}
		else if (value >= 0x80 && value <= 0x7FF)
		{
			// Encode it with 2 characters
			buf[0] = 0xC0 + (value >> 6);
			length = 2;
		}
		else if ((value >= 0x800 && value <= 0xD7FF) || (value >= 0xE000 && value <= 0xFFFF))
		{
			// Note: Values 0xD800 to 0xDFFF are not valid unicode characters
			buf[0] = 0xE0 + (value >> 12);
			length = 3;
		}
		else if (value >= 0x10000 && value <= 0x10FFFF)
		{
			buf[0] = 0xF0 + (value >> 18);
			length = 4;
		}

		int n = length - 1;
		for (; n > 0; n--)
		{
			buf[n] = 0x80 + (value & 0x3F);
			value >>= 6;
		}

		if (outLength) *outLength = length;
		return length;
	}

	int DecodeUTF16(const unsigned char *encodedBuffer, unsigned int *outLength, Endianness byteOrder)
	{
		const unsigned char *buf = (const unsigned char *)encodedBuffer;
		int value = 0;
		if (byteOrder == FB_LITTLE_ENDIAN)
		{
			value += buf[0];
			value += (unsigned int)(buf[1]) << 8;
		}
		else
		{
			value += buf[1];
			value += (unsigned int)(buf[0]) << 8;
		}

		if (value < 0xD800 || value > 0xDFFF)
		{
			if (outLength) *outLength = 2;
			return value;
		}
		else if (value < 0xDC00)
		{
			// We've found the first surrogate word
			value = ((value & 0x3FF) << 10);

			// Read the second surrogate word
			int value2 = 0;
			if (byteOrder == FB_LITTLE_ENDIAN)
			{
				value2 += buf[2];
				value2 += (unsigned int)(buf[3]) << 8;
			}
			else
			{
				value2 += buf[3];
				value2 += (unsigned int)(buf[2]) << 8;
			}

			// The second surrogate word must be in the 0xDC00 - 0xDFFF range
			if (value2 < 0xDC00 || value2 > 0xDFFF)
				return -1;

			value = value + (value2 & 0x3FF) + 0x10000;
			if (outLength) *outLength = 4;
			return value;
		}

		// It is an illegal sequence if a character in the 0xDC00-0xDFFF range comes first
		return -1;
	}

	int EncodeUTF16(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength, Endianness byteOrder)
	{
		if (value < 0x10000)
		{
			if (byteOrder == FB_LITTLE_ENDIAN)
			{
				outEncodedBuffer[0] = (value & 0xFF);
				outEncodedBuffer[1] = ((value >> 8) & 0xFF);
			}
			else
			{
				outEncodedBuffer[1] = (value & 0xFF);
				outEncodedBuffer[2] = ((value >> 8) & 0xFF);
			}

			if (outCharLength) *outCharLength = 2;
			return 2;
		}
		else
		{
			value -= 0x10000;
			int surrogate1 = ((value >> 10) & 0x3FF) + 0xD800;
			int surrogate2 = (value & 0x3FF) + 0xDC00;

			if (byteOrder == FB_LITTLE_ENDIAN)
			{
				outEncodedBuffer[0] = (surrogate1 & 0xFF);
				outEncodedBuffer[1] = ((surrogate1 >> 8) & 0xFF);
				outEncodedBuffer[2] = (surrogate2 & 0xFF);
				outEncodedBuffer[3] = ((surrogate2 >> 8) & 0xFF);
			}
			else
			{
				outEncodedBuffer[1] = (surrogate1 & 0xFF);
				outEncodedBuffer[0] = ((surrogate1 >> 8) & 0xFF);
				outEncodedBuffer[3] = (surrogate2 & 0xFF);
				outEncodedBuffer[2] = ((surrogate2 >> 8) & 0xFF);
			}

			if (outCharLength) *outCharLength = 4;
			return 4;
		}
	}

	// return data is temporary data. save it to other memory if you need
	unsigned char* AnsiToUTF8(const char* source)
	{
		static WCHAR wideBuffer[4096];
		static unsigned char utf8_buffer[4096];
		memset(wideBuffer, 0, 4096 * sizeof(WCHAR));
		memset(utf8_buffer, 0, 4096);
#if defined(_PLATFORM_WINDOWS_)
		int ret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, wideBuffer, 4096);
		if (ret == 0)
		{
			std::cerr << "(error) AnsiToUTF8 MultiByteToWideChar Failed!";
		}

		ret = WideCharToMultiByte(CP_UTF8, 0, wideBuffer, -1, (LPSTR)utf8_buffer, 4096, 0, 0);
		if (ret == 0)
		{
			std::cerr << "(error) AnsiToUTF8 MultiByteToWideChar Failed!";
		}
#else
		assert(0 && "Not implemented function.");
#endif
		return utf8_buffer;
	}

	// return data is temporary data. save it to other memory if you need
	WCHAR* AnsiToWide(const char* source, int size)
	{
		if (size == 0)
			size = (int)strlen(source);
		static WCHAR wideBuffer[4096];
        
		memset(wideBuffer, 0, 4096 * sizeof(WCHAR));

		bool err = false;
#if defined(_PLATFORM_WINDOWS_)
		int ret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, wideBuffer, 2048);
		err = ret == 0;		
#else
		std::string localeBackup = std::setlocale(LC_ALL, NULL);
		std::setlocale(LC_ALL, "");

		std::mbstate_t mbState = std::mbstate_t();
		auto ret = std::mbsrtowcs(wideBuffer, &source, 4096, &mbState);
		err = ret == -1;		

		std::setlocale(LC_ALL, localeBackup.c_str());
#endif
		if (err){
			std::cerr << "AnsiToWide MultiByteToWideChar Failed!";
		}
		return wideBuffer;
	}

	WCHAR* AnsiToWide(const char* source)
	{
		return AnsiToWide(source, (int)strlen(source));
	}
	WCHAR AnsiToWide(const char source)
	{
		char buf[] = { source, 0 };
		auto wideBuffer = AnsiToWide(buf, 2);
		return wideBuffer[0];
	}

	const char* WideToAnsi(const WCHAR* source)
	{
		static char ansiBuffer[4096];
		memset(ansiBuffer, 0, 4096);
		bool err = false;
#if defined(_PLATFORM_WINDOWS_)
		int ret = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)source,
			-1, ansiBuffer, 4096, NULL, NULL);
		err = ret == 0;
#else
		std::string localeBackup = std::setlocale(LC_ALL, NULL);
		std::setlocale(LC_ALL, "");

		std::mbstate_t mbState = std::mbstate_t();		
		auto ret = std::wcsrtombs(ansiBuffer, &source, 4096, &mbState);
		err = ret == -1;

		std::setlocale(LC_ALL, localeBackup.c_str());
#endif
		if (ret == 0)		
			std::cerr << "WideToAnsi Failed!";

		return ansiBuffer;
	}

	size_t Length(std::stringstream& s) {
		auto pos = s.tellg();
		s.seekg(0, std::ios::end);
		auto size = s.tellg();
		s.seekg(pos, std::ios::end);
		return (size_t)size;
	}

	HashedString::HashedString(char const * const pIdentString)
		: mIdent(hash_name(pIdentString))
		, mIdentStr(pIdentString)
	{
	}

	unsigned long HashedString::getHashValue(void) const
	{
		return reinterpret_cast<unsigned long>(mIdent);
	}

	const std::string & HashedString::getStr() const
	{
		return mIdentStr;
	}

	bool HashedString::operator< (HashedString const & o) const
	{
		bool r = (getHashValue() < o.getHashValue());
		return r;
	}

	bool HashedString::operator== (HashedString const & o) const
	{
		bool r = (getHashValue() == o.getHashValue());
		return r;
	}

	void * HashedString::hash_name(char const * pIdentStr)
	{
		// Relatively simple hash of arbitrary text string into a
		// 32-bit identifier Output value is
		// input-valid-deterministic, but no guarantees are made
		// about the uniqueness of the output per-input
		//
		// Input value is treated as lower-case to cut down on false
		// separations cause by human mistypes. Sure, it could be
		// construed as a programming error to mix up your cases, and
		// it cuts down on permutations, but in Real World Usage
		// making this text case-sensitive will likely just lead to
		// Pain and Suffering.
		//
		// This code lossely based upon the adler32 checksum by Mark
		// Adler and published as part of the zlib compression
		// library sources.

		// largest prime smaller than 65536
		unsigned long BASE = 65521L;

		// NMAX is the largest n such that 255n(n+1)/2 +
		// (n+1)(BASE-1) <= 2^32-1
		unsigned long NMAX = 5552;

#define DO1(buf,i)  {s1 += tolower(buf[i]); s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

		if (pIdentStr == NULL)
			return NULL;

		unsigned long s1 = 0;
		unsigned long s2 = 0;

		for (size_t len = strlen(pIdentStr); len > 0;)
		{
			unsigned long k = len < NMAX ? len : NMAX;

			len -= k;

			while (k >= 16)
			{
				DO16(pIdentStr);
				pIdentStr += 16;
				k -= 16;
			}

			if (k != 0) do
			{
				s1 += tolower(*pIdentStr++);
				s2 += s1;
			} while (--k);

			s1 %= BASE;
			s2 %= BASE;
		}

#pragma warning(push)
#pragma warning(disable : 4312)

		return reinterpret_cast<void *>((s2 << 16) | s1);

#pragma warning(pop)
#undef DO1
#undef DO2
#undef DO4
#undef DO8
#undef DO16
	}
}
