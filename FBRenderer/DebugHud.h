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
#include "FBMathLib/Math.h"
#include <queue>
#include <list>
#include <map>
#undef DrawText
namespace fb
{
	struct RenderParam;
	struct RenderParamOut;
	FB_DECLARE_SMART_PTR(MeshObject);
	FB_DECLARE_SMART_PTR(Shader);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(RenderStates);
	FB_DECLARE_SMART_PTR(VertexBuffer);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(DebugHud);
	class DebugHud
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(DebugHud);
		DebugHud();

	public:
		static DebugHudPtr Create();
		~DebugHud();
		void SetRenderTargetSize(const Vec2I& size);		
		void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut);
		/// for rendering mWorldLinesBeforeAlphaPass
		void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut);

		//--------------------------------------------------------------------
		// Own
		//--------------------------------------------------------------------
		void DrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text, 
			const Color& color, Real size);
		void ClearDurationTexts();
		void DrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size);
		void Draw3DText(const Vec3& pos, WCHAR* text, const Color& color, Real size);
		void Draw3DTextNow(const Vec3& pos, WCHAR* text, const Color& color, Real size);
		// if wolrdspace is false, it's in the screenspace 0~width, 0~height
		void DrawLine(const Vec3& start, const Vec3& end, const Color& color0,
			const Color& color1);
		void DrawLineNow(const Vec3& start, const Vec3& end, const Color& color0,
			const Color& color1);
		void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
			const Color& color1);
		void DrawLine(const Vec2I& start, const Vec2I& end, const Color& color0, 
			const Color& color1);
		void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color);

		void DrawSphere(const Vec3& pos, Real radius, const Color& color);
		void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha);
		void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha);
		void DrawPointNow(const Vec3& pos, Real radius, const Color& color);
		void DrawPointsNow(const Vec3::Array& pos, const Color& color);
	};

}