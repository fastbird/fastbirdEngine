#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class IUIObject;
	class Button;
	class Wnd;

	class DropDown : public Container
	{
	public:
		static const int ITEM_HEIGHT;
		DropDown();
		virtual ~DropDown();

		// IWinBase
		virtual void OnCreated();
		virtual void OnSizeChanged();
		virtual ComponentType::Enum GetType() const { return ComponentType::DropDown; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
		virtual size_t AddDropDownItem(WCHAR* szString);
		// alread added as a child
		virtual size_t AddDropDownItem(IWinBase* item);
		virtual void ClearDropDownItems();
		virtual size_t GetSelectedIndex() const;
		virtual void SetSelectedIndex(size_t index);
		virtual void SetReservedIndex(size_t index);
		virtual void OnFocusLost();
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual void OnParentVisibleChanged(bool show);		
		virtual void ModifyItem(unsigned index, UIProperty::Enum, const char* szString);
		const wchar_t* GetItemString(unsigned index);

	protected:
		void SetCommonProperty(IWinBase* item, size_t index);
		const static float LEFT_GAP;

	private:
		void OnMouseClick(void* arg);
		void OnItemSelected(void* arg);

		void CloseOptions();
		void SetVisibleDropDownItems(bool visible);

	private:
		int mCursorPos;
		bool mPasswd;
		Button* mButton;
		std::vector<Button*> mDropDownItems;
		size_t mCurIdx;
		size_t mReservedIdx;
		Wnd* mHolder;
		int mMaxHeight;
		bool mTriggerEvent;

		static DropDown* sCurrentDropDown;
		friend class UIManager;
	};

}