#include <CommonLib/StdAfx.h>
#include <CommonLib/MurmurHash.h>

namespace fastbird
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
}