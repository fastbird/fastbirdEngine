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
#include <cstring>
#include <vector>
namespace fb{
	enum VERTEX_COMPONENT
	{
		VC_POSITION = 0x1,
		VC_NORMAL = 0x2,
		VC_COLOR = 0x4,
		VC_TEXCOORD = 0x8,
		VC_TANGENT = 0x10,
		VC_BLENDINDICES = 0x20,
	};

	enum INPUT_ELEMENT_FORMAT
	{
		INPUT_ELEMENT_FORMAT_FLOAT4,
		INPUT_ELEMENT_FORMAT_FLOAT3,
		INPUT_ELEMENT_FORMAT_UBYTE4,
		INPUT_ELEMENT_FORMAT_FLOAT2,
		INPUT_ELEMET_FORMAT_INT4,

		INPUT_ELEMET_FORMAT_NUM
	};

	static const char* STR_INPUT_ELEMENT_FORMAT[] =
	{
		"FLOAT4",
		"FLOAT3",
		"UBYTE4",
		"FLOAT2",
		"INT4"
	};

	INPUT_ELEMENT_FORMAT InputElementFromString(const char* sz);

	enum INPUT_CLASSIFICATION
	{
		INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		INPUT_CLASSIFICATION_PER_INSTANCE_DATA,

		INPUT_CLASSIFICATION_NUM
	};
	static const char* STR_INPUT_CLASSIFICATION[] =
	{
		"VERTEX",
		"INSTANCE",
	};
	INPUT_CLASSIFICATION InputClassificationFromString(const char* sz);

	struct INPUT_ELEMENT_DESC
	{
		INPUT_ELEMENT_DESC(const char* semantic, unsigned index,
			INPUT_ELEMENT_FORMAT format, unsigned inputSlot,
			unsigned alignedByteOffset, INPUT_CLASSIFICATION slotClass,
			unsigned instanceDataStepRate)
			: mSemanticIndex(index)
			, mFormat(format), mInputSlot(inputSlot)
			, mAlignedByteOffset(alignedByteOffset), mInputSlotClass(slotClass)
			, mInstanceDataStepRate(instanceDataStepRate)
		{
			memset(mSemanticName, 0, 256);
			strcpy_s(mSemanticName, semantic);
		}
		INPUT_ELEMENT_DESC()
		{
			memset(this, 0, sizeof(INPUT_ELEMENT_DESC));
		}
		bool operator==(const INPUT_ELEMENT_DESC& other) const
		{
			return memcmp(this, &other, sizeof(INPUT_ELEMENT_DESC)) == 0;
		}

		bool operator!=(const INPUT_ELEMENT_DESC& other) const
		{
			return !operator==(other);
		}

		bool operator< (const INPUT_ELEMENT_DESC& other) const
		{
			return memcmp(this, &other, sizeof(INPUT_ELEMENT_DESC)) < 0;
		}

		char mSemanticName[256];
		unsigned mSemanticIndex;
		INPUT_ELEMENT_FORMAT mFormat;
		unsigned mInputSlot;
		unsigned mAlignedByteOffset;
		INPUT_CLASSIFICATION mInputSlotClass;
		unsigned mInstanceDataStepRate;
	};

	typedef std::vector<INPUT_ELEMENT_DESC> INPUT_ELEMENT_DESCS;
}