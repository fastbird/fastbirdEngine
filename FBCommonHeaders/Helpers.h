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
#include "Types.h"
#include <algorithm>
#undef min
#undef max
/**
\file FBHelpers.h
Convenent macros for manipulating stl containers.
\author Jungwan Byun
\ingroup FBCommonHeaders
*/
#define ValueExistsInVector(arr, v)	(std::find(arr.begin(), arr.end(), v) != arr.end())

#define DeleteValuesInVector_Size(arr, v) \
	unsigned arr ## SizeBefore = arr.size(); \
	arr.erase(std::remove(arr.begin(), arr.end(), v), arr.end()); \
	unsigned arr ## SizeAfter = arr.size();

#define DeleteValuesInVector(arr, v) \
	auto arr##It = std::remove(arr.begin(), arr.end(), v);\
	bool arr##DeletedAny = arr##It != arr.end();\
	arr.erase(arr##It, arr.end());	

#define DeleteValuesInList(arr, v) \
	unsigned arr ## SizeBefore = arr.size(); \
	for (auto it = arr.begin(); it != arr.end();)\
		{\
		if ((*it) == v)\
				{\
			it = mChildren.erase(it);\
				}\
				else\
				{\
			++it;\
				}\
		}\
	unsigned arr ## SizeAfter = arr.size();

#define IteratingWeakContainer(container, it, var)\
	auto (var) = (it)->lock();\
	if (!(var)){\
		(it) = (container).erase((it));\
		continue;\
	}\
	++it;\

	

#define ValidCString(szStr) szStr && szStr[0]!=0

#define ARRAYCOUNT(A)       (sizeof(A) / sizeof(A[0]))

#define if_assert_pass(V) assert((V)); \
if ((V))

//assert((V));
#define if_assert_fail(V)  \
if (!(V))

template <class T>
void ClearWithSwap(T& m)
{
	T empty;
	std::swap(m, empty);
}

namespace fb{
	inline float GetDistanceBetween(const Vec3Tuple& a, const Vec3Tuple& b){
		float ax = std::get<0>(a);
		float ay = std::get<1>(a);
		float az = std::get<2>(a);

		float bx = std::get<0>(b);
		float by = std::get<1>(b);
		float bz = std::get<2>(b);

		float mx = ax - bx;
		float my = ay - by;
		float mz = az - bz;

		return sqrt(mx*mx + my*my + mz*mz);
	}

	inline float GetDistanceSQBetween(const Vec3Tuple& a, const Vec3Tuple& b){
		float ax = std::get<0>(a);
		float ay = std::get<1>(a);
		float az = std::get<2>(a);

		float bx = std::get<0>(b);
		float by = std::get<1>(b);
		float bz = std::get<2>(b);

		float mx = ax - bx;
		float my = ay - by;
		float mz = az - bz;

		return mx*mx + my*my + mz*mz;
	}

	template<class T>
	unsigned RemoveInvalidWeakPtr(std::vector<T>& v){
		unsigned numDeleted = 0;
		for (auto it = v.begin(); it != v.end(); /**/){
			auto data = it->lock();
			if (!data){
				it = v.erase(it);
				++numDeleted;
				continue;
			}
			++it;
		}
		return numDeleted;
	}

	inline size_t GetStringMemSize(const std::string& str) {
		return sizeof(size_t) + str.length();
	}

	template <size_t _Size>
	inline unsigned ByteBufferToString(const ByteArray& byteBuffer, char (&dest)[_Size]) {
		if (byteBuffer.empty())
			return 0;
		memset(dest, 0, _Size);
		auto len = byteBuffer.size();
		auto copiedLen = std::min(_Size, len);
		if (copiedLen == _Size)
			copiedLen = _Size - 1;
		strncpy_s(dest, (const char*)&byteBuffer[0], copiedLen);
		return copiedLen;
	}

	inline size_t hash_combine_ret(size_t a, size_t b) {
		a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
		return a;
	}

	inline void hash_combine(size_t& a, size_t b) {
		a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);		
	}
}