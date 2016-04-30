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