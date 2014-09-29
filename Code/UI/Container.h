#pragma once
#include <UI/WinBase.h>
namespace fastbird
{
	class RadioBox;
	class Scroller;
	class Container : public WinBase
	{
	public:
		Container() : mScrollerV(0)
			, mUseScrollerH(0), mUseScrollerV(0), mBottomChild(0), mLastContentWNEnd(0.f)
			, mWndContentUI(0){}
		virtual ~Container();

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual IWinBase* AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type);
		virtual void RemoveChild(IWinBase* child);
		virtual void RemoveAllEvents(bool includeChildren);
		virtual IWinBase* GetChild(const char* name, bool includeSubChildren = false);
		virtual IWinBase* GetChild(unsigned idx);
		virtual unsigned GetNumChildren() const;
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(IMouse* mouse);
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void OnStartUpdate(float elapsedTime);
		virtual void OnChildPosSizeChanged(WinBase* child);
		virtual void SetVisible(bool visible);
		virtual void OnParentVisibleChanged(bool visible);
		virtual void Scrolled();
		virtual void SetNPosOffset(const Vec2& offset);
		virtual void SetAnimNPosOffset(const Vec2& offset);

		virtual bool SetProperty(UIProperty::Enum, const char*);

		void OnClickRadio(RadioBox* pRadio);

		virtual void RefreshScissorRects();

		virtual bool ParseXML(tinyxml2::XMLElement* pelem);
		
	private:
		friend class WinBase;

	protected:
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		/*virtual void GatherVisitAlpha(std::vector<IUIObject*>& v);*/
		virtual void OnPosChanged();
		virtual void OnSizeChanged();
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
		IWinBase* mBottomChild;
		float mLastContentWNEnd;
		IWinBase* mWndContentUI;

	};
}