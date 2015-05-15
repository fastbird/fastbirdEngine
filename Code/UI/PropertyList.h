#pragma once
#include <UI/ListBox.h>
namespace fastbird{

	class PropertyList : public ListBox
	{
		unsigned mFocusRow;
	public:
		PropertyList();
		virtual ~PropertyList();

		virtual void OnCreated();

		virtual ComponentType::Enum GetType() const { return ComponentType::PropertyList; }
		
		virtual unsigned InsertItem(const wchar_t* key, const wchar_t* value);
		virtual unsigned ModifyItem(const wchar_t* key, const wchar_t* value);
		virtual void RemoveItem(const wchar_t* key);
		virtual void ClearItems();

		const wchar_t* GetValue(const wchar_t* key);
		
		void SetFocusRow(unsigned row) { mFocusRow = row; }
		unsigned GetFocusRow() const { return mFocusRow; }

		bool GetCurKeyValue(std::string& key, std::string& value);
		
	private:
		ListItem* CreateNewKeyItem(int row, int col, float ny);
		ListItem* CreateNewValueItem(int row, int col, float ny);
		IWinBase* GetTextField(const wchar_t* key, unsigned* index);
		


		
	};
}