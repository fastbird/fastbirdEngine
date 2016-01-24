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
#include "FBCommonHeaders/Types.h"
#include "FBMathLib/Math.h"
namespace fb
{
	class Color
	{
	public:
		const static Color White;
		const static Color Black;
		const static Color Red;
		const static Color BrightRed;
		const static Color DarkGray;
		const static Color Gray;
		const static Color Silver;
		const static Color Green;
		const static Color Yellow;
		const static Color Blue;
		const static Color SkyBlue;
		const static Color Zero;

		struct RGBA
		{
			BYTE r;
			BYTE g;
			BYTE b;
			BYTE a;
		};
		static unsigned FixColorByteOrder(unsigned c);
		static unsigned ReplaceAlpha(unsigned c, float alpha);

		static Color RandomColor();

		Color();
		Color(const Vec3& color);
		Color(const Vec4& color);
		Color(Real r, Real g, Real b, Real a);
		Color(Real r, Real g, Real b);
		Color(LPCTSTR str);
		explicit Color(unsigned int c);
		explicit Color(const Vec4Tuple& t);

		// when you want to send the data to gpu
		unsigned int Get4Byte() const;
		// when you want to create hexa string
		unsigned int Get4ByteReversed() const;
		//operator unsigned int() const;
		const Vec4& GetVec4() const;
		Color operator* (Real scalar) const;
		Color operator*(const Color& other) const;
		Color& operator*= (Real scalar);
		Color& operator*= (const Color& c);
		Color operator+ (const Color& r) const;
		bool operator== (const Color& other) const;
		bool operator!= (const Color& other) const;
		operator Vec4Tuple() const;
		void SetColor(Real r, Real g, Real b, Real a = 1.f);
		Real r() const;
		Real g() const;
		Real b() const;
		Real a() const;

		Real& r();
		Real& g();
		Real& b();
		Real& a();

	private:
		Vec4 mValue;
	};

	Color Random(const Color& min, const Color& max);
}