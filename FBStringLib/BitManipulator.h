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
#include "FBCommonHeaders/Types.h"
namespace fb {
	enum Endianness {
		FB_LITTLE_ENDIAN,
		FB_BIG_ENDIAN,
	};
	Endianness ConvertEndianness(const char* str);
	const char* ConvertEndianness(Endianness endian);

	enum PODTypes {
		Int8,
		Int16,
		Int32,
		Int64,
		Float32,
		Float64,
		PODNum,
	};

	PODTypes ConvertPODType(const char* str);
	const char* ConvertPODType(PODTypes type);

	Endianness GetSystemEndianness();
	RealArray ConvertBytesToReals(const ByteArray& source, PODTypes sourceType, Endianness encodeType);	
	RealArray ConvertBytesToReals(char* source, size_t size, PODTypes sourceType, Endianness encodeType);
	RealArray ConvertBytesToRealsEndianSwap(char* source, size_t size, PODTypes sourceType);
}