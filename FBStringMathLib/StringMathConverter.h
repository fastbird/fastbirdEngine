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
#include "FBCommonHeaders/String.h"
#include "FBMathLib/Vec2.h"
#include "FBMathLib/Vec2I.h"
#include "FBMathLib/Vec3.h"
#include "FBMathLib/Vec3I.h"
#include "FBMathLib/Vec4.h"
#include "FBMathLib/Mat33.h"
#include "FBMathLib/Mat44.h"
#include "FBMathLib/Quat.h"
#include "FBMathLib/Rect.h"
#include "FBMathLib/Color.h"
namespace fb{
	class StringMathConverter{
	public:
		static std::string ToString(const Vec2& val);
		static std::string ToString(const Vec2I& val);
		static std::string ToString(const Vec3& val);
		static std::string ToString(const Vec3I& val);
		static std::string ToString(const Vec4& val);
		static std::string ToString(const Vec4& val, int w, int precision);
		static std::string ToString(const Mat33& val);
		static std::string ToString(const Mat44& val);
		static std::string ToString(const Quat& val);
		static std::string ToString(const Rect& val);
		static std::string ToString(const Color& val);

		static Vec2 ParseVec2(const std::string& str, const Vec2& def = Vec2::ZERO);
		static Vec2I ParseVec2I(const std::string& str, const Vec2I& def = Vec2I::ZERO);
		static Vec3 ParseVec3(const std::string& str, const Vec3& def = Vec3::ZERO);
		static Vec3I ParseVec3I(const std::string& str, const Vec3I& def = Vec3I::ZERO);
		static Vec4 ParseVec4(const std::string& str, const Vec4& def = Vec4::ZERO);
		static Mat33 ParseMat33(const std::string& str, const Mat33& def = Mat33::IDENTITY);
		static Mat44 ParseMat44(const std::string& str, const Mat44& def = Mat44::IDENTITY);
		static Quat ParseQuat(const std::string& str, const Quat& def = Quat::IDENTITY);
		static Rect ParseRect(const std::string& str, const Rect& def = { 0, 0, 0, 0 });
		static Color ParseColor(const std::string& str, const Color& def = Color(0.f, 0.f, 0.f));
	};
}
