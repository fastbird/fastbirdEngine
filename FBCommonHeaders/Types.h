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

// Should not define data in this file.
// Only typedefs or defines are allowed.
#pragma once
#define FBCommonHeaders_Types_h
#include <utility>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

#define FB_DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name


typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned __int64 UINT64;
typedef __int64 INT64;
#define FB_INVALID_REAL -FLT_MAX
namespace fb{
	typedef intptr_t ModuleHandle;
	typedef intptr_t FunctionHandle;
	//#define FB_DOUBLE_PRECISION
	typedef float Real;
	typedef std::vector<Real> RealArray;
	typedef std::vector<float> FloatArray;
	typedef std::vector<int> IntArray;
	typedef std::vector<unsigned char> ByteArray;
	typedef std::shared_ptr<ByteArray> ByteArrayPtr;

	typedef __int64 HWindowId;
	static const HWindowId INVALID_HWND_ID = (HWindowId)-1;
	typedef std::vector<std::string> StringVector;
	typedef std::vector<std::wstring> WStringVector;	
	// unsigned int : safe for 828 'days' at 60 frames/sec
	// unsigned long long : safe for 9749040289 'years' at 60 frames/sec
	typedef unsigned int FRAME_PRECISION;
	typedef float TIME_PRECISION;

	typedef size_t FunctionId;
	static const FunctionId INVALID_FUNCTION_ID = -1;

	typedef unsigned AudioId;
	static const AudioId INVALID_AUDIO_ID = -1;

	FB_DECLARE_HANDLE(HWindow);
	static const HWindow INVALID_HWND = (HWindow)-1;
	typedef std::lock_guard<std::mutex> MutexLock;

	typedef std::tuple<int, int> Vec2ITuple;
	typedef std::tuple<int, int, int> Vec3ITuple;
	typedef std::tuple<Real, Real> Vec2Tuple;
	typedef std::tuple<Real, Real, Real> Vec3Tuple;
	typedef std::tuple<Real, Real, Real, Real> Vec4Tuple;
	typedef std::vector<Vec3Tuple> Vec3Tuples;
	typedef struct QUAT_TUPLE{
		QUAT_TUPLE(){};
		QUAT_TUPLE(Real w, Real x, Real y, Real z)
			:value(w, x, y, z)	{}
		std::tuple < Real, Real, Real, Real > value;
	} QuatTuple;
	typedef std::tuple <
		// rotation
		Real, Real, Real,
		Real, Real, Real,
		Real, Real, Real,
		// quaternion
		Real, Real, Real, Real,
		// translation
		Real, Real, Real,
		// scale
		Real, Real, Real,
		// itentity, RSSeperated, UniformScale
		bool, bool, bool> TransformationTuple;

	template <typename T>
	bool operator == (const std::weak_ptr<T>& a, const std::shared_ptr<T>& b)
	{
		return a.lock() == b;
	}

	template <typename T>
	bool operator != (const std::weak_ptr<T>& a, const std::shared_ptr<T>& b)
	{
		return a.lock() != b;
	}

	template <typename T>
	bool operator == (const std::weak_ptr<T>& a, const std::weak_ptr<T>& b)
	{
		return a.lock() == b.lock();
	}	
}
using fb::operator==;
using fb::operator!=;


#define FB_DECLARE_NON_COPYABLE(className) \
	className(const className&) = delete;\
	className& operator= (const className&) = delete

#define FB_DECLARE_PIMPL(className) \
	class Impl; \
	std::unique_ptr<Impl> mImpl

#define FB_DECLARE_PIMPL_NON_COPYABLE(className) \
	FB_DECLARE_PIMPL(className); \
	FB_DECLARE_NON_COPYABLE(className)

#define FB_DECLARE_PIMPL_CLONEABLE(className) \
	FB_DECLARE_PIMPL(className); \
protected:\
	className(const className&);\
	className& operator= (const className&) = delete;\
private:\


#define FB_DECLARE_SMART_PTR(className) \
	class className;\
	typedef std::shared_ptr<className> className##Ptr;\
	typedef std::shared_ptr<const className> className##ConstPtr;\
	typedef std::weak_ptr<className> className##WeakPtr

#define FB_DECLARE_SMART_PTR2(class_name) \
	class class_name;\
	typedef std::shared_ptr<class_name> class_name##_ptr;\
	typedef std::shared_ptr<const class_name> class_name##_constptr;\
	typedef std::weak_ptr<class_name> class_name##_weakptr

#define FB_DECLARE_SMART_PTR_STRUCT(className) \
	struct className;\
	typedef std::shared_ptr<className> className##Ptr;\
	typedef std::weak_ptr<className> className##WeakPtr

#define FB_IMPLEMENT_STATIC_CREATE(className)\
	className##Ptr className##::Create(){\
		return className##Ptr(new className, [](className* obj){delete obj;});\
	}

#define FB_IMPLEMENT_STATIC_CREATE_SELF_PTR(className)\
	className##Ptr className##::Create(){\
		auto p = className##Ptr(new className, [](className* obj){delete obj;});\
		p->mImpl->mSelfPtr = p;\
		return p;\
	}

#define OVERRIDE override