#pragma once

namespace fastbird
{
	enum EUnicodeByteOrder
	{
		LITTLE_ENDIAN,
		BIG_ENDIAN,
	};

	// This function will attempt to decode a UTF-8 encoded character in the buffer.
	// If the encoding is invalid, the function returns -1.
	int DecodeUTF8(const unsigned char *encodedBuffer, unsigned int *outCharLength);

	// This function will encode the value into the buffer.
	// If the value is invalid, the function returns -1, else the encoded length.
	int EncodeUTF8(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength);

	// This function will attempt to decode a UTF-16 encoded character in the buffer.
	// If the encoding is invalid, the function returns -1.
	int DecodeUTF16(const unsigned char *encodedBuffer, unsigned int *outCharLength, EUnicodeByteOrder byteOrder = LITTLE_ENDIAN);

	// This function will encode the value into the buffer.
	// If the value is invalid, the function returns -1, else the encoded length.
	int EncodeUTF16(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength, EUnicodeByteOrder byteOrder = LITTLE_ENDIAN);
	
	// return data is temporary data. save it to other memory if you need
	unsigned char* AnsiToUTF8(const char* source);
	
	// return data is temporary data. save it to other memory if you need
	WCHAR* AnsiToWide(const char* source, int size);
	WCHAR* AnsiToWide(const char* source);
	WCHAR AnsiToWide(const char source);

	// return data is temporary data. save it to other memory if you need
	WCHAR* UTF8ToWide(const unsigned char* source);

	const char* WideToAnsi(const WCHAR* source);
}