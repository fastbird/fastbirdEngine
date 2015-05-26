#pragma once
#include <UI/IWinBase.h>
#include <UI/EventHandler.h>
#include <UI/VisibleStatus.h>

namespace fastbird
{
	class Container;
	class IUIAnimation;
	class ImageBox;
	class WinBase : public IWinBase, public EventHandler
	{
	protected:
		static const int WinBase::LEFT_GAP;
		static const int WinBase::BOTTOM_GAP;
		static const float NotDefined;
		static Vec2I OSWindowPos;
		static bool sSuppressPropertyWarning;

		static Vec2I sLastPos;
		friend class VisibleStatus;

		HWND_ID mHwndId;
		VisibleStatus mVisibility;
		std::string mName;
		std::string mScriptPath;
		std::string mUIPath;

		ALIGNH::Enum mAlignH;
		ALIGNV::Enum mAlignV;
		// local
		Vec2I mSize;
		Vec2 mAnimScale;
		Vec2I mScaledSize;
		Vec2 mNSize; //0.0f~1.0f
		bool mFillX;
		bool mFillY;
		Vec2I mPos;
		Vec2I mWPos;
		Vec2I mWAlignedPos;
		Vec2I mScrolledPos;
		Vec2I mAnimatedPos;

		Vec2 mAnimPos;			
		Vec2 mNPos;
		Vec2 mNPosAligned;
		Vec2 mWNPos;
		// scrolling offset;
		Vec2 mWNScrollingOffset;

		float mAspectRatio;
		bool mAspectRatioSet;
		bool mUseAbsoluteXPos;
		bool mUseAbsoluteYPos;

		bool mUseAbsoluteXSize;
		bool mUseAbsoluteYSize;

		Container* mParent;
		WinBase* mManualParent;

		Vec2I mAbsOffset; // when loading;
		Vec2 mNOffset;


		bool mMouseIn;
		bool mMouseInPrev;

		IWinBase* mPrev;
		IWinBase* mNext;

		std::wstring mTextw;
		std::string mTextBeforeTranslated;
		Color mTextColor;
		Color mTextColorHover;
		Color mTextColorDown;
		bool mFixedTextSize;
		float mTextSize;

		ALIGNH::Enum mTextAlignH;
		ALIGNV::Enum mTextAlignV;
		bool mMatchSize; // match text and window size

		IUIObject* mUIObject;

		static HCURSOR sCursorOver;
		static HCURSOR sCursorAll;
		static HCURSOR sCursorNWSE;
		static HCURSOR sCursorNESW;
		static HCURSOR sCursorWE;
		static HCURSOR sCursorNS;
		
		bool mMouseDragStartInHere;

		Vec2 mDestNPos;
		float mAnimationSpeed;
		VectorMap<std::string, IUIAnimation*> mAnimations;
		std::wstring mTooltipText;
		std::string mTooltipTextBeforeT;
		// this is not related to mAnimation(IUIAnimation)
		bool mSimplePosAnimEnabled;
		bool mNoMouseEvent;
		bool mNoMouseEventAlone;
		bool mUseScissor;
		bool mEnable;

		std::vector<ImageBox*> mBorders;

		void* mCustomContent;
		int mSpecialOrder;
		unsigned mTextWidth;
		unsigned mNumTextLines;

		static const char* sShowAnim;
		static const char* sHideAnim;
		static const float sFadeInOutTime;
		float mHideCounter;

		bool mLockTextSizeChange; // for changing the ui size by text size (MATCH_SIZE Property)
		bool mStopScissorParent;
		bool mInheritVisibleTrue;
		bool mPivot;
		bool mRender3D;
		bool mModal;
		Vec2I mDragable;
		Vec2I mRenderTargetSize;

		Vec2I mTextGap;

		float mAlpha;
		// for highlight
		float mHighlightSpeed;
		float mCurHighlightTime;
		bool mGoingBright;
		bool mShowingTooltip;
		int mTabOrder;
		bool mSaveCheck;
		bool mRunTimeChild;
		bool mGhost;
		bool mGatheringException;

		VectorMap<IEventHandler::EVENT, std::string> mEventFuncNames;

	public:
		WinBase();
		virtual ~WinBase();

		static void SuppressPropertyWarning(bool warning);

