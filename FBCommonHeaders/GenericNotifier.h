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
#include "FBCommonHeaders/Helpers.h"
#define GenericListenerSub(T) GenericListener<GenericNotifier<##T> >
template <class T>
class GenericListener {
public:
	virtual ~GenericListener() {
		std::vector<T*> v;
		v.swap(mNotifiers);
		for (auto n : v) {
			n->RemoveListenerFromListener((T::TypeRawPtr)this);
		}
	}

	virtual void AddNotifier(T* n) {
		if (!ValueExistsInVector(mNotifiers, n)) {
			mNotifiers.push_back(n);
		}
	}

	virtual void RemoveNotifier(T* n) {
		DeleteValuesInVector(mNotifiers, n);
		if (mNotifiersDeletedAny) {
			n->RemoveListenerFromListener((T::TypeRawPtr)this);
		}
	}

protected:
	std::vector<T*> mNotifiers;
};

template <class T>
class GenericNotifier
{
public:
	typedef T* TypeRawPtr;

	virtual ~GenericNotifier() {
		std::vector<TypeRawPtr> clone;
		clone.swap(mListeners);
		for (auto l : clone) {
			l->RemoveNotifier(this);
		}
	}
	virtual void AddListener(TypeRawPtr l)
	{
		if (!ValueExistsInVector(mListeners, l)) {
			mListeners.push_back(l);
			l->AddNotifier(this);
		}
	}
	
	virtual void OnListenerAdded(TypeRawPtr l) {

	}

	virtual void RemoveListener(TypeRawPtr l)
	{
		DeleteValuesInVector(mListeners, l);
		if (mListenersDeletedAny) {
			l->RemoveNotifier(this);
		}
	}

	virtual void RemoveListenerFromListener(TypeRawPtr l)
	{
		DeleteValuesInVector(mListeners, l);
	}

	virtual bool HasListener() const {
		return !mListeners.empty();
	}


protected:
	std::vector<TypeRawPtr> mListeners;
};