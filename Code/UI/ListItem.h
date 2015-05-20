#pragma once

#include <UI/Container.h>

namespace fastbird
{
	class CheckBox;
	class IUIObject;
	class ListItem : public Container
	{
	public:
		ListItem();
		virtual ~ListItem();
		static const size_t INVALID_INDEX;

		// IWinBase
		virtual ComponentType::Enum GetType() const { return ComponentType::ListItem; }
		virtual void GatherVisit(std::vector<IUIObject*>& v);

		// Own
		void SetRowIndex(size_t index);
		size_t GetRowIndex() const { return mRowIndex; }

		void SetColIndex(size_t index) { mColIndex = index; }
		size_t GetColIndex() const { return mColIndex; }
		CheckBox* ListItem::GetCheckBox() const;

		void SetBackColor(const char* backColor);
		void SetNoBackground(bool noBackground);
		const char* GetBackColor() const { return mBackColor.c_str(); }
		bool GetNoBackground() const { return mNoBackground; }

		virtual void OnFocusLost();
		virtual void OnFocusGain();

		//virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);

	protected:
		const static float LEFT_GAP;
		virtual void OnPosChanged();
		virtual void OnSizeChanged();

		size_t mRowIndex;
		size_t mColIndex;

		// backup values
		std::string mBackColor;
		bool mNoBackground;
	};
}