		virtual void SetHwndId(HWND_ID hwndId);
		virtual HWND_ID GetHwndId() const;
		virtual void OnCreated(){}

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) {
			return 0;
		}
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type) {
			return 0;
		}
		virtual IWinBase* AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type){
			return 0;
		}
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable) {
			return 0;
		}
		virtual IWinBase* AddChild(ComponentType::Enum type) {
			return 0;
		}
		virtual void RemoveChild(IWinBase* child, bool immediately = false) {}
		virtual void RemoveAllChild(bool immediately = false) {}
		virtual void RemoveChildNotDelete(IWinBase* child){}
		virtual IWinBase* GetChild(const std::string& name, bool includeSubChildren = false) { return 0; }
		virtual IWinBase* GetChild(unsigned idx) { return 0; }
		virtual IWinBase* GetParent() { return (IWinBase*)mParent; }
		virtual unsigned GetNumChildren(bool excludeRunTimeChild = false) const { return 0; }
		virtual void RemoveAllEvents(bool includeChildren);
		virtual void SetName(const char* name);
		virtual const char* GetName() const;
		virtual void ClearName();

		// Sizing
		virtual void ChangeSize(const Vec2I& size); // in runtime.
		virtual void ChangeSizeX(int sizeX);
		virtual void ChangeSizeY(int sizeY);
		virtual void SetSize(const Vec2I& size); 		
		virtual void SetSizeX(int x);		
		virtual void SetSizeY(int y);

		virtual void ChangeNSize(const Vec2& nsize);
		virtual void ChangeNSizeX(float x);
		virtual void ChangeNSizeY(float y);
		virtual void SetNSize(const Vec2& size); // normalized size (0.0~1.0)
		virtual void SetNSizeX(float x);
		virtual void SetNSizeY(float y);

		virtual void ModifySize(const Vec2I& sizemod);
		virtual void SetWNSize(const fastbird::Vec2& size);
		virtual void OnParentSizeChanged();
		void SetAspectRatio(float ratio) { mAspectRatioSet = true; mAspectRatio = ratio; }
		// called when the parent size has changed.
		virtual void NotifySizeChange() {
			/*nothing to do if this is not a container.*/
		} 		

		// Positioning
		virtual void ChangePos(const Vec2I& pos); // in runtime
		virtual void ChangePosX(int posx);
		virtual void ChangePosY(int posy);
		virtual void ChangeNPos(const Vec2& npos);
		virtual void ChangeNPosX(float xpos);
		virtual void ChangeNPosY(float ypos);
		virtual void ChangeWPos(const Vec2I& wpos);
		virtual void SetPos(const Vec2I& pos);
		virtual void SetPosX(int x);
		virtual void SetPosY(int y);		

		void SetPosWithTranslator(const Vec2I& pos);
		void SetPosWithTranslatorX(int x);
		void SetPosWithTranslatorY(int y);
		
		virtual void SetNPos(const fastbird::Vec2& pos); // normalized pos (0.0~1.0)
		virtual void SetNPosX(float x);
		virtual void SetNPosY(float y);

		virtual void SetInitialOffset(Vec2I offset);
		virtual void Move(Vec2I amount);
		virtual void SetWNPos(const fastbird::Vec2& wnPos);
		virtual void OnParentPosChanged();
		virtual void NotifyPosChange(){
			/*nothing to do if this is not a container.*/
		}
		
		// Accessors
		virtual const Vec2& GetWNPos() const { return mWNPos; }
		virtual const Vec2I& GetWPos() const { return mWPos; }
		virtual const Vec2I& GetFinalPos() const { return mAnimatedPos; }
		virtual const Vec2I& GetFinalSize() const { return mScaledSize; }
		virtual void SetWPos(const Vec2I& wpos);
		virtual const Vec2& GetNPos() const { return mNPos; }
		virtual const Vec2I& GetPos() const { return mPos; }
		//virtual const Vec2& GetWNSize() const { return mWNSize; }
		virtual const Vec2& GetNSize() const { return mNSize; }
		virtual const Vec2I& GetSize() const { return mSize; }
		
		// coordinates are decided by functions like SetNPos():for relative or SetPos() for absolute.
		virtual void SetUseAbsXPos(bool use) { mUseAbsoluteXPos = use; }
		virtual void SetUseAbsYPos(bool use) { mUseAbsoluteYPos = use; }
		virtual bool GetUseAbsXPos() const { return mUseAbsoluteXPos; }
		virtual bool GetUseAbsYPos() const { return mUseAbsoluteYPos; }
		virtual void SetUseAbsXSize(bool use) { mUseAbsoluteXSize = use; }
		virtual void SetUseAbsYSize(bool use) { mUseAbsoluteYSize = use; }
		virtual bool GetUseAbsXSize() const { return mUseAbsoluteXSize; }
		virtual bool GetUseAbsYSize() const { return mUseAbsoluteYSize; }

		virtual bool SetVisible(bool show);
		virtual bool SetVisibleChildren(bool show){ return false; }
		virtual void SetVisibleInternal(bool visible);
		virtual void OnParentVisibleChanged(bool visible) {}
		virtual bool GetVisible() const;
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void SetAlign(ALIGNH::Enum h, ALIGNV::Enum v);
		virtual void OnStartUpdate(float elapsedTime);
		virtual bool IsIn(IMouse* mouse) const;
		virtual bool IsIn(const Vec2I& pt, bool ignoreScissor, Vec2I* expand = 0) const;
		virtual bool IsPtOnLeft(const Vec2I& pt, int area) const;
		virtual bool IsPtOnRight(const Vec2I& pt, int area) const;
		virtual bool IsPtOnTop(const Vec2I& pt, int area) const;
		virtual bool IsPtOnBottom(const Vec2I& pt, int area) const;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual void OnFocusLost(){}
		virtual void OnFocusGain(){}
		std::string TranslateText(const char* text);
		virtual void SetTextColor(const Color& c);
		virtual void SetText(const wchar_t* szText);
		virtual const wchar_t* GetText() const;
		virtual int GetTextWidth() const { return mTextWidth; }
		virtual int GetTextEndPosLocal() const;

		virtual void SetPasswd(bool passwd) {};
		virtual IUIAnimation* GetOrCreateUIAnimation(const char* name);
		virtual IUIAnimation* GetUIAnimation(const char* name);
		virtual void SetUIAnimation(IUIAnimation* anim);
		virtual void ClearAnimationResult();

		virtual IEventHandler* GetEventHandler() const { return (IEventHandler*)this; }

		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);
		virtual bool GetPropertyAsBool(UIProperty::Enum prop, bool defaultVal = false);
		virtual float GetPropertyAsFloat(UIProperty::Enum prop, float defaultVal = 0.f);
		virtual int GetPropertyAsInt(UIProperty::Enum prop, int defaultVal = 0);

		virtual void Scrolled(){}
		virtual void SetWNScollingOffset(const Vec2& offset);
		virtual const Vec2& GetWNScrollingOffset() const { return mWNScrollingOffset; }
		virtual void SetAnimScale(const Vec2& scale);
		virtual void SetAnimPos(const Vec2& pos);
		/*virtual void SetPivotToUIObject(const Vec2& pivot);*/
		//virtual Vec2 GetPivotWNPos();
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

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual void Save(tinyxml2::XMLElement& elem);
		virtual bool ParseLua(const fastbird::LuaObject& compTable);
		virtual float GetTextBottomGap() const;

		virtual void RefreshScissorRects();

		virtual void SetEnable(bool enable);
		virtual bool GetEnable() const;

		virtual bool HasUIObject() const { return mUIObject != 0; }

		virtual void SetContent(void* p) {
			mCustomContent = p;
		}
		virtual void* GetContent() const { return mCustomContent; }

		//void UpdateWorldSize(bool settingSize = false);
		void UpdateWorldPos();
		void UpdateAlignedPos();
		void UpdateScrolledPos();
		void UpdateAnimatedPos();

		virtual int GetSpecialOrder() const { return mSpecialOrder; }
		virtual bool GetInheritVisibleTrue() const { return mInheritVisibleTrue; }
		virtual void SetScriptPath(const char* path) { assert(path);  mScriptPath = path; }
		virtual const char* GetScriptPath() const { return mScriptPath.c_str(); }

		virtual void SetUIFilePath(const char* path) { assert(path); mUIPath = path; }
		virtual const char* GetUIFilePath() const { return mUIPath.c_str(); }
		virtual void SetRender3D(bool render3D, const Vec2I& renderTargetSize);
		virtual bool GetRender3D() const{ return mRender3D; }
		virtual Vec2I GetRenderTargetSize() const;
		virtual Vec2I GetParentSize() const;

		virtual IWinBase* GetRootWnd() const;

		virtual bool IsAlwaysOnTop() const{ return false; }

		virtual IUIObject* GetUIObj() const { return mUIObject; }

		virtual bool GetCloseByEsc() const { return false; }

		virtual void StartHighlight(float speed);
		virtual void StopHighlight();

		virtual bool GetNoBackground() const;
		virtual const Color GetBackColor();

		virtual float GetAlpha() const;
		virtual void OnAlphaChanged();
		virtual const char* GetMsgTranslationUnit() const;
		virtual void TriggerRedraw();
		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const;
		virtual void GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const;
		virtual IWinBase* WinBaseWithTabOrder(unsigned tabOrder) const{
			return 0;
		}
		virtual int GetTabOrder() const { return mTabOrder; }
		virtual void GetBiggestTabOrder(int& curBiggest) const;
		virtual void TabPressed();

		virtual void SetSaveNameCheck(bool set);
		virtual bool GetSaveNameCheck() const;
		virtual void SetRuntimeChild(bool runtime) { mRunTimeChild = runtime; }
		virtual bool IsRuntimeChild() const { return mRunTimeChild; }
		virtual void SetGhost(bool ghost){ mGhost = ghost; }
		virtual bool GetGhost() const{ return mGhost; }

		virtual float GetContentHeight() const;

		virtual bool IsKeyboardFocused() const;

		virtual void SetGatheringException(){ mGatheringException = true; }
		virtual bool GetGatheringException() const{
			return mGatheringException;
		}

	protected:
		virtual void OnPosChanged(bool anim);
		virtual void OnSizeChanged();
		virtual void OnEnableChanged(){}

		virtual void AlignText();
		RECT GetScissorRegion() const;
		void GetScissorIntersection(RECT& region);
		virtual void SetUseBorder(bool use);
		void RefreshBorder();
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual void CalcTextWidth(); // virtual for mutiline text
		void OnDrag(int dx, int dy);
		virtual void OnChildHasDragged(){}
		
	private:
		friend class Container;
		void ToolTipEvent(IEventHandler::EVENT evt, const Vec2& mouseNPos);

		int ParseIntPosX(const std::string& posX);
		int ParseIntPosY(const std::string& posY);

		void ProcessHighlight(float dt);
		void ApplyAnim(IUIAnimation* anim, Vec2& pos, Vec2& scale, bool& hasPos, bool& hasScale);
	};
}