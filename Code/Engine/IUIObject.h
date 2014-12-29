#pragma once

#include <Engine/Foundation/Object.h>

namespace fastbird
{
	class IWinBase;
	class CLASS_DECLSPEC_ENGINE IUIObject : public Object
	{
	public:
		static IUIObject* CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize);
		void Delete();

		IUIObject(): mTypeString(0) {}
		virtual ~IUIObject(){}

		// to set vertices
		virtual void SetVertices(const Vec3* ndcPoints, int num,
			const DWORD* colors = 0, const Vec2* texcoords = 0) = 0;
		// or call
		virtual void SetTexCoord(Vec2 coord[], DWORD num) = 0;
		virtual void SetColors(DWORD colors[], DWORD num) = 0;

		virtual void SetNSize(const Vec2& size) = 0; // in normalized space 0.0f~1.0f
		virtual const Vec2& GetNSize() const = 0;
		virtual void SetNPos(const Vec2& npos) = 0; // in normalized space 0.0f~1.0f
		virtual const Vec2& GetNPos() const = 0;
		virtual void SetNPosOffset(const Vec2& nposOffset) = 0; // in normalized space 0.0f~1.0f
		virtual void SetAnimNPosOffset(const Vec2& nposOffset) = 0;// in normalized space 0.0f~1.0f
		virtual const Vec2& GetAnimNPosOffset() const = 0;
		virtual void SetAnimScale(const Vec2& scale, const Vec2& pivot) = 0;
		virtual void SetPivot(const Vec2& pivot) = 0;
		virtual void SetAlpha(float alpha) = 0;
		virtual void SetText(const wchar_t* s) = 0;
		virtual void SetTextStartNPos(const Vec2& npos) = 0;
		virtual const Vec2& GetTextStarNPos() const = 0;
		virtual void SetTextColor(const Color& c) = 0;
		virtual void SetTextSize(float size) = 0;
		virtual const RECT& GetRegion() const = 0;
		virtual void SetDebugString(const char* string) = 0;
		virtual void SetNoDrawBackground(bool flag) = 0;
		virtual bool GetNoDrawBackground() const = 0;
		virtual void SetUseScissor(bool use, const RECT& rect) = 0;
		virtual void SetAlphaBlending(bool set) = 0;
		virtual bool GetAlphaBlending() const = 0;

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