#pragma once
#include "FBCommonHeaders/Helpers.h"
#define GenericListenerSub(T) GenericListener<GenericNotifier<##T> >
template <class T>
class GenericListener {
public:
	virtual ~GenericListener() {
		for (auto n : mNotifiers) {
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