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

namespace fb
{
	class Vec3;
	class Color;
	class IDebugDrawer
	{
	public:
		virtual void DrawLine(const Vec3& from, const Vec3& to, const Color& color) = 0;
		virtual void DrawLine(const Vec3& from, const Vec3& to, const Color& fromColor, const Color& toColor) = 0;
		virtual void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
			const char* texture, bool textureFlow) = 0;

		virtual void	DrawSphere(const Vec3& p, float radius, const Color& color) = 0;
		virtual void	DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha) = 0;
		virtual void	DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha) = 0;
		virtual void  Draw3DText(const Vec3& location, const char* text) = 0;
	};
}