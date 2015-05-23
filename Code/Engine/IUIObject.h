#pragma once

#include <Engine/Object.h>

namespace fastbird
{
	class IWinBase;
	class IUIObject : public Object
	{
	public:
		static IUIObject* CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize);
		void Delete();

		IUIObject(): mTypeString(0) {}
		virtual ~IUIObject(){}

		virtual void SetTexCoord(Vec2 coord[], DWORD num, unsigned index=0) = 0;
		virtual void SetColors(DWORD colors[], DWORD num) = 0;

		virtual void SetUIPos(const Vec2I& pos) = 0;
		virtual const Vec2I& GetUIPos() const = 0;

		virtual void SetUISize(const Vec2I& size) = 0;
		virtual const Vec2I& GetUISize() const = 0;
		virtual void SetAlpha(float alpha) = 0;
		
		virtual void SetText(const wchar_t* s) = 0;
		virtual void SetTextOffset(const Vec2I& offset) = 0;
		virtual const Vec2I& GetTextOffset() const = 0;
		virtual Vec2I GetTextStartWPos() const = 0;
		virtual void SetTextColor(const Color& c) = 0;
		virtual void SetTextSize(float size) = 0;
		virtual const RECT& GetRegion() const = 0;
		virtual void SetDebugString(const char* string) = 0;
		virtual void SetNoDrawBackground(bool flag) = 0;
		virtual bool GetNoDrawBackground() const = 0;
		virtual void SetUseScissor(bool use, const RECT& rect) = 0;

		virtual void SetSpecialOrder(int order) = 0;
		virtual int GetSpecialOrder() const = 0;

		virtual void SetMultiline(bool multiline) = 0;
		//virtual void SetDebugNumber(unsigned num) { mDebugNumber = num; }
		virtual void SetDoNotDraw(bool doNotDraw) = 0;

		virtual void SetRenderTargetSize(const Vec2I& rtSize) = 0;
		virtual const Vec2I& GetRenderTargetSize() const = 0;

	public:
		// debug purpose
		IWinBase* mOwnerUI;
		const char* mTypeString;
		//unsigned mDebugNumber;
	};
}