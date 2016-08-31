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
#include "StringConverter.h"
#include "StringLib.h"
namespace fb{
	TString StringConverter::ToString(Real val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		std::_tstringstream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}
	//-----------------------------------------------------------------------
	TString StringConverter::ToString(int val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		std::_tstringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}
	//-----------------------------------------------------------------------
#ifdef _WIN64
#else
	TString StringConverter::ToString(size_t val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		std::_tstringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}
#endif
	//-----------------------------------------------------------------------
	TString StringConverter::ToString(unsigned long val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		std::_tstringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	//-----------------------------------------------------------------------
	TString StringConverter::ToStringK(unsigned val)
	{
		std::_tstringstream stream;
		if (val > 1000)
		{
			val += 500;
			val /= 1000;
			stream << val << "K";
		}
		else
		{
			stream << val;
		}
		return stream.str();
	}
	//-----------------------------------------------------------------------
	TString StringConverter::ToString(long val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		std::_tstringstream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	TString StringConverter::ToString(UINT64 val) {
		std::_tstringstream stream;				
		stream << val;
		return stream.str();
	}

	//-----------------------------------------------------------------------
	TString StringConverter::ToString(bool val, bool yesNo)
	{
		if (val)
		{
			if (yesNo)
			{
				return _T("yes");
			}
			else
			{
				return _T("true");
			}
		}
		else
			if (yesNo)
			{
				return _T("no");
			}
			else
			{
				return _T("false");
			}
	}
	
	//-----------------------------------------------------------------------
	TString StringConverter::ToString(const TStringVector& val)
	{
		std::_tstringstream stream;
		TStringVector::const_iterator i, iend, ibegin;
		ibegin = val.begin();
		iend = val.end();
		for (i = ibegin; i != iend; ++i)
		{
			if (i != ibegin)
				stream << " ";

			stream << *i;
		}
		return stream.str();
	}

	TString StringConverter::ToHexa(unsigned val){
		std::stringstream stream;
		stream << "0x" << std::hex << val;
		return stream.str();
	}

	//-----------------------------------------------------------------------
	Real StringConverter::ParseReal(const TString& val, Real defaultValue)
	{
		// Use istd::_tstringstream for direct correspondence with ToString
		std::_tstringstream str(val);
		Real ret = defaultValue;
		str >> ret;
		return ret;
	}
	//-----------------------------------------------------------------------
	int StringConverter::ParseInt(const TString& val, int defaultValue)
	{
		if (val[0] == '0' && val[1] == 'x')
		{
			unsigned c = StringConverter::ParseHexa(val, defaultValue);
			return (int)c;
		}

		// Use istd::_tstringstream for direct correspondence with ToString
		std::_tstringstream str(val);
		int ret = defaultValue;
		str >> ret;

		return ret;
	}
	//-----------------------------------------------------------------------
	unsigned int StringConverter::ParseUnsignedInt(const TString& val, unsigned int defaultValue)
	{
		// Use istd::_tstringstream for direct correspondence with ToString
		std::_tstringstream str(val);
		unsigned int ret = defaultValue;
		str >> ret;

		return ret;
	}

	unsigned int StringConverter::ParseHexa(const TString& val, unsigned int defaultValue)
	{
		std::_tstringstream str(val);
		unsigned int ret = defaultValue;
		str >> std::hex >> ret;
		return ret;
	}
	//-----------------------------------------------------------------------
	long StringConverter::ParseLong(const TString& val, long defaultValue)
	{
		// Use istd::_tstringstream for direct correspondence with ToString
		std::_tstringstream str(val);
		long ret = defaultValue;
		str >> ret;

		return ret;
	}
	//-----------------------------------------------------------------------
	unsigned long StringConverter::ParseUnsignedLong(const TString& val, unsigned long defaultValue)
	{
		// Use istd::_tstringstream for direct correspondence with ToString
		std::_tstringstream str(val);
		unsigned long ret = defaultValue;
		str >> ret;

		return ret;
	}
	unsigned long long StringConverter::ParseUnsignedLongLong(const TString& val, unsigned long long defaultValue)
	{
		std::_tstringstream str(val);
		unsigned long long ret = defaultValue;
		str >> ret;

		return ret;
	}

	UINT64 StringConverter::ParseUINT64(const TString& val, unsigned long long defaultValue) {
		return ParseUnsignedLongLong(val, defaultValue);
	}

	//-----------------------------------------------------------------------
	bool StringConverter::ParseBool(const TString& val, bool defaultValue)
	{
		if ((StartsWith(val, _T("true")) || StartsWith(val, _T("yes"))
			|| StartsWith(val, _T("1"))))
			return true;
		else if ((StartsWith(val, _T("false")) || StartsWith(val, _T("no"))
			|| StartsWith(val, _T("0"))))
			return false;
		else
			return defaultValue;
	}

	//-----------------------------------------------------------------------
	TStringVector StringConverter::ParseStringVector(const TString& val, const std::string& delims,
		unsigned int maxSplits , bool preserveDelims)
	{
		return Split(val, delims, maxSplits, preserveDelims);
	}

	//-----------------------------------------------------------------------
	bool StringConverter::IsNumber(const TString& val)
	{
		std::_tstringstream str(val);
		float tst;
		str >> tst;
		return !str.fail() && str.eof();
	}
}