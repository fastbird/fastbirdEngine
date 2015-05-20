#pragma once
#include <UI/ListItemDataType.h>

namespace fastbird{
	class ListBoxData{
		ListItemDataType::Enum mDataType;
		std::wstring mText; // or texturepath(or region) for texture.
		unsigned mKey;
		bool mChecked; // for check box data.

	public:
		ListBoxData();
		ListBoxData(ListItemDataType::Enum type, const wchar_t* text, bool checked = false);
		~ListBoxData();

		ListItemDataType::Enum GetDataType() const { return mDataType; }
		const wchar_t* GetText() const { return mText.c_str(); }
		bool GetChecked() const { return mChecked; }
		unsigned GetKey() const { return mKey; }
	
		void SetDataType(ListItemDataType::Enum type){ mDataType = type; }
		void SetText(const wchar_t* text){
			mText = text; 
			// need to set the type manually.
		}
		void SetChecked(bool checked){ mChecked = checked; mDataType = ListItemDataType::CheckBox; }
		void SetKey(unsigned key){ mKey = key; mDataType = ListItemDataType::NumberKey; }

		bool IsTextData() const;
		bool CanHaveFocus() const;

	};
}