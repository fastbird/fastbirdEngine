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
#include "Color.h"
namespace fb{

	FB_DECLARE_SMART_PTR(ColorRamp);
	class ColorRamp
	{
		FB_DECLARE_PIMPL(ColorRamp);		

	public:

		ColorRamp();
		ColorRamp(const ColorRamp& other);
		~ColorRamp();
		ColorRamp& operator=(const ColorRamp& other);
		bool operator==(const ColorRamp& other) const;
		const Color& operator[] (int idx) const;

		struct Bar
		{
			float position;
			Color color;

			Bar(float pos, const Color& color_)
				:position(pos), color(color_){}
			bool operator < (const Bar& other) const {
				return position < other.position;
			}
			bool operator== (const Bar& other) const {
				return (position == other.position && color == other.color);
			}
		};

		void InsertBar(float pos, const Color& color);
		void Clear();
		unsigned NumBars() const;
		Bar& GetBar(int idx);
		void GenerateColorRampTextureData(int textureWidth, float noiseStrength);
		void UpperAlign(float gap = 0.01f);
		void LowerAlign(float gap = 0.01f);
		void Align(float gap = 0.01f);
	};
}