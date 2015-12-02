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
#include <memory>
namespace fb{
	template <class T>
	class CowPtr{
	public:
		typedef std::shared_ptr<T> TargetPtr;	

		CowPtr(){}
		/// Do not pass a pointer managed by other auto ptr functions like the one 
		/// you get from std::shared_ptr<T>::get().
		explicit CowPtr(T* other) 
			: mP(other)
		{
		}

		CowPtr(const typename CowPtr<T>::TargetPtr& other) 
			: mP(other)
		{
		}		

		CowPtr<T>& operator=(const typename CowPtr<T>& other){
			if (other.mP != mP){
				mP = other.mP;
			}
			return *this;
		}

		CowPtr<T>& operator=(T* other){
			if (mP.get() != other){
				mP = TargetPtr(other);
			}
			return *this;
		}

		bool operator == (const CowPtr<T>& other) const{
			return mP.get() == other.mP.get();
		}

		bool operator !=(const CowPtr<T>& other) const{
			return mP.get != other.mP.get();
		}

		bool operator!() const{
			return !mP.get();
		}

		T& operator*(){
			detach();
			return *mP.get();
		}

		const T& operator*() const{
			return *mP.get();
		}

		T* operator->(){
			detach();
			return mP.get();
		}

		const T* operator->() const{
			return mP.get();
		}

		operator T*(){
			detach();
			return mP.get();
		}

		operator const T*() const{
			return mP.get();
		}

		T* get(){
			detach();
			return mP.get();
		}

		const T* get() const{
			return mP.get();
		}

		// preferred to use this one to the above.
		const T* const_get() const{
			return mP.get();
		}

		TargetPtr data() const{
			return mP;
		}

		void reset(){
			mP.reset();
		}		

	private:
		void detach(){
			T* temp = mP.get();
			if (temp && !mP.unique()){
				mP = TargetPtr(new T(*temp));
			}
		}

	private:
		TargetPtr mP;
	};
}