#pragma once
#include <UI/ListItemDataType.h>

namespace fastbird{
	class ITexture;
	class ListBoxData{
		ListItemDataType::Enum mDataType;
		std::wstring mText; // or texturepath(or region) for texture.
		unsigned mKey;
		SmartPtr<ITexture> mTexture;
		bool mChecked; // for check box data.
		union Number{
			int i;
			float f;
		};
		Number mNumber;

	public:
		ListBoxData();
		ListBoxData(ListItemDataType::Enum type, const wchar_t* text, bool checked = false);
		~ListBoxData();

		ListItemDataType::Enum GetDataType() const { return mDataType; }
		const wchar_t* GetText() const { return mText.c_str(); }
		bool GetChecked() const { return mChecked; }
		unsigned GetKey() const { return mKey; }
		int GetInt() const { return mNumber.i; }
		float GetFloat() const { return mNumber.f; }
		
		ITexture* GetTexture() const { return mTexture; }
	
		void SetDataType(ListItemDataType::Enum type){ mDataType = type; }
		void SetText(const wchar_t* text){
			mText = text; 
			// need to set the type manually.
		}
		void SetTexture(ITexture* texture);
		void SetInt(int number); // numeric up down
		void SetFloat(float f);
		void SetChecked(bool checked){ mChecked = checked; mDataType = ListItemDataType::CheckBox; }
		void SetKey(unsigned key){ mKey = key; mDataType = ListItemDataType::NumberKey; }

		bool IsTextData() const;
		bool CanHaveFocus() const;

	};
}