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
#include "WinBase.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(HorizontalGauge);
	class FB_DLL_UI HorizontalGauge : public WinBase
	{
	protected:
		HorizontalGauge();
		~HorizontalGauge();

	public:
		
		static HorizontalGaugePtr Create();

		// IWinBase
		ComponentType::Enum GetType() const { return ComponentType::HorizontalGauge; }
		void GatherVisit(std::vector<UIObject*>& v);
		void OnStartUpdate(float elapsedTime);

		void SetPercentage(float p);
		float GetPercentage() const { return mPercentage; }
		void SetMaximum(float m);
		float GetMaximum() const { return mMaximum; }
		void Blink(bool blink);
		void Blink(bool blink, float time);
		void SetGaugeColor(const Color& color);
		const Color& GetGaugeColor() const;
		void SetGaugeColorEmpty(const Color& color);
		void SetBlinkColor(const Color& color);
		const Color& GetBlinkColor() const;

		bool SetProperty(UIProperty::Enum prop, const char* val);
		bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

	private:
		float mPercentage;
		float mMaximum;
		Color mGaugeColor;
		Color mGaugeColorEmpty;
		Color mGaugeBorderColor;
		bool mGaugeColorEmptySet;
		Color mBlinkColor;
		bool mBlink;
		float mBlinkSpeed;
		float mBlinkTime;


	};
}
