#pragma once
#include <UI/ListBox.h>
#include <UI/KeyValueDataSet.h>
namespace fastbird{

	class PropertyList : public ListBox
	{
		unsigned mFocusRow;
		unsigned mStartIndex;
		unsigned mEndIndex;

		KeyValueDataSet mData;
		typedef std::vector<std::pair<ListItem*, ListItem*> > PropertyListRecycle;
		PropertyListRecycle mRecycleBin;

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

		virtual void Scrolled();

		void MoveToRecycle(unsigned row);

		void VisualizeData(unsigned index);

		virtual float GetContentHeight() const;
		void Sort();
		void GoToNext(char c, unsigned curIndex);
		void MoveFocusToEdit(unsigned index);
		void MoveToNextLine();

		

	protected:
		virtual void OnSizeChanged();
		
	private:
		ListItem* CreateNewKeyItem(int row, int col, float ny);
		ListItem* CreateNewValueItem(int row, int col, float ny);		
	};
}