#pragma once

#include <UI/Wnd.h>
#include <UI/IFileSelector.h>

namespace fastbird
{

class StaticText;
class TextField;
class ListBox;
class Button;

class FileSelector : public Wnd, public IFileSelector
{
public:
	FileSelector();
	~FileSelector();

	virtual void OnCreated();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::FileSelector; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// Event
	void OnOK(void* pButton);
	void OnCancel(void* pButton);
	void OnListClick(void* pList);
	void OnListDoubleClick(void* pList);
	void OnDriveClick(void* pButton);

	// IFileSelector
	void SetTitle(const wchar_t* szTitle);
	void ListFiles(const char* folder, const char* filter);
	std::string GetFile() const { return mFile;}


private:
	std::string mFile;
	std::string mFolder;
	std::string mFilter;;
	StaticText* mStaticText;
	std::vector<Button*> mDriveButtons;
	TextField* mFileTextField;
	ListBox* mListBox;
	Button* mOKButton;
	Button* mCancelButton;
};

}