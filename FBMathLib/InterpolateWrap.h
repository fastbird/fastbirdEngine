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
#include "FBMathLib/Length.h"
#include <unordered_map>
namespace fb {	
	template <class T>
	class InterpolateWrap {		
		T mDestValue;		
		T* mTargetVariable;
		T mDeltaPerSec;
		std::function<void(T&)> mWhenDone;
		bool mReached;

	public:
		InterpolateWrap(T* targetVariable, const T& destValue, Real reachAfterSec)
			: mTargetVariable(targetVariable)
			, mReached(false)
		{
			SetDestValue(destValue, reachAfterSec);
		}

		bool Update(Real dt) {
			if (!mReached) {
				auto delta = mDestValue - *mTargetVariable;
				auto stepDelta = mDeltaPerSec * dt;
				if (Length(delta) <= Length(stepDelta)) {
					*mTargetVariable += delta;
					mReached = true;
				}
				else {
					*mTargetVariable += stepDelta;
				}
			}
			if (mReached && mWhenDone) {
				mWhenDone(*mTargetVariable);
			}
			return mReached;
		}

		void SetDestValue(const T& v, Real reachAfterSec) {
			mDestValue = v;
			if (*mTargetVariable != v) {				
				auto delta = mDestValue - *mTargetVariable;
				mDeltaPerSec = delta / reachAfterSec;
				mReached = false;
			}
			else {
				mReached = true;
			}
		}
		const T& GetDestValue() const {
			return mDestValue;
		}

		void SetCallback(std::function<void(T&)> whenDone) {
			mWhenDone = whenDone;
		}
	};

	template <class T>
	class InterpolateWrapManager {
		std::unordered_map< T*, std::shared_ptr<InterpolateWrap<T>> > mIs;

	public:
		// You don't need to hold the pointer.
		std::shared_ptr<InterpolateWrap<T>> RegisterInterpolator(T* targetVariable, const T& destValue, Real reachAfterSec) {
			auto p = mIs[targetVariable];
			if (!p) {
				p = std::make_shared<InterpolateWrap<T>>(targetVariable, destValue, reachAfterSec);
				mIs[targetVariable] = p;
			}
			else {
				p->SetDestValue(destValue, reachAfterSec);
			}
			return p;
		}

		// You don't need to hold the pointer.
		std::shared_ptr<InterpolateWrap<T>> RegisterInterpolator(T* targetVariable, const T& destValue, Real reachAfterSec, std::function<void(T&)> whenDone) {
			auto p = RegisterInterpolator(targetVariable, destValue, reachAfterSec);
			if (p) {
				p->SetCallback(whenDone);
			}
			return p;
		}

		void UnregisterInterpolator(T* targetVariable) {
			auto it = mIs.find(targetVariable);
			if (it != mIs.end()) {
				mIs.erase(it);
			}
		}

		T const GetDestValue(T* targetVariable) {
			auto it = mIs.find(targetVariable);
			if (it == mIs.end())
				return *targetVariable;
			return it->second->GetDestValue();
		}

		void Update(Real dt) {
			for (auto it = mIs.begin(); it != mIs.end(); /**/) {
				auto curIt = it++;
				auto reached = curIt->second->Update(dt);
				if (reached) {
					mIs.erase(curIt);
				}
			}
		}

		bool IsEmpty() const {
			return mIs.empty();
		}

	};
}
