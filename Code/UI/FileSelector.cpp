#include <UI/StdAfx.h>
#include <UI/FileSelector.h>
#include <UI/IUIManager.h>
#include <UI/TextField.h>
#include <UI/ListBox.h>
#include <UI/StaticText.h>
#include <UI/Button.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
FileSelector::FileSelector()
{	
	SetProperty(UIProperty::BACK_COLOR, "0.15, 0.15, 0.15, 1.0");
	mStaticText = static_cast<StaticText*>(
		AddChild(0.05f, 0.01f, 0.9f, 0.05f, ComponentType::StaticText) );
	mStaticText->SetText(L"Select a file:");
	char buf[256];
	DWORD size = GetLogicalDriveStrings(256, buf);
	StringVector drives;
	size_t start = 0;
	for (DWORD i=0; i<size; i++)
	{
		if (buf[i]==0)
		{
			drives.push_back(std::string(buf+start, buf+i));
			start=i+1;
			i++;
		}
	}
	float xpos = 0.05f;
	size_t numDrives = std::min((size_t)6, drives.size());
	for (size_t i=0; i<numDrives; i++)
	{
		mDriveButtons.push_back(static_cast<Button*>(
			AddChild(xpos, 0.07f, 0.08f, 0.05f, ComponentType::Button)));
		mDriveButtons.back()->SetProperty(UIProperty::BACK_COLOR, "0.30f, 0.30f, 0.30f, 1.0f");
		mDriveButtons.back()->SetProperty(UIProperty::BACK_COLOR_OVER, "0.2, 0.2, 0.2, 1.0f");
		mDriveButtons.back()->SetText(AnsiToWide(drives[i].c_str(), drives[i].size()));
		mDriveButtons.back()->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&FileSelector::OnDriveClick, this, std::placeholders::_1));
		xpos+= 0.082f;
	}
	mFileTextField = static_cast<TextField*>(
		AddChild(0.05f, 0.13f, 0.9f, 0.05f, ComponentType::TextField) );
	mFileTextField->SetProperty(UIProperty::BACK_COLOR, "0.10, 0.10, 0.10, 1.0");

	mListBox = static_cast<ListBox*>(
		AddChild(0.05f, 0.19f, 0.9f, 0.70f, ComponentType::ListBox) );
	mListBox->SetProperty(UIProperty::BACK_COLOR, "0.10, 0.10, 0.10, 1.0");
	mListBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK, 
		std::bind(&FileSelector::OnListClick, this, std::placeholders::_1));
	mListBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_DOUBLE_CLICK, 
		std::bind(&FileSelector::OnListDoubleClick, this, std::placeholders::_1));
	
	mOKButton = static_cast<Button*>(
		AddChild(0.2f, 0.93f, 0.3f, 0.05f, ComponentType::Button) );
	mOKButton->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK, 
		std::bind(&FileSelector::OnOK, this, std::placeholders::_1));
	mOKButton->SetProperty(UIProperty::TEXT_ALIGN, "center");
	mOKButton->SetText(L"OK");

	mCancelButton = static_cast<Button*>(
		AddChild(0.51f, 0.93f, 0.3f, 0.05f, ComponentType::Button) );
	mCancelButton->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&FileSelector::OnCancel, this, std::placeholders::_1));
	mCancelButton->SetProperty(UIProperty::TEXT_ALIGN, "center");
	mCancelButton->SetText(L"Cancel");	

	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

FileSelector::~FileSelector()
{
}

void FileSelector::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;

	__super::GatherVisit(v);
}

void FileSelector::OnOK(void* pButton)
{
	OnEvent(IEventHandler::EVENT_FILE_SELECTOR_OK);
}
void FileSelector::OnCancel(void* pButton)
{
	mFile.clear();
	OnEvent(IEventHandler::EVENT_FILE_SELECTOR_CANCEL);
}

