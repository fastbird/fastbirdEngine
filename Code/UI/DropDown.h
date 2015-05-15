#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class IUIObject;
	class Button;

	class DropDown : public Container
	{
	public:
		DropDown();
		virtual ~DropDown();

		// IWinBase
		virtual void OnCreated();
		virtual ComponentType::Enum GetType() const { return ComponentType::DropDown; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);
		virtual bool SetProperty(UIProperty::Enum prop, const char* val);
		virtual bool GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly);
		virtual size_t AddDropDownItem(WCHAR* szString);
		// alread added as a child
		virtual size_t AddDropDownItem(IWinBase* item);
		virtual size_t GetSelectedIndex() const;
		virtual void SetSelectedIndex(size_t index);
		virtual void SetReservedIndex(size_t index);
		virtual void OnFocusLost();
		virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
		virtual void OnParentVisibleChanged(bool show);
		virtual bool SetVisible(bool show);
		virtual void SetHwndId(HWND_ID hwndId);

	protected:
		void SetCommonProperty(IWinBase* item, size_t index);
		const static float LEFT_GAP;
		virtual void OnPosChanged();
		virtual void OnSizeChanged();

	private:
		void OnMouseClick(void* arg);
		void OnMouseHover(void* arg);
		void OnMouseOut(void* arg);
		void OnItemSelected(void* arg);

		void CloseOptions();

	private:
		int mCursorPos;
		bool mPasswd;
		Button* mButton;
		std::vector<Button*> mDropDownItems;
		size_t mCurIdx;
		size_t mReservedIdx;

		static DropDown* sCurrentDropDown;
	};

}