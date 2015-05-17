#include <CommonLib/StdAfx.h>
#include "Unicode.h"
#include "Debug.h"

namespace fastbird
{

int DecodeUTF8(const unsigned char *encodedBuffer, unsigned int *outLength)
{
	const unsigned char *buf = (const unsigned char*)encodedBuffer;
	
	int value = 0;
	int length = -1;
	unsigned char byte = buf[0];
	if( (byte & 0x80) == 0 )
	{
		// This is the only byte
		if( outLength ) *outLength = 1;
		return byte;
	}
	else if( (byte & 0xE0) == 0xC0 )
	{
		// There is one more byte
		value = int(byte & 0x1F);
		length = 2;

		// The value at this moment must not be less than 2, because 
		// that should have been encoded with one byte only.
		if( value < 2 )
			length = -1;
	}
	else if( (byte & 0xF0) == 0xE0 )
	{
		// There are two more bytes
		value = int(byte & 0x0F);
		length = 3;
	}
	else if( (byte & 0xF8) == 0xF0 )
	{
		// There are three more bytes
		value = int(byte & 0x07);
		length = 4;
	}

	int n = 1;
	for( ; n < length; n++ )
	{
		byte = buf[n];
		if( (byte & 0xC0) == 0x80 )
			value = (value << 6) + int(byte & 0x3F);
		else 
			break;
	}

	if( n == length )
	{
		if( outLength ) *outLength = (unsigned)length;
		return value;
	}

	// The byte sequence isn't a valid UTF-8 byte sequence.
	return -1;
}

int EncodeUTF8(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outLength)
{
	unsigned char *buf = (unsigned char*)outEncodedBuffer;

	int length = -1;

	if( value <= 0x7F )
	{
		buf[0] = value;
		if( outLength ) *outLength = 1;
		return 1;
	}
	else if( value >= 0x80 && value <= 0x7FF )
	{
		// Encode it with 2 characters
		buf[0] = 0xC0 + (value >> 6);
		length = 2;
	}
	else if( value >= 0x800 && value <= 0xD7FF || value >= 0xE000 && value <= 0xFFFF )
	{
		// Note: Values 0xD800 to 0xDFFF are not valid unicode characters
		buf[0] = 0xE0 + (value >> 12);
		length = 3;
	}
	else if( value >= 0x10000 && value <= 0x10FFFF )
	{
		buf[0] = 0xF0 + (value >> 18);
		length = 4;
	}

	int n = length-1;
	for( ; n > 0; n-- )
	{
		buf[n] = 0x80 + (value & 0x3F);
		value >>= 6;
	}

	if( outLength ) *outLength = length;
	return length;
}

int DecodeUTF16(const unsigned char *encodedBuffer, unsigned int *outLength, EUnicodeByteOrder byteOrder)
{
	const unsigned char *buf = (const unsigned char *)encodedBuffer;
	int value = 0;
	if( byteOrder == LITTLE_ENDIAN )
	{
		value += buf[0];
		value += (unsigned int)(buf[1]) << 8; 
	}
	else
	{
		value += buf[1];
		value += (unsigned int)(buf[0]) << 8; 
	}

	if( value < 0xD800 || value > 0xDFFF )
	{
		if( outLength ) *outLength = 2;
		return value;
	}
	else if( value < 0xDC00 )
	{
		// We've found the first surrogate word
		value = ((value & 0x3FF)<<10);

		// Read the second surrogate word
		int value2 = 0;
		if( byteOrder == LITTLE_ENDIAN )
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
		if( value2 < 0xDC00 || value2 > 0xDFFF )
			return -1;

		value = value + (value2 & 0x3FF) + 0x10000;
		if( outLength ) *outLength = 4;
		return value;
	}
	
	// It is an illegal sequence if a character in the 0xDC00-0xDFFF range comes first
	return -1;
}

int EncodeUTF16(unsigned int value, unsigned char *outEncodedBuffer, unsigned int *outCharLength, EUnicodeByteOrder byteOrder)
{
	if( value < 0x10000 )
	{
		if( byteOrder == LITTLE_ENDIAN )
		{
			outEncodedBuffer[0] = (value & 0xFF);
			outEncodedBuffer[1] = ((value >> 8) & 0xFF);
		}
		else
		{
			outEncodedBuffer[1] = (value & 0xFF);
			outEncodedBuffer[2] = ((value >> 8) & 0xFF);
		}

		if( outCharLength ) *outCharLength = 2;
		return 2;
	}
	else
	{
		value -= 0x10000;
		int surrogate1 = ((value >> 10) & 0x3FF) + 0xD800;
		int surrogate2 = (value & 0x3FF) + 0xDC00;

		if( byteOrder == LITTLE_ENDIAN )
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

		if( outCharLength ) *outCharLength = 4;
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
	int ret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, wideBuffer, 4096);
	if (ret==0)
	{
		DebugOutput(FB_DEFAULT_DEBUG_ARG, "AnsiToUTF8 MultiByteToWideChar Failed!");
	}
	
	ret = WideCharToMultiByte( CP_UTF8, 0, wideBuffer, -1, (LPSTR)utf8_buffer, 4096, 0, 0 );
	if (ret==0)
	{
		DebugOutput(FB_DEFAULT_DEBUG_ARG, "AnsiToUTF8 WideCharToMultiByte Failed!");
	}
	return utf8_buffer;
}

// return data is temporary data. save it to other memory if you need
WCHAR* AnsiToWide(const char* source, int size)
{
	if (size==0)
		size = strlen(source);
	static WCHAR wideBuffer[4096];
	memset(wideBuffer, 0, 4096 * sizeof(WCHAR));
	int ret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, wideBuffer, 2048);
	if (ret==0)
	{
		DebugOutput(FB_DEFAULT_DEBUG_ARG, "AnsiToWide MultiByteToWideChar Failed!");
	}
	return wideBuffer;
}

WCHAR* AnsiToWide(const char* source)
{
	return AnsiToWide(source, strlen(source));
}
WCHAR AnsiToWide(const char source)
{
	static WCHAR wideBuffer[4];
	memset(wideBuffer, 0, 4 * sizeof(WCHAR));
	int ret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &source, 1, wideBuffer, 2);
	return wideBuffer[0];
}

// return data is temporary data. save it to other memory if you need
WCHAR* UTF8ToWide(const unsigned char* source)
{
	static WCHAR wideBuffer[4096];
	memset(wideBuffer, 0, 4096 * sizeof(WCHAR));
	int ret = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)source, -1, wideBuffer, 4096);
	if (ret==0)
	{
		DebugOutput(FB_DEFAULT_DEBUG_ARG, "UTF8ToWide MultiByteToWideChar Failed!");
	}
	return wideBuffer;
}

const char* WideToAnsi(const WCHAR* source)
{
	static char ansiBuffer[4096];
	memset(ansiBuffer, 0, 4096);
	int ret = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)source, 
		-1, ansiBuffer, 4096, NULL, NULL);
	if (ret==0)
	{
		DebugOutput(FB_DEFAULT_DEBUG_ARG, "WideToAnsi WideCharToMultiByte Failed!");
	}
	return ansiBuffer;
}

}