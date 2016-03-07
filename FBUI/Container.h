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
#include "WinBase.h"
namespace fb
{
	class RadioBox;	
	FB_DECLARE_SMART_PTR(Scroller);
	FB_DECLARE_SMART_PTR(Container);
	class FB_DLL_UI Container : public WinBase
	{
	public:
		typedef std::function<float() > ChildrenContentEndFunc;
		typedef std::function<void() > ScrolledFunc;

		Container();
		virtual ~Container();		
		virtual void OnResolutionChanged(HWindowId hwndId);

		virtual WinBasePtr AddChild(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual WinBasePtr AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type);
		virtual WinBasePtr AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type);
		virtual WinBasePtr AddChild(ComponentType::Enum type);
		virtual WinBasePtr AddChild(const fb::LuaObject& compTable);
		virtual void RemoveChild(WinBasePtr child, bool immediately = false);
		virtual void RemoveAllChildren(bool immediately = false);
		virtual void RemoveAllChildExceptRuntime();
		virtual void RemoveAllEvents(bool includeChildren);
		virtual WinBasePtr GetChild(const std::string& name, bool includeSubChildren = false);
		virtual WinBasePtr GetChild(unsigned idx);
		virtual unsigned GetNumChildren(bool excludeRunTimeChild = false) const;
		virtual bool OnInputFromHandler(IInputInjectorPtr injector);

		// called when the parent size has changed.
		virtual void NotifySizeChange();
		virtual void NotifyPosChange();		

		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void OnStartUpdate(float elapsedTime);
		virtual void RefreshVScrollbar();
		virtual bool SetVisible(bool visible);
		virtual bool SetVisibleChildren(bool show);
		virtual void SetVisibleInternal(bool visible);
		virtual void OnParentVisibleChanged(bool visible);
		virtual void Scrolled();
		virtual void SetWNScollingOffset(const Vec2& offset);

		virtual bool SetProperty(UIProperty::Enum, const char*);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);

		void OnClickRadio(RadioBox* pRadio);

		virtual void RefreshScissorRects();

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		void ParseXMLChildren(tinyxml2::XMLElement* pelem);
		virtual void Save(tinyxml2::XMLElement& elem);
		void SaveChildren(tinyxml2::XMLElement& elem);
		virtual bool ParseLua(const fb::LuaObject& compTable);

		void SetChildrenPosSizeChanged() { mChildrenPosSizeChanged = true; }

		bool HasVScroll() { return !mScrollerV.expired(); }
		const Vec2& GetScrollOffset() const;
		void SetScrollOffset(const Vec2& offset);
		void SetRender3D(bool render3D, const Vec2I& renderTargetSize);
		void MatchHeight(bool checkName);

		virtual void StartHighlight(float speed);
		virtual void StopHighlight();
		virtual void OnAlphaChanged();

		virtual void SetHwndId(HWindowId hwndId);

		virtual WinBasePtr WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const;
		virtual WinBasePtr WinBaseWithTabOrder(unsigned tabOrder) const;
		virtual void GatherTabOrder(VectorMap<unsigned, WinBasePtr>& winbases) const;
		virtual void GetBiggestTabOrder(int& curBiggest) const;
		virtual void TabPressed();
		virtual float GetContentHeight() const;
		virtual float GetContentEnd() const;
		virtual float GetChildrenContentEnd() const;
		int GetChildrenContentEndInPixel() const;
		
		virtual void SetSpecialOrder(int specialOrder);

		void TransferChildrenTo(ContainerPtr destContainer);
		void AddChild(WinBasePtr child);
		void AddChildSimple(WinBasePtr child);
		void DoNotTransfer(WinBasePtr child);
		
		ContainerPtr GetWndContentUI() const { return mWndContentUI.lock(); }

		bool HasScissorIgnoringChild() const;

		void SetChildrenContentEndFunc(ChildrenContentEndFunc func);
		void SetScrolledFunc(ScrolledFunc func);
		ScrolledFunc SetScrolledFunc() const { return mScrolledFunc; }

		ScrollerPtr GetScrollerV() const { return mScrollerV.lock(); }

		virtual void ReservePendingDelete(bool pendingDelete);
		void RemoveGapChildren();
		void GetChildrenNames(LuaObject& t);



	private:
		friend class WinBase;

	protected:
		virtual void GatherVisit(std::vector<UIObject*>& v);
		/*virtual void GatherVisitAlpha(std::vector<IUIObject*>& v);*/
		virtual void OnSizeChanged();
		virtual void OnPosChanged(bool anim);
		friend class UIManager;

		virtual void OnMouseIn(IInputInjectorPtr injector, bool propergated = false);
		virtual void OnMouseOut(IInputInjectorPtr injector, bool propergated = false);
		virtual void OnMouseHover(IInputInjectorPtr injector, bool propergated = false);

	protected:
		typedef std::list<WinBasePtr> ComponentPtrs;
		typedef std::list<WinBasePtr> ComponentWeakPtrs;
		ComponentPtrs mChildren;
		ComponentWeakPtrs mPendingDelete;
		ScrollerWeakPtr mScrollerV;
		bool mUseScrollerH;
		bool mUseScrollerV;
		ContainerWeakPtr mWndContentUI;
		bool mChildrenPosSizeChanged;
		bool mChildrenChanged;  // only detecting addition. not deletion.
		bool mMatchHeight;
		bool mHandlingInput;
		bool mSendEventToChildren;
		ComponentPtrs::reverse_iterator mCurInputHandling;
		bool mCurInputHandlingChanged;

		std::set<WinBaseWeakPtr, std::owner_less<WinBaseWeakPtr>> mDoNotTransfer;
		
		ChildrenContentEndFunc mChildrenContentEndFunc;
		ScrolledFunc mScrolledFunc;
	};
}