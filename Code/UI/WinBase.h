#pragma once
#include <UI/IWinBase.h>
#include <UI/EventHandler.h>

namespace fastbird
{
	class Container;
	class WinBase : public IWinBase, public EventHandler
	{
	public:
		WinBase();
		virtual ~WinBase(){}

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
		{
			return 0;
		}
		virtual void RemoveChild(IWinBase* child) {}
		virtual void SetName(const char* name);		
		virtual void SetSize(const fastbird::Vec2I& size);
		virtual void SetNSize(const fastbird::Vec2& size); // normalized size (0.0~1.0)
		virtual void SetWNSize(const fastbird::Vec2& size);
		virtual void SetPos(const fastbird::Vec2I& pos);
		virtual void SetNPos(const fastbird::Vec2& pos); // normalized pos (0.0~1.0)
		virtual void SetWNPos(const fastbird::Vec2& wnPos);
		virtual const Vec2& GetWNPos() const { return mWNPos;}
		virtual const Vec2& GetWNSize() const { return mWNSize;}
		virtual void SetVisible(bool show);
		virtual bool GetVisible() const;
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v);
		virtual void OnStartUpdate() {}
		virtual void OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(Vec2 normalizedMousePos);
		virtual void OnFocusLost(){}
		virtual void OnFocusGain(){}
		virtual void SetTextColor(const Color& c);
		virtual void SetText(const wchar_t* szText);
		virtual const wchar_t* GetText() const;
		virtual void SetPasswd(bool passwd) {};

		virtual IEventHandler* GetEventHandler() const { return (IEventHandler*)this; }

		virtual bool SetProperty(Property prop, const char*);

		virtual void Scrolled(){}
		virtual void SetNPosOffset(const Vec2& offset);
		virtual void SetScissorRect(bool use, const RECT& rect);
		virtual const RECT& GetRegion() const;

		// OWN
		fastbird::Vec2 ConvertToAlignedPos(const fastbird::Vec2& beforeAlign) const;
		fastbird::Vec2I ConvertToScreen(const fastbird::Vec2 npos) const;
		fastbird::Vec2 ConvertToNormalized(const fastbird::Vec2I pos) const;
		void SetParent(Container* parent);

		virtual void SetNext(IWinBase* next);
		virtual void SetPrev(IWinBase* prev);
		virtual IWinBase* GetNext() const;
		virtual IWinBase* GetPrev() const;

		static void InitMouseCursor();
		static HCURSOR GetMouseCursorOver();
		static void FinalizeMouseCursor();

	protected:
		virtual void OnPosChanged();
		virtual void OnSizeChanged();
		virtual void AlignText();		
		
	private:
		void UpdateAlignedPos();
		void UpdateWorldSize();
		friend class Container;
		void UpdateWorldPos();
		

	protected:
		static const float WinBase::LEFT_GAP;
		static const float WinBase::BOTTOM_GAP;

		bool mVisible;
		std::string mName;
		ALIGNH::Enum mAlignH;
		ALIGNV::Enum mAlignV;
		// local
		Vec2I mSize;
		Vec2 mNSize; //0.0f~1.0f
		Vec2I mPos;
		Vec2 mNPos; // normalized pos 0.f ~ 1.f
		
		// aligned local
		Vec2 mNPosAligned; // normalized pos 0.f ~ 1.f
		
		// world
		Vec2 mWNSize; // worldSize aligned.
		Vec2 mWNPos; // worldPos;
		Vec2 mWNPosOffset;
		Container* mParent;

		bool mMouseIn;
		bool mMouseInPrev;

		IWinBase* mPrev;
		IWinBase* mNext;

		std::wstring mTextw;
		Color mTextColor;
		Color mTextColorHover;
		Color mTextColorDown;
		bool mFixedTextSize;
		float mTextSize;

		ALIGNH::Enum mTextAlignH;
		bool mMatchSize; // match text and window size

		SmartPtr<IUIObject> mUIObject;

		static HCURSOR mCursorOver;

	};
}