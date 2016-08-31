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
#include "MurmurHash.h"

namespace fb
{
unsigned int murmur3_32(const char *key, unsigned int len, unsigned int seed)
{
	static const unsigned int c1 = 0xcc9e2d51;
	static const unsigned int c2 = 0x1b873593;
	static const unsigned int r1 = 15;
	static const unsigned int r2 = 13;
	static const unsigned int m = 5;
	static const unsigned int n = 0xe6546b64;
 
	unsigned int hash = seed;
 
	const int nblocks = len / 4;
	const unsigned int *blocks = (const unsigned int *) key;
	int i;
	for (i = 0; i < nblocks; i++) {
		unsigned int k = blocks[i];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
 
		hash ^= k;
		hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
	}
 
	const unsigned char *tail = (const unsigned char *) (key + nblocks * 4);
	unsigned int k1 = 0;
 
	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];
 
		k1 *= c1;
		k1 = (k1 << r1) | (k1 >> (32 - r1));
		k1 *= c2;
		hash ^= k1;
	}
 
	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);
 
	return hash;
}

/**
* Generates 64 bit hash from byte array of the given length and seed.
*
* @param data byte array to hash
* @param length length of the array to hash
* @param seed initial seed value
* @return 64 bit hash of the given array
*/
UINT64 hash64(const char* data, int length, int seed) {
	UINT64 m = 0xc6a4a7935bd1e995L;
	int r = 47;

	UINT64 h = (seed & 0xffffffffl) ^ (length*m);

	int length8 = length / 8;

	for (int i = 0; i<length8; i++) {
		int i8 = i * 8;
		UINT64 k = ((UINT64)data[i8 + 0] & 0xff) + (((UINT64)data[i8 + 1] & 0xff) << 8)
			+ (((UINT64)data[i8 + 2] & 0xff) << 16) + (((UINT64)data[i8 + 3] & 0xff) << 24)
			+ (((UINT64)data[i8 + 4] & 0xff) << 32) + (((UINT64)data[i8 + 5] & 0xff) << 40)
			+ (((UINT64)data[i8 + 6] & 0xff) << 48) + (((UINT64)data[i8 + 7] & 0xff) << 56);

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	switch (length % 8) {
	case 7: h ^= (UINT64)(data[(length&~7) + 6] & 0xff) << 48;
	case 6: h ^= (UINT64)(data[(length&~7) + 5] & 0xff) << 40;
	case 5: h ^= (UINT64)(data[(length&~7) + 4] & 0xff) << 32;
	case 4: h ^= (UINT64)(data[(length&~7) + 3] & 0xff) << 24;
	case 3: h ^= (UINT64)(data[(length&~7) + 2] & 0xff) << 16;
	case 2: h ^= (UINT64)(data[(length&~7) + 1] & 0xff) << 8;
	case 1: h ^= (UINT64)(data[length&~7] & 0xff);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

/**
* Generates 64 bit hash from byte array with default seed value.
*
* @param data byte array to hash
* @param length length of the array to hash
* @return 64 bit hash of the given string
*/
UINT64 hash64(const char* data, int length) {
	return hash64(data, length, 0xe17a1465);
}

/**
* Generates 64 bit hash from a string.
*
* @param text string to hash
* @return 64 bit hash of the given string
*/
UINT64 hash64(std::string& text) {
	auto bytes = text.size();
	return hash64(&text[0], bytes);
}

/**
* Generates 64 bit hash from a substring.
*
* @param text string to hash
* @param from starting index
* @param length length of the substring to hash
* @return 64 bit hash of the given array
*/
UINT64 hash64(std::string& text, int from, int length) {
	
	return hash64(text.substr(from, length));
}
}