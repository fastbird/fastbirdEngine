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
#include <algorithm>
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

	

#define ValidCStringLength(szStr) szStr && strlen((szStr))

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