void FileSelector::OnListClick(void* pList)
{
	ListBox* pListBox = (ListBox*)pList;
	mFile = ConcatFilepath(mFolder.c_str(), pListBox->GetSelectedString().c_str());
	char unifiedPath[MAX_PATH] = {0};
	UnifyFilepath(unifiedPath, mFile.c_str());
	// if dir
	if (!tinydir_isdir(unifiedPath))
	{
		OnEvent(IEventHandler::EVENT_FILE_SELECTOR_SELECTED);
		mFileTextField->SetText( AnsiToWide(unifiedPath, strlen(unifiedPath)) );
	}
}

void FileSelector::OnListDoubleClick(void* pList)
{
	ListBox* pListBox = (ListBox*)pList;
	std::string strFilepath = ConcatFilepath(mFolder.c_str(), pListBox->GetSelectedString().c_str());
	char unifiedPath[MAX_PATH] = {0};
	UnifyFilepath(unifiedPath, strFilepath.c_str());
	if ( tinydir_isdir(unifiedPath) )
	{
		ListFiles( unifiedPath, std::string(mFilter.c_str()).c_str() );
		IUIManager::GetUIManager().SetFocusUI(mListBox);
	}
	else
	{
		OnEvent(IEventHandler::EVENT_FILE_SELECTOR_OK);
		mFile = unifiedPath;
	}

	mFileTextField->SetText(AnsiToWide(unifiedPath, strlen(unifiedPath)));
}

void FileSelector::SetTitle(const wchar_t* szTitle)
{
	assert(szTitle);
	mStaticText->SetText(szTitle);
}

void FileSelector::ListFiles(const char* folder, const char* filter)
{
	assert(folder);
	if (!folder || strlen(folder)==0)
		return;

	std::string folderBackup = mFolder;

	mFolder = folder;
	char buf[MAX_PATH] = {0};
	mFolder = ToAbsolutePath(buf, mFolder.c_str());
	mFolder = UnifyFilepath(buf, mFolder.c_str());
	if (filter && strlen(filter)!=0)
	{
		mFilter = filter;
		ToLowerCase(mFilter);
	}	
	tinydir_dir dir;
	if (tinydir_open(&dir, folder) == -1)
	{
		Error("Iterating dir is failed!");
		IUIManager::GetUIManager().DisplayMsg("Cannot open the directory.");
		mFolder = folderBackup;
		return;
	}
	mFileTextField->SetText( AnsiToWide(mFolder.c_str(), mFolder.size() ) );
	mFileTextField->SetScissorRect(true, mFileTextField->GetRegion());
	mListBox->Clear();

	if (mFolder.size()<=3)
	{
		if (mFolder[1]==':')
		{
			// add drives

		}
	}
	WStringVector svDirs, svFiles;
	svDirs.reserve(100);
	svFiles.reserve(100);
	while(dir.has_next)
	{
		tinydir_file file;
		if (tinydir_readfile(&dir, &file)==-1)
		{
			Error("tinydir readfile failed!");
			break;
		}

		ListItem* pNewListItem = 0;
		std::string filename = file.name;
		if (file.is_dir)
		{
			if (filename!=".")
			{
				filename += "/";			
				svDirs.push_back( (AnsiToWide(filename.c_str(), filename.size())) );
			}
		}
		else
		{
			if (!mFilter.empty())
			{
				const char* szExt = GetExtension(filename.c_str());
				if (szExt!=0)
				{
					std::string ext(szExt);
					ToLowerCase(ext);
					if (mFilter.find(ext) != std::string::npos)
					{
						svFiles.push_back( AnsiToWide(filename.c_str(), filename.size()) );
					}
				}
			}
			else
			{
				svFiles.push_back( AnsiToWide(filename.c_str(), filename.size()) );
			}
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	FB_FOREACH(itDir, svDirs)
	{
		mListBox->InsertItem(itDir->c_str());
	}

	FB_FOREACH(itFile, svFiles)
	{
		mListBox->InsertItem(itFile->c_str());
	}
}

void FileSelector::OnDriveClick(void* pButton)
{
	Button* driveBtn = (Button*)pButton;
	ListFiles(WideToAnsi(driveBtn->GetText()), std::string(mFilter).c_str());
}
}