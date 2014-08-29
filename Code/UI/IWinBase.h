#pragma once

#include <UI/ComponentType.h>
#include <UI/Align.h>
#include <UI/UIProperty.h>
#include <Engine/IUIObject.h>

namespace fastbird
{
	class IMouse;
	class IKeyboard;
	class IEventHandler;
	class IUIAnimation;

	class CLASS_DECLSPEC_UI IWinBase
	{
	public:
		IWinBase() {}
		virtual ~IWinBase() {}

		virtual ComponentType::Enum GetType() const = 0;
		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) = 0;
		virtual void RemoveChild(IWinBase* child) = 0;
		virtual IWinBase* GetChild(const char* name) = 0;
		virtual void SetSize(const fastbird::Vec2I& size) = 0;
		virtual void SetNSize(const fastbird::Vec2& size) = 0; // normalized size (0.0~1.0)
		virtual void SetWNSize(const fastbird::Vec2& size) = 0;
		virtual void SetPos(const fastbird::Vec2I& pos) = 0;
		virtual void SetWNPos(const fastbird::Vec2& wnPos) = 0;
		virtual void SetNPos(const fastbird::Vec2& pos) = 0; // normalized pos (0.0~1.0)
		virtual const Vec2& GetNPos() const = 0;
		virtual const Vec2& GetWNPos() const = 0;
		virtual const Vec2& GetWNSize() const = 0;
		virtual const Vec2& GetNSize() const = 0;
		virtual void SetName(const char* name) = 0;		
		virtual const char* GetName() const = 0;
		virtual void SetVisible(bool show) = 0;
		virtual bool GetVisible() const = 0;
		virtual bool GetFocus(bool includeChildren = false) const = 0;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v) = 0;		
		virtual void GatherVisit(std::vector<IUIObject*>& v) = 0;

		virtual void OnStartUpdate(float elapsedTime) = 0;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard) = 0;
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

		virtual bool SetProperty(UIProperty::Enum, const char*) = 0;

		virtual void Scrolled() = 0;
		virtual void SetNPosOffset(const Vec2& offset) = 0;
		virtual void SetAnimNPosOffset(const Vec2& offset) = 0;
		virtual void SetScissorRect(bool use, const RECT& rect) = 0;
		virtual const RECT& GetRegion() const = 0;

		virtual float PixelToLocalNWidth(int pixel) const = 0;
		virtual float PixelToLocalNHeight(int pixel) const = 0;
		virtual Vec2 PixelToLocalNSize(const Vec2I& pixel) const = 0;
		virtual float PixelToLocalNPosX(int pixel) const = 0;
		virtual float PixelToLocalNPosY(int pixel) const = 0;
		virtual Vec2 PixelToLocalNPos(const Vec2I& pixel) const = 0;

		virtual IUIAnimation* GetUIAnimation()= 0;

		virtual bool ParseXML(tinyxml2::XMLElement* pelem) = 0;
		virtual float GetTextBottomGap() const = 0;

		// You usually controll the ui-layer by changing the order of Adding new UI. Later added ui draw first.
		// But if you use alpha blending, you need to use these enumeration by calling IWinBase::SetLayer()
		/*
		enum Layer
		{
			LAYER_UP_5,
			LAYER_UP_4,
			LAYER_UP_3,
			LAYER_UP_2,
			LAYER_UP_1,

			LAYER_DEFAULT,

			LAYER_DOWN_1,
			LAYER_DOWN_2,
			LAYER_DOWN_3,
			LAYER_DOWN_4,
			LAYER_DOWN_5,
		};
		*/
		virtual void SetAlphaBlending(bool set) = 0;

		virtual float GetTextWidthLocal() const = 0;
		virtual float GetTextEndWLocal() const = 0;

	protected:
		virtual void OnPosChanged() = 0;
		virtual void OnSizeChanged() = 0;
	};
}