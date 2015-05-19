#pragma once
#include <UI/WinBase.h>
namespace fastbird
{
	class RadioBox;
	class Scroller;
	class Container : public WinBase
	{
	public:
		Container();
		virtual ~Container();

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type);
		virtual IWinBase* AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type);
		virtual IWinBase* AddChild(ComponentType::Enum type);
		virtual IWinBase* AddChild(const fastbird::LuaObject& compTable);
		virtual void RemoveChild(IWinBase* child, bool immediately = false);
		virtual void RemoveChildNotDelete(IWinBase* child);
		virtual void RemoveAllChild(bool immediately = false);
		virtual void RemoveAllEvents(bool includeChildren);
		virtual IWinBase* GetChild(const char* name, bool includeSubChildren = false);
		virtual IWinBase* GetChild(unsigned idx);
		virtual unsigned GetNumChildren(bool excludeRunTimeChild = false) const;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(IMouse* mouse);
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void OnStartUpdate(float elapsedTime);
		virtual void RefreshVScrollbar();
		virtual bool SetVisible(bool visible);
		virtual bool SetVisibleChildren(bool show);
		virtual void SetVisibleInternal(bool visible);
		virtual void OnParentVisibleChanged(bool visible);
		virtual void Scrolled();
		virtual void SetWNPosOffset(const Vec2& offset);
		virtual void SetAnimNPosOffset(const Vec2& offset);
		virtual void SetAnimScale(const Vec2& scale, const Vec2& pivot);

		virtual bool SetProperty(UIProperty::Enum, const char*);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);

		void OnClickRadio(RadioBox* pRadio);

		virtual void RefreshScissorRects();

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		virtual void Save(tinyxml2::XMLElement& elem);
		virtual bool ParseLua(const fastbird::LuaObject& compTable);

		void SetChildrenPosSizeChanged() { mChildrenPosSizeChanged = true; }

		virtual float PixelToLocalNWidth(int pixel) const;
		virtual float PixelToLocalNHeight(int pixel) const;
		virtual Vec2 PixelToLocalNSize(const Vec2I& pixel) const;

		bool HasVScroll() { return mScrollerV != 0; }
		const Vec2& GetScrollOffset() const;
		void SetRender3D(bool render3D, const Vec2I& renderTargetSize);
		void MatchHeight(bool checkName);

		virtual void StartHighlight(float speed);
		virtual void StopHighlight();
		virtual void OnAlphaChanged();

		virtual void SetHwndId(HWND_ID hwndId);

		virtual IWinBase* WinBaseWithPoint(const Vec2I& pt, bool container) const;
		virtual IWinBase* WinBaseWithTabOrder(unsigned tabOrder) const;
		virtual void GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const;
		virtual void TabPressed();
		virtual float GetContentHeight() const;
		

	private:
		friend class WinBase;

	protected:
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		/*virtual void GatherVisitAlpha(std::vector<IUIObject*>& v);*/
		virtual void OnPosChanged();
		virtual void OnSizeChanged();
		friend class UIManager;
		Vec2 ConvertChildSizeToWorldCoord(const fastbird::Vec2& size) const;
		Vec2 ConvertChildPosToWorldCoord(const fastbird::Vec2& pos) const;
		Vec2 ConvertWorldSizeToParentCoord(const fastbird::Vec2& worldSize) const;
		Vec2 ConvertWorldPosToParentCoord(const fastbird::Vec2& worldPos) const;

	protected:
		typedef std::list<IWinBase*> COMPONENTS;
		COMPONENTS mChildren;
		COMPONENTS mPendingDelete;
		Scroller* mScrollerV;
		bool mUseScrollerH;
		bool mUseScrollerV;
		IWinBase* mWndContentUI;
		bool mChildrenPosSizeChanged;
		bool mChildrenChanged;  // only detecting addition. not deletion.
		bool mMatchHeight;
		bool mHandlingInput;
		COMPONENTS::reverse_iterator mCurInputHandling;
		bool mCurInputHandlingChanged;
	};
}