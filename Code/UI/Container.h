#pragma once
#include <UI/WinBase.h>
namespace fastbird
{
	class RadioBox;
	class Scroller;
	class Container : public WinBase
	{
	public:
		typedef std::function<float() > ChildrenContentEndFunc;
		typedef std::function<void() > ScrolledFunc;

		Container();
		virtual ~Container();

		struct BackupContentWnd{
			BackupContentWnd(Container** wndContent){
				mOriginal = wndContent;
				mWndContent = *wndContent;
				*wndContent = 0;
			}
			~BackupContentWnd(){
				if (mWndContent)
					*mOriginal = mWndContent;
			}

			Container* mWndContent;
			Container** mOriginal;
		};
		virtual void OnResolutionChanged(HWND_ID hwndId);

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type);
		virtual IWinBase* AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type);
		virtual IWinBase* AddChild(ComponentType::Enum type);
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable);
		virtual void RemoveChild(IWinBase* child, bool immediately = false);
		virtual void RemoveChildNotDelete(IWinBase* child);
		virtual void RemoveAllChild(bool immediately = false);
		virtual void RemoveAllChildExceptRuntime();
		virtual void RemoveAllEvents(bool includeChildren);
		virtual IWinBase* GetChild(const std::string& name, bool includeSubChildren = false);
		virtual IWinBase* GetChild(unsigned idx);
		virtual unsigned GetNumChildren(bool excludeRunTimeChild = false) const;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);

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
		virtual bool ParseLua(const fastbird::LuaObject& compTable);

		void SetChildrenPosSizeChanged() { mChildrenPosSizeChanged = true; }

		bool HasVScroll() { return mScrollerV != 0; }
		const Vec2& GetScrollOffset() const;
		void SetRender3D(bool render3D, const Vec2I& renderTargetSize);
		void MatchHeight(bool checkName);

		virtual void StartHighlight(float speed);
		virtual void StopHighlight();
		virtual void OnAlphaChanged();

		virtual void SetHwndId(HWND_ID hwndId);

		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const;
		virtual IWinBase* WinBaseWithTabOrder(unsigned tabOrder) const;
		virtual void GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const;
		virtual void GetBiggestTabOrder(int& curBiggest) const;
		virtual void TabPressed();
		virtual float GetContentHeight() const;
		virtual float GetContentEnd() const;
		virtual float GetChildrenContentEnd() const;
		
		virtual void SetSpecialOrder(int specialOrder);

		void TransferChildrenTo(Container* destContainer);
		void AddChild(IWinBase* child);
		void AddChildSimple(IWinBase* child);
		void DoNotTransfer(IWinBase* child);
		
		Container* GetWndContentUI() const { return mWndContentUI; }

		bool HasScissorIgnoringChild() const;

		void SetChildrenContentEndFunc(ChildrenContentEndFunc func);
		void SetScrolledFunc(ScrolledFunc func);
		ScrolledFunc SetScrolledFunc() const { return mScrolledFunc; }

		Scroller* GetScrollerV() const { return mScrollerV; }



	private:
		friend class WinBase;

	protected:
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		/*virtual void GatherVisitAlpha(std::vector<IUIObject*>& v);*/
		virtual void OnSizeChanged();
		virtual void OnPosChanged(bool anim);
		friend class UIManager;

		virtual void OnMouseIn(IMouse* mouse, IKeyboard* keyboard, bool propergated = false);
		virtual void OnMouseOut(IMouse* mouse, IKeyboard* keyboard, bool propergated = false);
		virtual void OnMouseHover(IMouse* mouse, IKeyboard* keyboard, bool propergated = false);

	protected:
		typedef std::list<IWinBase*> COMPONENTS;
		COMPONENTS mChildren;
		COMPONENTS mPendingDelete;
		Scroller* mScrollerV;
		bool mUseScrollerH;
		bool mUseScrollerV;
		Container* mWndContentUI;
		bool mChildrenPosSizeChanged;
		bool mChildrenChanged;  // only detecting addition. not deletion.
		bool mMatchHeight;
		bool mHandlingInput;
		bool mSendEventToChildren;
		COMPONENTS::reverse_iterator mCurInputHandling;
		bool mCurInputHandlingChanged;

		std::set<IWinBase*> mDoNotTransfer;
		
		ChildrenContentEndFunc mChildrenContentEndFunc;
		ScrolledFunc mScrolledFunc;
	};
}