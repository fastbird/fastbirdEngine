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
	FB_DECLARE_SMART_PTR(HorizontalGauge);
	class FB_DLL_UI HorizontalGauge : public Container
	{
	protected:
		HorizontalGauge();
		~HorizontalGauge();

	public:
		
		static HorizontalGaugePtr Create();

		// IWinBase
		ComponentType::Enum GetType() const { return ComponentType::HorizontalGauge; }
		virtual void GatherVisit(std::vector<UIObject*>& v);
		virtual void OnStartUpdate(float elapsedTime);

		virtual void SetPercentage(float p);
		virtual void SetMaximum(float m);		

		virtual void SetGaugeColor(const Color& color);
		virtual void SetGaugeColorEmpty(const Color& color);

		void SetBlinkColor(const Color& color);		
		void Blink(bool blink);
		void Blink(bool blink, float time);
		float GetPercentage() const { return mPercentage; }		
		float GetMaximum() const { return mMaximum; }		
		const Color& GetGaugeColor() const;		
		const Color& GetBlinkColor() const;		

		virtual bool SetProperty(UIProperty::Enum prop, const char* val) OVERRIDE;
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly) OVERRIDE;

	protected:		
		void SetTextureAtlasRegion(UIProperty::Enum prop, const char* region);
		void ChangeMaterial(const char* materialPath);
		bool UsingEmptyColor();

		float mPercentage;
		float mMaximum;
		Color mGaugeColor;
		Color mGaugeColorEmpty;
		Color mGaugeBorderColor;
		bool mVertical;
		bool mHorizontalFlip;
		bool mMaterialUsingImage;
		
		std::string mTextureAtlasFile;
		TextureAtlasPtr mTextureAtlas;
		TextureAtlasRegionPtr mAtlasRegions[2];
		TexturePtr mTextures[2];

		std::string mRegionFilled;
		std::string mRegionNotFilled;
		Color mBlinkColor;
		bool mBlink;
		float mBlinkSpeed;
		float mBlinkTime;


	};
}
