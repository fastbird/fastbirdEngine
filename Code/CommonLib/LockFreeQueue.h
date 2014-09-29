#pragma once
#include <CommonLib/AsyncObjects.h>
namespace fastbird
{
	//
	// A fixed-size, lockfree queue.
	//
	//---------------------------------------------------------------------------

	template<class type>
	class LockFreeQueue
	{
	public:
		LockFreeQueue()
			: mSize(0)
			, mBuffer(0)
			, mReadIndex(0)
			, mWriteIndex(0)
			, mInitialized(false)
		{}

		~LockFreeQueue()
		{
			if (mBuffer)
			{
				free(mBuffer);
			}
		}

		void Init(int _Size)
		{
			assert(!mInitialized);
			mSize = fastbird::GetNextPowerOfTwo(_Size);
			size_t bufferSize = sizeof(type*) * mSize;
			mBuffer = (type**)malloc(bufferSize);
			assert(mBuffer);
			memset(mBuffer, 0, bufferSize);
			mInitialized = true;
		}

		bool IsInitialized() const { return mInitialized; }
		bool HasElement() const { return mReadIndex != mWriteIndex; }
		bool Enqueue(type* Elem)
		{
			LOCK_CRITICAL_SECTION lock(mLock);
			assert(Elem);
			if (mBuffer[mWriteIndex] != 0)
			{
				assert(0);
				return false;
			}
				

			mBuffer[mWriteIndex] = Elem;
			++mWriteIndex;
			if (mWriteIndex >= mSize)
			{
				mWriteIndex = 0;
			}
				

			if (mBuffer[mWriteIndex] != 0)
			{
				Error("LockFreeQueue is too small");
			}
			
			return true;
		}

		type* Dequeue()
		{
			LOCK_CRITICAL_SECTION lock(mLock);
			type* p = mBuffer[mReadIndex];
			if (p)
			{
				mBuffer[mReadIndex] = 0;
				++mReadIndex;
				if (mReadIndex >= mSize)
				{
					mReadIndex = 0;
				}

				return p;
			}
			else
			{
				return NULL;
			}	
		}

		type* SeeCur()
		{
			LOCK_CRITICAL_SECTION lock(mLock);
			return mBuffer[mReadIndex];
		}

	private:

		int mSize;
		type** mBuffer;
		long mReadIndex;
		long mWriteIndex;
		FB_CRITICAL_SECTION mLock;
		bool mInitialized;
	};


	//template<class type>
	//class LockFreeQueue
	//{
	//public:
	//	LockFreeQueue()
	//		: mSize(0)
	//		, mBuffer(NULL)
	//		, mReadIndex(0)
	//		, mWriteIndex(0)
	//	{}

	//	~LockFreeQueue()
	//	{
	//		if (mRealBuffer)
	//		{
	//			free((void*)mRealBuffer);
	//		}
	//	}

	//	struct LFQNode
	//	{
	//		struct ElemKey
	//		{
	//			type* Elem;
	//			DWORD Key;
	//		};

	//		union Data
	//		{
	//			ElemKey mElemKey;
	//			long long mValue;
	//		};

	//		LFQNode()
	//		{}

	//		LFQNode(type* _Elem, DWORD _Key)
	//		{
	//			mData.mElemKey.Elem = _Elem;
	//			mData.mElemKey.Key = _Key;
	//		}

	//		Data mData;
	//	};

	//	void Init(int _Size)
	//	{
	//		mSize = fastbird::GetNextPowerOfTwo(_Size);
	//		mRealBuffer = (LFQNode*)malloc(sizeof(LFQNode)* mSize + 128);
	//		assert(mRealBuffer);
	//		// for memory alignment
	//		mBuffer = (LFQNode*)(((DWORD)mRealBuffer + 127) & ~127);
	//		assert(!((DWORD)mBuffer & 127));

	//		for (int i = 0; i < mSize; i++)
	//		{
	//			mBuffer[i].mData.mElemKey.Elem = NULL;
	//			mBuffer[i].mData.mElemKey.Key = i;
	//		}
	//	}

	//	bool Enqueue(type* Elem)
	//	{
	//		LOCK_CRITICAL_SECTION lock(mLock);
	//		assert(Elem);
	//		LFQNode CurNode(NULL, 0);
	//		LFQNode NewNode(Elem, 0);
	//		while ((mWriteIndex - mReadIndex) < mSize)
	//		{
	//			const int CurWriteIndex = mWriteIndex;
	//			const int WriteBucket = (CurWriteIndex & (mSize - 1));

	//			CurNode.mData.mElemKey.Key = CurWriteIndex;
	//			NewNode.mData.mElemKey.Key = CurWriteIndex;

	//			if (InterlockedCompareExchange64(&mBuffer[WriteBucket].mData.mValue,
	//				NewNode.mData.mValue, CurNode.mData.mValue) == CurNode.mData.mValue)
	//			{
	//				++mWriteIndex;
	//				return true;
	//			}
	//		}
	//		//Log("ReadIdx = %u, writeIdx = %u", mReadIndex, mWriteIndex);
	//		Error("LockFreeQueue enqueue() failed!");
	//		assert(0);
	//		return false;
	//	}

	//	type* Dequeue()
	//	{
	//		LOCK_CRITICAL_SECTION lock(mLock);
	//		while (mReadIndex != mWriteIndex)
	//		{
	//			const int CurReadIndex = mReadIndex;
	//			const int ReadBucket = (CurReadIndex & (mSize - 1));
	//			const LFQNode CurNode((type*)mBuffer[ReadBucket].mData.mElemKey.Elem, CurReadIndex);

	//			if (CurNode.mData.mElemKey.Elem)
	//			{
	//				// make the key unique
	//				const LFQNode NewNode(NULL, CurReadIndex + mSize);

	//				if (InterlockedCompareExchange64(&mBuffer[ReadBucket].mData.mValue,
	//					NewNode.mData.mValue, CurNode.mData.mValue) == CurNode.mData.mValue)
	//				{
	//					++mReadIndex;
	//					return (type*)CurNode.mData.mElemKey.Elem;
	//				}
	//			}
	//			else
	//			{
	//				assert(0);
	//			}
	//		}

	//		return NULL;
	//	}

	//private:

	//	int mSize;
	//	LFQNode* mRealBuffer;
	//	LFQNode* mBuffer;
	//	std::atomic<long> mReadIndex;
	//	std::atomic<long> mWriteIndex;
	//	FB_CRITICAL_SECTION mLock;
	//};

}