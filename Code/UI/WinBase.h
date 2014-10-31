#pragma once
#include <UI/IWinBase.h>
#include <UI/EventHandler.h>

namespace fastbird
{
	class Container;
	class IUIAnimation;
	class ImageBox;
	class WinBase : public IWinBase, public EventHandler
	{
	public:
		WinBase();
		virtual ~WinBase();

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
		{
			return 0;
		}
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type)
		{
			return 0;
		}
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable)
		{
			return 0;
		}
		virtual void RemoveChild(IWinBase* child, bool immediately = false) {}
		virtual void RemoveAllChild(bool immediately = false) {}
		virtual IWinBase* GetChild(const char* name, bool includeSubChildren = false) { return 0; }
		virtual IWinBase* GetChild(unsigned idx) { return 0; }
		virtual unsigned GetNumChildren() const { return 0; }
		virtual void RemoveAllEvents(bool includeChildren);
		virtual void SetName(const char* name);		
		virtual const char* GetName() const;
		virtual void SetSize(const fastbird::Vec2I& size);
		virtual void SetSizeX(int x);
		virtual void SetSizeY(int y);
		virtual void SetPos(const fastbird::Vec2I& pos);
		virtual void SetPosX(int x);
		virtual void SetPosY(int y);
		virtual void SetInitialOffset(Vec2I offset);

		virtual void SetNSize(const fastbird::Vec2& size); // normalized size (0.0~1.0)
		virtual void SetNSizeX(float x);
		virtual void SetNSizeY(float y);

		virtual void SetWNSize(const fastbird::Vec2& size);		
		virtual void SetNPos(const fastbird::Vec2& pos); // normalized pos (0.0~1.0)
		virtual void SetNPosX(float x);
		virtual void SetNPosY(float y);
		virtual void SetWNPos(const fastbird::Vec2& wnPos);
		void SetAspectRatio(float ratio) { mAspectRatio = ratio; }
		virtual void SetSizeModificator(const Vec2I& sizemod);

		virtual const Vec2& GetWNPos() const { return mWNPos;}
		virtual const Vec2& GetNPos() const { return mNPos; }
		virtual const Vec2I& GetPos() const { return mPos; }
		virtual const Vec2& GetWNSize() const { return mWNSize;}
		virtual const Vec2& GetNSize() const { return mNSize; }
		virtual const Vec2I& GetSize() const { return mSize; }
		// coordinates are decided by functions like SetNPos():for relative or SetPos() for absolute.
		virtual void SetUseAbsXPos(bool use) { mUseAbsoluteXPos = use; }
		virtual void SetUseAbsYPos(bool use) { mUseAbsoluteYPos = use; }
		virtual bool GetUseAbsXPos() const { return mUseAbsoluteXPos; }
		virtual bool GetUseAbsYPos() const { return mUseAbsoluteYPos; }
		virtual void SetUseAbsXSize(bool use) { mUseAbsoluteXSize = use; }
		virtual void SetUseAbsYSize(bool use) { mUseAbsoluteYSize = use; }
		virtual bool GetUseAbsXSize() const { return mUseAbsoluteXSize;  }
		virtual bool GetUseAbsYSize() const { return mUseAbsoluteXSize; }

		virtual void SetVisible(bool show);
		virtual void OnParentVisibleChanged(bool visible) {}
		virtual bool GetVisible() const;
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v);
		virtual void OnStartUpdate(float elapsedTime);
		virtual bool WinBase::IsIn(IMouse* mouse);
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(IMouse* mouse);
		virtual void OnFocusLost(){}
		virtual void OnFocusGain(){}
		virtual void SetTextColor(const Color& c);
		virtual void SetText(const wchar_t* szText);
		virtual const wchar_t* GetText() const;
		virtual void SetPasswd(bool passwd) {};
		virtual IUIAnimation* GetUIAnimation(const char* name);

		virtual IEventHandler* GetEventHandler() const { return (IEventHandler*)this; }

		virtual bool SetProperty(UIProperty::Enum prop, const char* val);

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
		// manually controlling objects are allowed to have parent which is not a container.
		// only for scissor culling.
		void SetManualParent(WinBase* parent);

		virtual void SetNext(IWinBase* next);
		virtual void SetPrev(IWinBase* prev);
		virtual IWinBase* GetNext() const;
		virtual IWinBase* GetPrev() const;

		static void InitMouseCursor();
		static HCURSOR GetMouseCursorOver();
		static void FinalizeMouseCursor();

		void PosAnimationTo(const Vec2& destNPos, float speed);

		virtual void SetAlphaBlending(bool set);
		/*virtual bool GetAlphaBlending() const;*/
		virtual float PixelToLocalNWidth(int pixel) const;
		virtual float PixelToLocalNHeight(int pixel) const;
		virtual Vec2 PixelToLocalNSize(const Vec2I& pixel) const;

		virtual int LocalNWidthToPixel(float nwidth) const;
		virtual int LocalNHeightToPixel(float nheight) const;
		virtual Vec2I LocalNSizeToPixel(const Vec2& nsize) const;

		virtual float PixelToLocalNPosX(int pixel) const;
		virtual float PixelToLocalNPosY(int pixel) const;
		virtual Vec2 PixelToLocalNPos(const Vec2I& pixel) const;
		
		virtual int LocalNPosXToPixel(float nposx) const;
		virtual int LocalNPosYToPixel(float nposy) const;
		virtual Vec2I LocalNPosToPixel(const Vec2& npos) const;

		virtual float GetTextWidthLocal() const;
		virtual float GetTextEndWLocal() const;

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual bool ParseLua(const fastbird::LuaObject& compTable);
		virtual float GetTextBottomGap() const;

		virtual void RefreshScissorRects();

		virtual void SetEnable(bool enable);
		virtual bool GetEnable(bool enable) const;

		virtual bool HasUIObject() const { return mUIObject != 0; }

		virtual void SetContent(void* p) {
			mCustomContent = p;
		}
		virtual void* GetContent() const { return mCustomContent; }

		void UpdateAlignedPos();
		void UpdateWorldSize(bool settingSize = false);
		void UpdateWorldPos(bool settingPos = false);

		virtual int GetSpecialOrder() const { return mSpecialOrder; }
		virtual bool GetInheritVisibleTrue() const { return mInheritVisibleTrue; }

	protected:
		virtual void OnPosChanged();
		virtual void OnSizeChanged();
		virtual void AlignText();
		const RECT& GetScissorRegion();
		void SetUseBorder(bool use);
		void RefreshBorder();
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void CalcTextWidth(); // virtual for mutiline text
		
	private:
		friend class Container;
		void ToolTipEvent(IEventHandler::EVENT evt, const Vec2& mouseNPos);
		

	protected:
		static const float WinBase::LEFT_GAP;

		bool mVisible;
		std::string mName;
		ALIGNH::Enum mAlignH;
		ALIGNV::Enum mAlignV;
		// local
		Vec2I mSize;
		Vec2 mNSize; //0.0f~1.0f
		Vec2I mSizeMod;
		Vec2I mPos;
		Vec2 mNPos; // normalized pos 0.f ~ 1.f
		float mAspectRatio;
		bool mUseAbsoluteXPos;
		bool mUseAbsoluteYPos;

		bool mUseAbsoluteXSize;
		bool mUseAbsoluteYSize;
		bool mAbsTempLock;
		
		// aligned local
		Vec2 mNPosAligned; // normalized pos 0.f ~ 1.f
		
		// world
		Vec2 mWNSize; // worldSize aligned.
		Vec2 mWNPos; // worldPos;
		Vec2 mWNPosOffset; // scrollbar offset
		Vec2 mWNAnimPosOffset; // animation offset
		Container* mParent;
		WinBase* mManualParent;

		Vec2I mAbsOffset; // when loading;


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
		// this is not related to mAnimation(IUIAnimation)
		bool mSimplePosAnimEnabled; 

		VectorMap<std::string, IUIAnimation*> mAnimations;

		std::wstring mTooltipText;
		bool mNoMouseEvent;

		bool mUseScissor;
		bool mEnable;

		std::vector<ImageBox*> mBorders;

		void* mCustomContent;
		bool mLockTextSizeChange; // for changing the ui size by text size (MATCH_SIZE Property)
		
		bool mStopScissorParent;
		int mSpecialOrder;
		unsigned mTextWidth;
		unsigned mNumTextLines;
		bool mInheritVisibleTrue;

	};
}