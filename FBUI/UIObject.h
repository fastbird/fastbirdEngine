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
	class WinBase;
	FB_DECLARE_SMART_PTR(UIObject);
	class UIObject
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(UIObject);
		UIObject();
		~UIObject();

	public:
		static UIObjectPtr Create(const Vec2I& renderTargetSize, WinBase* uicomponent);

		// debug data.
		WinBase* mOwnerUI;
		std::string mTypeString;

		//-------------------------------------------------------------------------
		void SetTexCoord(Vec2 coord[], DWORD num, unsigned index=0);
		void ClearTexCoord(unsigned index = 0);
		void SetColors(DWORD colors[], DWORD num);

		void SetUIPos(const Vec2I& pos);
		const Vec2I& GetUIPos() const;

		void SetUISize(const Vec2I& size);
		const Vec2I& GetUISize() const;
		
		void SetAlpha(float alpha);

		void SetText(const wchar_t* s);
		void SetTextOffset(const Vec2I& offset);
		const Vec2I& GetTextOffset() const;
		void SetTextOffsetForCursorMovement(const Vec2I& offset);
		Vec2I GetTextStartWPos() const;

		void SetTextColor(const Color& c);
		void SetTextSize(float size);
		const Rect& GetRegion() const;
		void SetDebugString(const char* string);
		void SetNoDrawBackground(bool flag);		
		virtual bool GetNoDrawBackground() const;
		void SetUseScissor(bool use, const Rect& rect);
		void SetSpecialOrder(int order);
		int GetSpecialOrder() const;

		void SetMultiline(bool multiline);
		void SetDoNotDraw(bool doNotDraw);
		void SetRenderTargetSize(const Vec2I& rtSize);
		const Vec2I& GetRenderTargetSize() const;
		bool HasTexCoord() const;
		bool HasTexCoord(int index) const;
		void EnableLinearSampler(bool linear);

		void PreRender();
		void Render();		

		void SetMaterial(const char* name);
		MaterialPtr GetMaterial() const;

		
		void UpdateRegion();

		void SetSeperatedBackground(bool seperated);
		void SetUseSeperatedUVForAlpha(bool seperatedUV);

		void SetUIComponent(WinBase* comp);
		WinBase* GetUIComponent() const;

		void SetRenderSimpleBorder(bool simpleBorder);
	};
}