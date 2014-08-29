#pragma once
#include <UI/IWinBase.h>
#include <UI/EventHandler.h>

namespace fastbird
{
	class Container;
	class IUIAnimation;
	class WinBase : public IWinBase, public EventHandler
	{
	public:
		WinBase();
		virtual ~WinBase();

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
		{
			return 0;
		}
		virtual void RemoveChild(IWinBase* child) {}
		virtual IWinBase* GetChild(const char* name) { return 0; }
		virtual void SetName(const char* name);		
		virtual const char* GetName() const;
		virtual void SetSize(const fastbird::Vec2I& size);
		virtual void SetNSize(const fastbird::Vec2& size); // normalized size (0.0~1.0)
		virtual void SetWNSize(const fastbird::Vec2& size);
		virtual void SetPos(const fastbird::Vec2I& pos);
		virtual void SetNPos(const fastbird::Vec2& pos); // normalized pos (0.0~1.0)
		virtual void SetWNPos(const fastbird::Vec2& wnPos);
		void SetAspectRatio(float ratio) { mAspectRatio = ratio; }
		virtual const Vec2& GetWNPos() const { return mWNPos;}
		virtual const Vec2& GetNPos() const { return mNPos; }
		virtual const Vec2& GetWNSize() const { return mWNSize;}
		virtual const Vec2& GetNSize() const { return mNSize; }
		virtual void SetVisible(bool show);
		virtual bool GetVisible() const;
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v);
		virtual void OnStartUpdate(float elapsedTime);
		virtual bool WinBase::IsIn(const Vec2& mouseNormpos);
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(Vec2 normalizedMousePos);
		virtual void OnFocusLost(){}
		virtual void OnFocusGain(){}
		virtual void SetTextColor(const Color& c);
		virtual void SetText(const wchar_t* szText);
		virtual const wchar_t* GetText() const;
		virtual void SetPasswd(bool passwd) {};
		virtual IUIAnimation* GetUIAnimation();

		virtual IEventHandler* GetEventHandler() const { return (IEventHandler*)this; }

		virtual bool SetProperty(UIProperty::Enum, const char*);

		virtual void Scrolled(){}
		virtual void SetNPosOffset(const Vec2& offset);
		virtual void SetAnimNPosOffset(const Vec2& offset);
		virtual void SetScissorRect(bool use, const RECT& rect);
		virtual const RECT& GetRegion() const;
		// OWN
		// local space.
		fastbird::Vec2 ConvertToAlignedPos(const fastbird::Vec2& beforeAlign) const;
		fastbird::Vec2I ConvertToScreen(const fastbird::Vec2 npos) const;
		fastbird::Vec2 ConvertToNormalized(const fastbird::Vec2I pos) const; // convert to 0~1
		

		void SetParent(Container* parent);

		virtual void SetNext(IWinBase* next);
		virtual void SetPrev(IWinBase* prev);
		virtual IWinBase* GetNext() const;
		virtual IWinBase* GetPrev() const;

		static void InitMouseCursor();
		static HCURSOR GetMouseCursorOver();
		static void FinalizeMouseCursor();

		void PosAnimationTo(const Vec2& destNPos, float speed);

		virtual void SetAlphaBlending(bool set);
		virtual float PixelToLocalNWidth(int pixel) const;
		virtual float PixelToLocalNHeight(int pixel) const;
		virtual Vec2 PixelToLocalNSize(const Vec2I& pixel) const;

		virtual float PixelToLocalNPosX(int pixel) const;
		virtual float PixelToLocalNPosY(int pixel) const;
		virtual Vec2 PixelToLocalNPos(const Vec2I& pixel) const;

		virtual float GetTextWidthLocal() const;
		virtual float GetTextEndWLocal() const;

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual float GetTextBottomGap() const;

	protected:
		virtual void OnPosChanged();
		virtual void OnSizeChanged();
		virtual void OnChildPosSizeChanged(WinBase* child){}
		virtual void AlignText();		
		
	private:
		void UpdateAlignedPos();
		void UpdateWorldSize();
		friend class Container;
		void UpdateWorldPos();
		

	protected:
		static const float WinBase::LEFT_GAP;

		bool mVisible;
		std::string mName;
		ALIGNH::Enum mAlignH;
		ALIGNV::Enum mAlignV;
		// local
		Vec2I mSize;
		Vec2 mNSize; //0.0f~1.0f
		Vec2I mPos;
		Vec2 mNPos; // normalized pos 0.f ~ 1.f
		float mAspectRatio;
		
		// aligned local
		Vec2 mNPosAligned; // normalized pos 0.f ~ 1.f
		
		// world
		Vec2 mWNSize; // worldSize aligned.
		Vec2 mWNPos; // worldPos;
		Vec2 mWNPosOffset;
		Vec2 mWNAnimPosOffset;
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
		ALIGNV::Enum mTextAlignV;
		bool mMatchSize; // match text and window size

		IUIObject* mUIObject;

		static HCURSOR mCursorOver;
		bool mMouseDragStartInHere;

		Vec2 mDestNPos;
		float mAnimationSpeed;
		bool mAnimationEnabled;

		IUIAnimation* mAnimation;
	};
}