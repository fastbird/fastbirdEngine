#pragma once

#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <Engine/IUIObject.h>

namespace fastbird
{
	class IMouse;
	class IKeyboard;
	class IEventHandler;

	class CLASS_DECLSPEC_UI IWinBase
	{
	public:
		IWinBase() {}

		enum Property
		{
			PROPERTY_BACK_COLOR,		// vec4
			PROPERTY_BACK_COLOR_OVER,	// vec4
			PROPERTY_TEXT_ALIGN,		// left, center, right
			PROPERTY_TEXT_SIZE,			// be sure set fixed text size also if you need.
			PROPERTY_TEXT_COLOR,		// vec4
			PROPERTY_TEXT_COLOR_HOVER,	// vec4
			PROPERTY_TEXT_COLOR_DOWN,	// vec4
			PROPERTY_FIXED_TEXT_SIZE,	
			PROPERTY_MATCH_SIZE,		// true, false
			PROPERTY_NO_BACKGROUND,		// true, false
		};

		
		virtual ~IWinBase() {}
		virtual ComponentType::Enum GetType() const = 0;
		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual void RemoveChild(IWinBase* child) = 0;
		virtual void SetSize(const fastbird::Vec2I& size) = 0;
		virtual void SetNSize(const fastbird::Vec2& size) = 0; // normalized size (0.0~1.0)
		virtual void SetWNSize(const fastbird::Vec2& size) = 0;
		virtual void SetPos(const fastbird::Vec2I& pos) = 0;
		virtual void SetWNPos(const fastbird::Vec2& wnPos) = 0;
		virtual void SetNPos(const fastbird::Vec2& pos) = 0; // normalized pos (0.0~1.0)
		virtual const Vec2& GetWNPos() const = 0;
		virtual const Vec2& GetWNSize() const = 0;
		virtual void SetName(const char* name) = 0;		
		virtual void SetVisible(bool show) = 0;
		virtual bool GetVisible() const = 0;
		virtual bool GetFocus(bool includeChildren = false) const = 0;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v) = 0;		
		virtual void GatherVisit(std::vector<IUIObject*>& v) = 0;

		virtual void OnStartUpdate() = 0;
		virtual void OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard) = 0;
		virtual IWinBase* FocusTest(Vec2 normalizedMousePos) = 0;
		virtual void OnFocusLost() = 0;
		virtual void OnFocusGain() = 0;

		virtual void SetTextColor(const Color& c) = 0;
		virtual void SetText(const wchar_t* szText) = 0;
		virtual const wchar_t* GetText() const = 0;
		virtual void SetPasswd(bool passwd) = 0;

		virtual void SetNext(IWinBase* next) = 0;
		virtual void SetPrev(IWinBase* prev) = 0;
		virtual IWinBase* GetNext() const = 0;
		virtual IWinBase* GetPrev() const = 0;

		virtual IEventHandler* GetEventHandler() const = 0;

		virtual bool SetProperty(Property prop, const char*) = 0;

		virtual void Scrolled() = 0;
		virtual void SetNPosOffset(const Vec2& offset) = 0;
		virtual void SetScissorRect(bool use, const RECT& rect) = 0;
		virtual const RECT& GetRegion() const = 0;

	protected:
		virtual void OnPosChanged() = 0;
		virtual void OnSizeChanged() = 0;
	};
}