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

#include "Container.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(Button);
	FB_DECLARE_SMART_PTR(NumericUpDown);
	class FB_DLL_UI NumericUpDown : public Container
	{
	protected:
		NumericUpDown();
		~NumericUpDown();

	public:
		static NumericUpDownPtr Create();
		
		// IWinBase
		void GatherVisit(std::vector<UIObject*>& v);
		void OnCreated();
		ComponentType::Enum GetType() const { return ComponentType::NumericUpDown; }
		bool SetProperty(UIProperty::Enum prop, const char* val);
		bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

		void SetNumber(int number);
		void SetMinMax(int min, int max);
		int GetValue() const { return mValue; }

		void SetEnableUp(bool enable);
		void SetEnableDown(bool enable);
		void SetEnable(bool enable);


	protected:
		void InitializeButtons();

		void OnDown(void* arg);
		void OnUp(void* arg);
		

	private:
		ButtonWeakPtr mDown;
		ButtonWeakPtr mUp;
		int mValue;
		int mMin;
		int mMax;
		int mShiftStep;
		int mStep;
	};

}