#pragma once
#ifndef _SmartPtr_header_included_
#define _SmartPtr_header_included_

#include <assert.h>

namespace fastbird
{
	class ReferenceCounter
	{
	public:
		ReferenceCounter() 
			:mRefCounter(0)
		{}

		virtual ~ReferenceCounter()
		{}

		inline void AddRef()
		{
			InterlockedIncrement(&mRefCounter);
		}

		inline void Release()
		{
			const int count = InterlockedDecrement(&mRefCounter);
			if (count == 0)
			{
				delete this;
			}
			else if (count < 0)
			{
				assert(0 && "Reference count is lesser than zero!");
			}
		}

		// Debug purpose only.
		int NumRefs()
		{
			return mRefCounter;
		}

	private:
		volatile long mRefCounter;
	};

	template <class T>
	class SmartPtr
	{
	public:
		inline SmartPtr() : mP(0){}
		inline SmartPtr(T* p)
		{
			mP = p;
			if (mP)
				mP->AddRef();
		}

		inline SmartPtr(const SmartPtr& other)
		{
			mP = other.mP;
			if (mP)
				mP->AddRef();
		}
		
		inline ~SmartPtr()
		{
			if (mP)
				mP->Release();
		}

		inline operator T*() const { return mP; }
		inline T& operator*() const { return *mP;}
		inline T* operator->() const { return mP;}
		inline T* operator+() const { return mP; }
		inline T* get() const { return mP; }
		
		inline SmartPtr& operator=(T* newP)
		{
			if (newP)
				newP->AddRef();
			if (mP)
				mP->Release();
			mP = newP;
			return *this;
		}

		inline void reset()
		{
			SmartPtr<T>().swap(*this);
		}

		inline void reset(T* p)
		{
			SmartPtr<T>(p).swap(*this);
		}

		inline SmartPtr& operator=(const SmartPtr& newP)
		{
			if (newP.mP)
				newP.mP->AddRef();
			if (mP)
				mP->Release();
			mP = newP.mP;
			return *this;
		}

		inline bool operator !() const
		{
			return mP==0;
		}

		inline friend T* ReleaseOwndership( SmartPtr<T>& ptr )
		{
			T* ret = ptr.mP;
			ptr.mP = 0;
			return ret;
		}

		inline void swap(SmartPtr<T>& other)
		{
			std::swap(mP, other.mP);
		}


	private:
		T* mP;
	};

	template <class T>
	inline void swap(SmartPtr<T>& a, SmartPtr<T>& b)
	{
		a.swap(b);
	}

};

#endif //_SmartPtr_header_included_