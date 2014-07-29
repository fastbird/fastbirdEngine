#pragma once
namespace fastbird
{
const unsigned int murmurSeed = 0xaabbccdd;
unsigned int murmur3_32(const char *key, unsigned int len, unsigned int seed);
}