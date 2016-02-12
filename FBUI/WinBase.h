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
#include "EventHandler.h"
#include "VisibleStatus.h"
#include "UIProperty.h"
#include "ComponentType.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(IInputInjector);
	FB_DECLARE_SMART_PTR(UIAnimation);	
	FB_DECLARE_SMART_PTR(ImageBox);
	struct RegionTestParam;
	FB_DECLARE_SMART_PTR(UIObject);
	FB_DECLARE_SMART_PTR(Container);
	FB_DECLARE_SMART_PTR(WinBase);
	class FB_DLL_UI WinBase : public EventHandler
	{
	protected:
		static const int WinBase::LEFT_GAP;
		static const int WinBase::BOTTOM_GAP;
		static const float NotDefined;
		static bool sSuppressPropertyWarning;

		static Vec2I sLastPos;
		friend class VisibleStatus;
		friend class UIManager;

		WinBaseWeakPtr mSelfPtr;
		HWindowId mHwndId;
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
		Vec2 mWNPos;
		// scrolling offset;
		Vec2 mWNScrollingOffset;

		float mAspectRatio;
		bool mAspectRatioSet;
		bool mUseAbsoluteXPos;
		bool mUseAbsoluteYPos;

		bool mUseAbsoluteXSize;
		bool mUseAbsoluteYSize;

		ContainerWeakPtr mParent;
		WinBaseWeakPtr mManualParent;

		Vec2I mAbsOffset; // when loading;
		Vec2I mSizeMod;
		Vec2 mNOffset;


		bool mMouseIn;
		bool mMouseInPrev;		

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

		UIObjectPtr mUIObject;

		static HCURSOR sCursorOver;
		static HCURSOR sCursorAll;
		static HCURSOR sCursorNWSE;
		static HCURSOR sCursorNESW;
		static HCURSOR sCursorWE;
		static HCURSOR sCursorNS;
		static HCURSOR sCursorArrow;
		
		bool mMouseDragStartInHere;

		Vec2 mDestNPos;
		float mAnimationSpeed;
		VectorMap<std::string, UIAnimationPtr> mAnimations;
		std::wstring mTooltipText;
		std::string mTooltipTextBeforeT;
		// this is not related to mAnimations
		bool mSimplePosAnimEnabled;
		bool mNoMouseEvent;
		bool mNoMouseEventAlone;
		bool mVisualOnlyUI;
		bool mUseScissor;
		bool mEnable;

		std::vector<ImageBoxPtr> mBorders;

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
		bool mRunTimeChildRecursive;
		bool mGhost;
		bool mGatheringException;
		bool mKeepUIRatio;
		bool mUpdateAlphaTexture;
		bool mHand;
		bool mNoFocusByClick;
		bool mReceiveEventFromParent;
		FunctionId mHandFuncId;
		int mUserDataInt;
		bool mUseBorderAlpha;

		VectorMap<UIEvents::Enum, std::string> mEventFuncNames;

		WinBase();
		virtual ~WinBase();

	public:		

		static void SuppressPropertyWarning(bool warning);

		virtual void SetHwndId(HWindowId hwndId);
		virtual void OnResolutionChanged(HWindowId hwndId);
		virtual HWindowId GetHwndId() const;
		WinBasePtr GetPtr();
		virtual void OnCreated(){}
		virtual ComponentType::Enum GetType() const = 0;

		virtual WinBasePtr AddChild(float posX, float posY, float width, float height, ComponentType::Enum type) {
			return 0;
		}
		virtual WinBasePtr AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type) {
			return 0;
		}
		virtual WinBasePtr AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type){
			return 0;
		}
		virtual WinBasePtr AddChild(const fb::LuaObject& compTable) {
			return 0;
		}
		virtual WinBasePtr AddChild(ComponentType::Enum type) {
			return 0;
		}
		virtual void RemoveChild(WinBasePtr child, bool immediately = false) {}
		virtual void RemoveAllChildren(bool immediately = false) {}		
		virtual WinBasePtr GetChild(const std::string& name, bool includeSubChildren = false) { return 0; }
		virtual WinBasePtr GetChild(unsigned idx) { return 0; }
		ContainerPtr GetParent() const { return mParent.lock(); }
		WinBasePtr GetManualParent() const { return mManualParent.lock(); }
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
		virtual void SetFillX(bool fill);
		virtual void SetFillY(bool fill);
		virtual bool GetFillX() const;
		virtual bool GetFillY() const;

		virtual void ModifySize(const Vec2I& sizemod);
		virtual void SetWNSize(const fb::Vec2& size);
		virtual void OnParentSizeChanged();
		void SetAspectRatio(float ratio) { mAspectRatioSet = true; mAspectRatio = ratio; }
		// called when the parent size has changed.
		virtual void NotifySizeChange() {
			/*nothing to do if this is not a container.*/
		} 		
		virtual void SetSpecialOrder(int specialOrder);
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
		
		virtual void SetNPos(const fb::Vec2& pos); // normalized pos (0.0~1.0)
		virtual void SetNPosX(float x);
		virtual void SetNPosY(float y);

		virtual void SetInitialOffset(Vec2I offset);
		virtual void Move(Vec2I amount);
		virtual void SetWNPos(const fb::Vec2& wnPos);
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
		virtual const Vec2I GetAlignedPos() const;
		//virtual const Vec2& GetWNSize() const { return mWNSize; }
		virtual const Vec2& GetNSize() const { return mNSize; }
		virtual const Vec2I& GetSize() const { return mSize; }
		virtual const Vec2I& GetInitialOffset() const { return mAbsOffset; }
		virtual const Vec2I& GetSizeMod() const { return mSizeMod; }
		virtual void SetSizeMod(const Vec2I& mod);
		
		// coordinates are decided by functions like SetNPos():for relative or SetPos() for absolute.
		virtual void SetUseAbsPos(bool use){ mUseAbsoluteXPos = use; mUseAbsoluteYPos = use; }
		virtual void SetUseAbsXPos(bool use) { mUseAbsoluteXPos = use; }
		virtual void SetUseAbsYPos(bool use) { mUseAbsoluteYPos = use; }
		virtual bool GetUseAbsXPos() const { return mUseAbsoluteXPos; }
		virtual bool GetUseAbsYPos() const { return mUseAbsoluteYPos; }
		virtual void SetUseAbsSize(bool use){ mUseAbsoluteXSize = use; mUseAbsoluteYSize = use; }
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
		virtual ALIGNH::Enum GetHAlign() const{
			return mAlignH;
		}
		virtual void OnStartUpdate(float elapsedTime);
		virtual bool IsIn(IInputInjectorPtr injector) const;
		virtual bool IsIn(const Vec2I& pt, bool ignoreScissor, Vec2I* expand = 0) const;
		virtual bool IsPtOnLeft(const Vec2I& pt, int area) const;
		virtual bool IsPtOnRight(const Vec2I& pt, int area) const;
		virtual bool IsPtOnTop(const Vec2I& pt, int area) const;
		virtual bool IsPtOnBottom(const Vec2I& pt, int area) const;
		virtual bool OnInputFromHandler(IInputInjectorPtr injector);
		virtual void OnFocusLost(){}
		virtual void OnFocusGain(){}
		std::string TranslateText(const char* text);
		virtual void SetTextColor(const Color& c);
		virtual void SetText(const wchar_t* szText);
		virtual const wchar_t* GetText() const;
		virtual int GetTextWidth() const { return mTextWidth; }
		virtual int GetTextEndPosLocal() const;

		virtual void SetPasswd(bool passwd) {};
		virtual UIAnimationPtr GetOrCreateUIAnimation(const char* name);
		virtual UIAnimationPtr GetUIAnimation(const char* name);
		virtual void SetUIAnimation(UIAnimationPtr anim);
		virtual void ClearAnimationResult();

		virtual EventHandler* GetEventHandler() const { return (EventHandler*)this; }

		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
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
		virtual const Rect& GetRegion() const;
		// OWN
		// local space.
		fb::Vec2 ConvertToAlignedPos(const fb::Vec2& beforeAlign) const;
		fb::Vec2I ConvertToScreen(const fb::Vec2 npos) const;
		fb::Vec2 ConvertToNormalized(const fb::Vec2I pos) const; // convert to 0~1


		void SetParent(ContainerPtr parent);
		// manually controlling objects are allowed to have parent which is not a container.
		// only for scissor culling.
		void SetManualParent(WinBasePtr parent);		

		static void InitMouseCursor();
		static HCURSOR GetMouseCursorOver();
		static void FinalizeMouseCursor();

		void PosAnimationTo(const Vec2& destNPos, float speed);

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual void Save(tinyxml2::XMLElement& elem);
		virtual bool ParseLua(const fb::LuaObject& compTable);
		virtual float GetTextBottomGap() const;

		virtual void RefreshScissorRects();

		virtual void SetEnable(bool enable, bool ignoreSame = true);
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

		WinBasePtr GetRootWnd() const;
		WinBase* GetRootWndRaw() const;

		virtual bool IsAlwaysOnTop() const{ return false; }

		virtual UIObjectPtr GetUIObj() const { return mUIObject; }

		virtual bool GetCloseByEsc() const { return false; }

		virtual void StartHighlight(float speed);
		virtual void StopHighlight();

		virtual bool GetNoBackground() const;
		virtual const Color GetBackColor();

		virtual float GetAlpha() const;
		virtual void OnAlphaChanged();
		virtual const char* GetMsgTranslationUnit() const;
		virtual void TriggerRedraw();
		virtual WinBasePtr WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const;
		virtual void GatherTabOrder(VectorMap<unsigned, WinBasePtr>& winbases) const;
		virtual WinBasePtr WinBaseWithTabOrder(unsigned tabOrder) const{
			return 0;
		}
		virtual int GetTabOrder() const { return mTabOrder; }
		virtual void GetBiggestTabOrder(int& curBiggest) const;
		virtual void TabPressed();

		virtual void SetSaveNameCheck(bool set);
		virtual bool GetSaveNameCheck() const;
		virtual void SetRuntimeChild(bool runtime) { mRunTimeChild = runtime; }
		virtual void SetRuntimeChildRecursive(bool runtime) { mRunTimeChildRecursive = runtime; }
		virtual bool IsRuntimeChild() const { return mRunTimeChild; }
		virtual bool IsRuntimeChildRecursive() const { return mRunTimeChildRecursive; }
		virtual void SetGhost(bool ghost){ mGhost = ghost; }
		virtual bool GetGhost() const{ return mGhost; }

		virtual float GetContentHeight() const;
		virtual float GetContentEnd() const;

		virtual bool IsKeyboardFocused() const;

		virtual void SetGatheringException(){ mGatheringException = true; }
		virtual bool GetGatheringException() const{
			return mGatheringException;
		}

		virtual void SetEvent(UIEvents::Enum e, const char* luaFuncName);
		virtual const char* GetEvent(UIEvents::Enum e);

		virtual bool GetNoMouseEvent() const { return mNoMouseEvent; }
		virtual bool GetNoMouseEventAlone() const { return mNoMouseEventAlone; }
		virtual bool GetVisualOnly() const { return mVisualOnlyUI; }
		virtual void RecreateBorders();

		virtual bool GetUseScissor() const { return mUseScissor; }
		virtual bool GetNoFocusByClick() const { return mNoFocusByClick; }
		virtual bool GetReceiveEventFromParent() const { return mReceiveEventFromParent; }		

	protected:
		virtual void OnPosChanged(bool anim);
		virtual void OnSizeChanged();
		virtual void OnEnableChanged(){}

		virtual void AlignText();
		Rect GetScissorRegion() const;
		void GetScissorIntersection(Rect& region);
		virtual void SetUseBorder(bool use);
		void SetUseBorderAlpha(bool use);
		void RefreshBorder();
		virtual void GatherVisit(std::vector<UIObject*>& v);
		virtual void CalcTextWidth(); // virtual for mutiline text
		void OnDrag(int dx, int dy);
		virtual void OnChildHasDragged(){}

		virtual void OnMouseIn(IInputInjectorPtr injector, bool propergated = false);
		virtual void OnMouseOut(IInputInjectorPtr injector, bool propergated = false);
		virtual void OnMouseHover(IInputInjectorPtr injector, bool propergated = false);
		virtual void OnMouseDown(IInputInjectorPtr injector);
		virtual void OnMouseClicked(IInputInjectorPtr injector);
		// return processed
		virtual bool OnMouseDoubleClicked(IInputInjectorPtr injector);
		virtual void OnMouseRButtonClicked(IInputInjectorPtr injector);
		virtual void OnMouseDrag(IInputInjectorPtr injector);

		
	private:
		friend class Container;
		void ToolTipEvent(UIEvents::Enum evt, const Vec2& mouseNPos);

		int ParseIntPosX(const std::string& posX);
		int ParseIntPosY(const std::string& posY);

		void ProcessHighlight(float dt);
		void ApplyAnim(UIAnimationPtr anim, Vec2& pos, Vec2& scale, bool& hasPos, bool& hasScale);

		void UpdateAlphaTexture();

		void OnHandPropChanged();

		void OnMouseHoverHand(void* arg);
	};
}