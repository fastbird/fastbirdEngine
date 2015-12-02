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
#include <map>
#include <vector>
#include <iostream>
#include <memory>
#include "Types.h"
#include "Helpers.h"
namespace fb{
	template <typename Observer>
	class Observable{
	public:
		typedef int ObserverEventType;
		typedef std::shared_ptr<Observer> ObserverPtr;
		typedef std::weak_ptr<Observer> ObserverWeakPtr;
	protected:
		std::map< ObserverEventType, std::vector<ObserverWeakPtr> > mObservers_;

	public:

		void AddObserver(ObserverEventType type, ObserverPtr observer){
			if (!observer){				
				return;
			}
			auto& observers = mObservers_[type];
			if (!ValueExistsInVector(observers, observer)){
				observers.push_back(observer);
				OnObserverAdded(observer);
			}
		}

		bool RemoveObserver(ObserverEventType type, ObserverPtr observer){
			auto& observers = mObservers_[type];
			DeleteValuesInVector(observers, observer);
			return observersDeletedAny;
		}

		virtual void OnObserverAdded(ObserverPtr observer) {}
	};	
}