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
#include "Color.h"
#include "FBStringLib/StringLib.h"
#include "FBStringMathLib/StringMathConverter.h"
#include "FBStringLib/StringConverter.h"

namespace fb
{
const Color Color::White(1.f, 1.f, 1.f);
const Color Color::Black(0, 0, 0);
const Color Color::Red(1, 0, 0);
const Color Color::BrightRed(1.f, 0.2f, 0.2f);
const Color Color::DarkGray(0.15f, 0.15f, 0.15f);
const Color Color::Gray(0.5f, 0.5f, 0.5f);
const Color Color::Silver(0.8f, 0.8f, 0.8f);
const Color Color::Green(0, 1, 0);
const Color Color::Yellow(1, 1, 0);
const Color Color::Blue(0, 0, 1);
const Color Color::SkyBlue(0, 0.5f, 1);
const Color Color::Zero(0, 0, 0, 0);

Color::Color(){
}

Color::Color(const Vec3& color)
	: mValue(color, 1.0f)
{
}

Color::Color(const Vec4& color)
	: mValue(color) 
{
}

Color::Color(Real r, Real g, Real b, Real a)
	: mValue(r, g, b, a) 
{
}

Color::Color(Real r, Real g, Real b)
	: mValue(r, g, b, 1.f)
{
}

static unsigned int ParseHexa(const TString& val, unsigned int defaultValue = 0){
	std::stringstream str(val);
	unsigned int ret = defaultValue;
	str >> std::hex >> ret;
	return ret;
}

Color::Color(const char* str)
{
	if (str[0] == '0' && str[1] == 'x')
	{
		if (strlen(str) == 8)
		{
			TString strColor = str;
			strColor += "ff";
			*this = Color(Color::FixColorByteOrder(ParseHexa(strColor.c_str())));
		}
		else
		{
			*this = Color(Color::FixColorByteOrder(ParseHexa(str)));
		}
	}
	else
	{
		mValue = Vec4(str);
	}
}

Color::Color(unsigned int c)
{
	RGBA* rgba = (RGBA*)&c;
	mValue.x = rgba->r / 255.0f;
	mValue.y = rgba->g / 255.0f;
	mValue.z = rgba->b / 255.0f;
	mValue.w = rgba->a / 255.0f;
}

Color::Color(const Vec4Tuple& t)
	: mValue(t)
{
}

// when you want to send the data to gpu
unsigned int Color::Get4Byte() const
{
	RGBA color;
	color.r = BYTE(mValue.x * 255.f);
	color.g = BYTE(mValue.y * 255.f);
	color.b = BYTE(mValue.z * 255.f);
	color.a = BYTE(mValue.w * 255.f);
	return *(unsigned int*)&color;
}

// when you want to create hexa string
unsigned int Color::Get4ByteReversed() const
{
	RGBA color;
	color.r = BYTE(mValue.w * 255.f);
	color.g = BYTE(mValue.z * 255.f);
	color.b = BYTE(mValue.y * 255.f);
	color.a = BYTE(mValue.x * 255.f);
	return *(unsigned int*)&color;
}

//Color::operator unsigned int() const {
//	return Get4Byte(); 
//}

const Vec4& Color::GetVec4() const {
	return mValue; 
}

Color Color::operator* (Real scalar) const{
	return Color(mValue*scalar);
}

Color Color::operator*(const Color& other) const {
	return Color(mValue*other.mValue);
}

Color& Color::operator*= (Real scalar){
	mValue *= scalar;
	return *this;
}

Color& Color::operator*= (const Color& c)
{
	mValue *= c.mValue;
	return *this;
}

Color Color::operator+ (const Color& r) const {
	return mValue + r.GetVec4();
}

bool Color::operator== (const Color& other) const{
	return mValue == other.mValue;
}

bool Color::operator!= (const Color& other) const{
	return mValue != other.mValue;
}

Color::operator Vec4Tuple() const{
	return Vec4Tuple(mValue);
}

void Color::SetColor(Real r, Real g, Real b, Real a){
	mValue.x = r;
	mValue.y = g;
	mValue.z = b;
	mValue.w = a;
}

void Color::SetColorInt(int r, int g, int b, int a)
{
	mValue.x = r / 255.f;
	mValue.y = g / 255.f;
	mValue.z = b / 255.f;
	mValue.w = a / 255.f;
}

Real Color::r() const {
	return mValue.x; 
}

Real Color::g() const {
	return mValue.y; 
}

Real Color::b() const {
	return mValue.z; 
}

Real Color::a() const { 
	return mValue.w; 
}

Real& Color::r(){
	return mValue.x; 
}

Real& Color::g(){
	return mValue.y; 
}

Real& Color::b(){
	return mValue.z; 
}

Real& Color::a(){ 
	return mValue.w; 
}

std::string Color::ToString() const {
	return mValue.ToString();
}

unsigned Color::FixColorByteOrder(unsigned c)
{
	RGBA color;
	color.r = c >> 24;
	color.g = c >> 16 & 0xff;
	color.b = c >> 8 & 0xff;
	color.a = c & 0xff;
	return *(unsigned*)&color;
}

unsigned Color::ReplaceAlpha(unsigned c, float alpha){
	char a = (char)Round(alpha * 255.f);
	return (c & 0xffffff) + (a << 24);
}


Color Color::RandomColor(){
	return Color(Random(), Random(), Random(), 1.f);
}

Color Random(const Color& min, const Color& max)
{
	return Color(Random(min.r(), max.r()), Random(min.g(), max.g()),
		Random(min.b(), max.b()));
}
}