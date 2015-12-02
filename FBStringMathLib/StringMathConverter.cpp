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

#include "StringMathConverter.h"
#include "FBStringLib/StringLib.h"
#include "FBStringLib/StringConverter.h"
#include <sstream>
#include <iomanip>
#include <assert.h>

namespace fb{

std::string StringMathConverter::ToString(const Vec2& val){
	std::stringstream stream;
	stream << val.x << " " << val.y;
	return stream.str();
}
std::string StringMathConverter::ToString(const Vec2I& val){
	std::stringstream stream;
	stream << val.x << " " << val.y;
	return stream.str();
}
std::string StringMathConverter::ToString(const Vec3& val){
	std::stringstream stream;
	stream << val.x << " " << val.y << " " << val.z;
	return stream.str();
}
std::string StringMathConverter::ToString(const Vec3I& val){
	std::stringstream stream;
	stream << val.x << " " << val.y << " " << val.z;
	return stream.str();
}
std::string StringMathConverter::ToString(const Vec4& val){
	std::stringstream stream;
	stream << val.x << " " << val.y << " " << val.z << " " << val.w;
	return stream.str();
}
std::string StringMathConverter::ToString(const Vec4& val, int w, int precision){
	std::stringstream stream;
	stream << std::setw(w);
	stream.precision(precision);
	stream << std::fixed;
	stream << val.x << " " << val.y << " " << val.z << " " << val.w;
	return stream.str();
}
std::string StringMathConverter::ToString(const Mat33& val){
	std::stringstream stream;
	stream << val[0][0] << " "
		<< val[0][1] << " "
		<< val[0][2] << " "
		<< val[1][0] << " "
		<< val[1][1] << " "
		<< val[1][2] << " "
		<< val[2][0] << " "
		<< val[2][1] << " "
		<< val[2][2];
	return stream.str();
}
std::string StringMathConverter::ToString(const Mat44& val){
	std::stringstream stream;
	stream << val[0][0] << " "
		<< val[0][1] << " "
		<< val[0][2] << " "
		<< val[0][3] << " "
		<< val[1][0] << " "
		<< val[1][1] << " "
		<< val[1][2] << " "
		<< val[1][3] << " "
		<< val[2][0] << " "
		<< val[2][1] << " "
		<< val[2][2] << " "
		<< val[2][3] << " "
		<< val[3][0] << " "
		<< val[3][1] << " "
		<< val[3][2] << " "
		<< val[3][3];
	return stream.str();
}
std::string StringMathConverter::ToString(const Quat& val){
	std::stringstream stream;
	stream << val.w << " " << val.x << " " << val.y << " " << val.z;
	return stream.str();
}
std::string StringMathConverter::ToString(const Rect& val){
	std::stringstream stream;
	stream << val.left << " " << val.top << " " << val.right << " " << val.bottom;
	return stream.str();
}

std::string StringMathConverter::ToString(const Color& val){
	std::stringstream stream;
	stream << val.r() << " " << val.g() << " " << val.b() << " " << val.a();
	return stream.str();
}

Vec2 StringMathConverter::ParseVec2(const std::string& str, const Vec2& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 2)
	{
		return def;
	}
	else
	{
		return Vec2(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]));
	}
}
Vec2I StringMathConverter::ParseVec2I(const std::string& str, const Vec2I& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 2)
	{
		return def;
	}
	else
	{
		return Vec2I(StringConverter::ParseInt(vec[0]), StringConverter::ParseInt(vec[1]));
	}
}
Vec3 StringMathConverter::ParseVec3(const std::string& str, const Vec3& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 3)
	{
		return def;
	}
	else
	{
		return Vec3(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), 
			StringConverter::ParseReal(vec[2]));
	}
}
Vec3I StringMathConverter::ParseVec3I(const std::string& str, const Vec3I& def){
	StringVector vec = Split(str);

	if (vec.size() < 3)
	{
		return def;
	}
	else
	{
		return Vec3I(StringConverter::ParseInt(vec[0]), StringConverter::ParseInt(vec[1]),
			StringConverter::ParseInt(vec[2]));
	}
}
Vec4 StringMathConverter::ParseVec4(const std::string& str, const Vec4& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 4)
	{
		return def;
	}
	else
	{
		return Vec4(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), 
			StringConverter::ParseReal(vec[2]), StringConverter::ParseReal(vec[3]));
	}
}
Mat33 StringMathConverter::ParseMat33(const std::string& str, const Mat33& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 9)
	{
		return def;
	}
	else
	{
		return Mat33(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), StringConverter::ParseReal(vec[2]),
			StringConverter::ParseReal(vec[3]), StringConverter::ParseReal(vec[4]), StringConverter::ParseReal(vec[5]),
			StringConverter::ParseReal(vec[6]), StringConverter::ParseReal(vec[7]), StringConverter::ParseReal(vec[8]));
	}
}
Mat44 StringMathConverter::ParseMat44(const std::string& str, const Mat44& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 16)
	{
		return def;
	}
	else
	{
		return Mat44(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), StringConverter::ParseReal(vec[2]), StringConverter::ParseReal(vec[3]),
			StringConverter::ParseReal(vec[4]), StringConverter::ParseReal(vec[5]), StringConverter::ParseReal(vec[6]), StringConverter::ParseReal(vec[7]),
			StringConverter::ParseReal(vec[8]), StringConverter::ParseReal(vec[9]), StringConverter::ParseReal(vec[10]), StringConverter::ParseReal(vec[11]),
			StringConverter::ParseReal(vec[12]), StringConverter::ParseReal(vec[13]), StringConverter::ParseReal(vec[14]), StringConverter::ParseReal(vec[15]));
	}
}
Quat StringMathConverter::ParseQuat(const std::string& str, const Quat& def){
	// Split on space
	StringVector vec = Split(str);

	if (vec.size() < 4)
	{
		return def;
	}
	else
	{
		return Quat(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), StringConverter::ParseReal(vec[2]), StringConverter::ParseReal(vec[3]));
	}
}
Rect StringMathConverter::ParseRect(const std::string& str, const Rect& def){
	StringVector vec = Split(str);
	Rect value = { def.left, def.top, def.right, def.bottom };
	if (vec.size() == 4)
	{
		value.left = StringConverter::ParseLong(vec[0]);
		value.top = StringConverter::ParseLong(vec[1]);
		value.right = StringConverter::ParseLong(vec[2]);
		value.bottom = StringConverter::ParseLong(vec[3]);
	}
	else
	{
		assert(0);
	}

	return value;
}

Color StringMathConverter::ParseColor(const std::string& str, const Color& def){
	StringVector vec = Split(str);

	if (vec.size() < 4)
	{
		return def;
	}
	else
	{
		return Color(StringConverter::ParseReal(vec[0]), StringConverter::ParseReal(vec[1]), StringConverter::ParseReal(vec[2]), StringConverter::ParseReal(vec[3]));
	}
}

}