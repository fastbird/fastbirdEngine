#pragma once
#include <UI/ListBox.h>

namespace fastbird{
	class ListItem;
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

		const wchar_t* GetValue(const wchar_t* key);

		bool GetCurKeyValue(std::string& key, std::string& value);		

		void MoveFocusToEdit(unsigned index);
		void MoveLine(bool applyInput, bool next);
		void RemoveHighlight(unsigned index);
		void MoveFocusToKeyItem();

		//virtual void VisualizeData(unsigned index);

		void SetFocusRow(unsigned row){ mFocusRow = row; }
		unsigned GetFocusRow() const { return mFocusRow; }
		
	};
}