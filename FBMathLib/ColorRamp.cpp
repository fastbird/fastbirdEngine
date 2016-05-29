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

#include "stdafx.h"
#include "ColorRamp.h"
#include "NoiseGen.h"
using namespace fb;
class ColorRamp::Impl{
public:
	typedef std::vector<Bar> BAR_VECTOR;
	BAR_VECTOR mBars;
	std::vector<Color> mColors;
	bool mGenerated;

	Impl(){

	}

	const Color& operator[] (int idx) const{
		assert(idx < (int)mColors.size());
		return mColors[idx];
	}

	bool operator==(const ColorRamp& other) const
	{
		if (mBars.size() != other.mImpl->mBars.size())
			return false;

		size_t numBars = mBars.size();
		for (size_t i = 0; i<numBars; i++)
		{
			if (!(mBars[i] == other.mImpl->mBars[i]))
				return false;
		}


		return true;
	}

	void InsertBar(float pos, const Color& color){
		mBars.push_back(Bar(pos, color));
		std::sort(mBars.begin(), mBars.end());
	}

	void Clear() {
		mBars.clear();
	}

	unsigned NumBars() const { 
		return mBars.size(); 
	}

	Bar& GetBar(int idx) { 
		assert((unsigned)idx < mBars.size()); return mBars[idx]; 
	}

	void GenerateColorRampTextureData(int textureWidth, float noiseStrength){
		if (mBars.empty())
			return;

		mColors.resize(textureWidth);
		if (mBars.size() == 1) {
			for (int i = 0; i < textureWidth; ++i) {
				mColors[i] = mBars[0].color;
			}
			return;
		}		
		NoiseGen noise1(1);
		NoiseGen noise2(2);
		NoiseGen noise3(3);
		for (auto barIt = mBars.begin(); barIt != mBars.end(); ++barIt) {
			float leftPos = barIt->position;
			Color leftColor = barIt->color;
			float rightPos = leftPos;
			Color rightColor = leftColor;
			if (barIt + 1 != mBars.end()) {
				rightPos = (barIt + 1)->position;
				rightColor = (barIt + 1)->color;
			}

			if(barIt == mBars.begin()) {				
				auto leftLength = barIt->position;
				auto oppositeLength = 1.0f - mBars.back().position;
				leftColor = Lerp(mBars.back().color, barIt->color, oppositeLength / (leftLength + oppositeLength));
			}
			else if (barIt == mBars.end() - 1) {				
				rightPos = 1.f;
				auto rightLength = 1.0f - barIt->position;
				auto oppositeLength = mBars.begin()->position;
				rightColor = Lerp(barIt->color, mBars.begin()->color, rightLength / (rightLength + oppositeLength));
			}

			
			
			auto startIndex = Round(leftPos * (textureWidth-1));
			auto endIndex = Round(rightPos*(textureWidth-1));
			float numIndices = float(endIndex - startIndex);
			float curIndexCount = 0;
			
			for (int pixelIndex = startIndex; pixelIndex <= endIndex; ++pixelIndex, curIndexCount+=1.f) {
				Real u = pixelIndex / (Real)textureWidth;
				Color noiseColor(noise1.Get(u), noise2.Get(u), noise3.Get(u));
				noiseColor *= noiseStrength;
				mColors[pixelIndex] = Saturate(Lerp(leftColor+ noiseColor, rightColor+ noiseColor, curIndexCount / numIndices));
			}
		}
	}

	

	void UpperAlign(float gap = 0.01f)
	{
		for (size_t i = 1; i < mBars.size(); ++i)
		{
			if (mBars[i].position - mBars[i - 1].position< gap)
			{
				mBars[i].position = mBars[i - 1].position + gap;
				mBars[i].position = std::min(mBars[i].position, 1.0f);
			}
		}
	}
	void LowerAlign(float gap = 0.01f)
	{
		for (int i = (int)mBars.size() - 2; i >= 0; --i)
		{
			if (mBars[i + 1].position - mBars[i].position < gap)
			{
				mBars[i].position = mBars[i + 1].position - gap;
				mBars[i].position = std::max(0.f, mBars[i].position);
			}
		}
	}
};

//---------------------------------------------------------------------------
ColorRamp::ColorRamp()
: mImpl(new Impl)
{
}

ColorRamp::ColorRamp(const ColorRamp& other)
	: mImpl(new Impl(*other.mImpl.get()))
{
}

ColorRamp::~ColorRamp(){

}

ColorRamp& ColorRamp::operator = (const ColorRamp& other){
	*mImpl = *other.mImpl;
	return *this;
}
bool ColorRamp::operator==(const ColorRamp& other) const{
	return mImpl->operator==(other);
}

const Color& ColorRamp::operator[] (int idx) const{
	return mImpl->operator[](idx);
}

void ColorRamp::InsertBar(float pos, const Color& color){
	return mImpl->InsertBar(pos, color);
}

void ColorRamp::Clear() {
	return mImpl->Clear();
}

unsigned ColorRamp::NumBars() const{
	return mImpl->NumBars();
}

ColorRamp::Bar& ColorRamp::GetBar(int idx){
	return mImpl->GetBar(idx);
}

void ColorRamp::GenerateColorRampTextureData(int textureWidth, float noiseScale){
	mImpl->GenerateColorRampTextureData(textureWidth, noiseScale);
}

void ColorRamp::UpperAlign(float gap){
	mImpl->UpperAlign(gap);
}

void ColorRamp::LowerAlign(float gap){
	mImpl->LowerAlign(gap);
}

void ColorRamp::Align(float gap) {
	mImpl->LowerAlign(gap);
	mImpl->UpperAlign(gap);
}
