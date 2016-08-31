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
#include "BitManipulator.h"

namespace fb {
	Endianness ConvertEndianness(const char* str) {
		if (_stricmp("BigEndian", str) == 0)
			return FB_BIG_ENDIAN;

		return FB_LITTLE_ENDIAN;
	}

	const char* ConvertEndianness(Endianness endian) {
		if (endian == FB_BIG_ENDIAN)
			return "BigEndian";

		return "LittleEndian";
	}

	const char* PODTypesStr[] = {
		"INT8", "INT16", "INT32", "INT64",
		"FLOAT32", "FLOAT64", "PODNum"
	};

	PODTypes ConvertPODType(const char* str) {
		for (int i = 0; i < PODNum; ++i) {
			if (_stricmp(str, PODTypesStr[i]) == 0)
				return PODTypes(i);
		}
		return PODNum;
	}

	const char* ConvertPODType(PODTypes type) {
		return PODTypesStr[type];
	}

	float ReverseFloat(float& value)
	{
		char *floatToConvert = (char*)&value;
		std::swap(floatToConvert[0], floatToConvert[3]);
		std::swap(floatToConvert[1], floatToConvert[2]);
		return value;
	}
	double ReverseFloat(double& value)
	{
		char *floatToConvert = (char*)&value;
		std::swap(floatToConvert[0], floatToConvert[7]);
		std::swap(floatToConvert[1], floatToConvert[6]);
		std::swap(floatToConvert[2], floatToConvert[5]);
		std::swap(floatToConvert[3], floatToConvert[4]);
		return value;
	}

	RealArray ConvertBytesToRealsInt8(char* source, size_t size);
	RealArray ConvertBytesToRealsInt16(char* source, size_t size);
	RealArray ConvertBytesToRealsInt32(char* source, size_t size);
	RealArray ConvertBytesToRealsInt64(char* source, size_t size);
	RealArray ConvertBytesToRealsFloat32(char* source, size_t size);
	RealArray ConvertBytesToRealsFloat64(char* source, size_t size);

	RealArray ConvertBytesToRealsInt16Swap(char* source, size_t size);
	RealArray ConvertBytesToRealsInt32Swap(char* source, size_t size);
	RealArray ConvertBytesToRealsInt64Swap(char* source, size_t size);
	RealArray ConvertBytesToRealsFloat32Swap(char* source, size_t size);
	RealArray ConvertBytesToRealsFloat64Swap(char* source, size_t size);

	Endianness GetSystemEndianness() {
		int A = 1;
		return *((char*)&A) == 1 ? FB_LITTLE_ENDIAN : FB_BIG_ENDIAN;
	}

	UINT8 Convert8(const unsigned char* p) {
		return *(UINT8*)p;
	}

	INT8 Convert8I(const unsigned char* p) {
		return *(INT8*)p;
	}

	UINT16 Convert16(const unsigned char* p) {
		return *(UINT16*)p;
	}

	INT16 Convert16I(const unsigned char* p) {
		return *(INT16*)p;
	}

	UINT32 Convert32(const unsigned char* p) {
		return *(UINT32*)p;
	}

	INT32 Convert32I(const unsigned char* p) {
		return *(INT32*)p;
	}

	UINT64 Convert64(const unsigned char* p) {
		return *(UINT64*)p;
	}

	INT64 Convert64I(const unsigned char* p) {
		return *(INT64*)p;
	}

	float ConvertF32(const unsigned char* p) {
		return *(float*)p;
	}

	double ConvertF64(const unsigned char* p) {
		return *(double*)p;
	}

	UINT16 Convert16Swap(const unsigned char* p) {
		return _byteswap_ushort(*(UINT16*)p);
	}

	INT16 Convert16ISwap(const unsigned char* p) {
		return (INT16)_byteswap_ushort(*(UINT16*)p);
	}

	UINT32 Convert32Swap(const unsigned char* p) {
		return _byteswap_ulong(*(UINT32*)p);
	}

	INT32 Convert32ISwap(const unsigned char* p) {
		return (INT32)_byteswap_ulong(*(UINT32*)p);
	}

	UINT64 Convert64Swap(const unsigned char* p) {
		return _byteswap_uint64(*(UINT64*)p);
	}

	INT64 Convert64ISwap(const unsigned char* p) {
		return (INT64)_byteswap_uint64(*(UINT64*)p);
	}

	float ConvertF32Swap(const unsigned char* p) {
		return ReverseFloat(*(float*)p);
	}

	double ConvertF64Swap(const unsigned char* p) {
		return *(double*)p;
	}

	RealArray ConvertBytesToReals(const ByteArray& source, PODTypes sourceType, Endianness encodeType) {		
		if (source.empty())
			return{};

		return ConvertBytesToReals((char*)&source[0], source.size(), sourceType, encodeType);
	}

	RealArray ConvertBytesToReals(char* source, size_t size, PODTypes sourceType, Endianness encodeType) {
		if (GetSystemEndianness() != encodeType) {
			return ConvertBytesToRealsEndianSwap(source, size, sourceType);
		}

		switch (sourceType) {
		case Int8:
			return ConvertBytesToRealsInt8(source, size);
		case Int16:
			return ConvertBytesToRealsInt16(source, size);
		case Int32:
			return ConvertBytesToRealsInt32(source, size);
		case Float32:
			return ConvertBytesToRealsFloat32(source, size);
		case Int64:
			return ConvertBytesToRealsInt64(source, size);
		case Float64:
			return ConvertBytesToRealsFloat64(source, size);
		default: {
			assert(0 && "Invalid source type.");
			return{};
		}
		}
	}

	RealArray ConvertBytesToRealsInt8(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; ++i, ++p) {
			ret.push_back((Real)Convert8I((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt16(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 2, p += 2) {
			ret.push_back((Real)Convert16I((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt32(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 4, p += 4) {
			ret.push_back((Real)Convert32I((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt64(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 8, p += 8) {
			ret.push_back((Real)Convert64I((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsFloat32(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 4, p += 4) {
			ret.push_back((Real)ConvertF32((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsFloat64(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 8, p += 8) {
			ret.push_back((Real)ConvertF64((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt16Swap(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 2, p += 2) {
			ret.push_back((Real)(short)Convert16ISwap((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt32Swap(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 4, p += 4) {
			ret.push_back((Real)Convert32ISwap((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsInt64Swap(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 8, p += 8) {
			ret.push_back((Real)Convert64ISwap((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsFloat32Swap(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 4, p += 4) {
			ret.push_back((Real)ConvertF32Swap((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsFloat64Swap(char* source, size_t size) {		
		RealArray ret;
		auto p = &source[0];
		for (size_t i = 0; i < size; i += 8, p += 8) {
			ret.push_back((Real)ConvertF64Swap((const unsigned char*)p));
		}
		return ret;
	}

	RealArray ConvertBytesToRealsEndianSwap(char* source, size_t size, PODTypes sourceType) {
		switch (sourceType) {
		case Int8:
			return ConvertBytesToRealsInt8(source, size);
		case Int16:
			return ConvertBytesToRealsInt16Swap(source, size);
		case Int32:
			return ConvertBytesToRealsInt32Swap(source, size);
		case Float32:
			return ConvertBytesToRealsFloat32Swap(source, size);
		case Int64:
			return ConvertBytesToRealsInt64Swap(source, size);
		case Float64:
			return ConvertBytesToRealsFloat64Swap(source, size);
		default: {
			assert(0 && "Invalid source type.");
			return{};
		}
		}
	}
}