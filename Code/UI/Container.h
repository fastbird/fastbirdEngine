#pragma once
#include <UI/WinBase.h>
namespace fastbird
{
	class RadioBox;
	class Container : public WinBase
	{
	public:
		Container(){}
		virtual ~Container();

		virtual IWinBase* AddChild(float posX, float posY, float width, float height, ComponentType::Enum type);
		virtual void RemoveChild(IWinBase* child);
		virtual void OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual IWinBase* FocusTest(Vec2 normalizedMousePos);
		virtual bool GetFocus(bool includeChildren = false) const;
		virtual void OnStartUpdate();

		void OnClickRadio(RadioBox* pRadio);
		
	private:
		friend class WinBase;

	protected:
		virtual void GatherVisit(std::vector<IUIObject*>& v);
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

	};
}