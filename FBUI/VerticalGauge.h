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
FB_DECLARE_SMART_PTR(VerticalGauge);
class FB_DLL_UI VerticalGauge : public WinBase
{
protected:
	VerticalGauge();
	~VerticalGauge();

public:
	static VerticalGaugePtr Create();	

	// IWinBase
	ComponentType::Enum GetType() const { return ComponentType::VerticalGauge; }
	void GatherVisit(std::vector<UIObject*>& v);
	void OnStartUpdate(float elapsedTime);

	void SetPercentage(float p);
	void SetMaximum(float m);
	void Blink(bool blink);
	void SetGaugeColor(const Color& color);
	void SetBlinkColor(const Color& color);

	void SetTextureAtlasRegion(UIProperty::Enum, const char* region);

	bool SetProperty(UIProperty::Enum prop, const char* val);
	bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

private:
	float mPercentage;
	float mMaximum;
	Color mGaugeColor;
	Color mBlinkColor;
	bool mBlink;
	float mBlinkSpeed;

	std::string mTextureAtlasFile;
	TextureAtlasPtr mTextureAtlas;
	TextureAtlasRegionPtr mAtlasRegions[2];
	TexturePtr mTextures[2];

	bool mMaterialUsingImage;
	bool mHorizontalFlip;

	std::string mRegionFilled;
	std::string mRegionNotFilled;
	std::string mStrGaugeBorderColor;

};
}
