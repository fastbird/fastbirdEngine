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
#include "FBMathLib/Color.h"
#include "FBMathLib/Vec2I.h"
#include "FBMathLib/Vec3.h"
namespace fb{
	class IScene;
	struct RenderParamOut;
	struct RenderParam;
	FB_DECLARE_SMART_PTR(GeometryRenderer);
	class GeometryRenderer{
		FB_DECLARE_PIMPL_NON_COPYABLE(GeometryRenderer);
		GeometryRenderer();

	public:
		static GeometryRendererPtr Create();
		~GeometryRenderer();

		void SetRenderTargetSize(const Vec2I& size);
		void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut);
		// if wolrdspace is false, it's in the screenspace 0~width, 0~height
		void DrawLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1);
		void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1);
		void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
			const char* texture, bool textureFlow);
		void DrawSphere(const Vec3& pos, Real radius, const Color& color);
		void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha);
		void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha);

		void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut);		
	};
}