/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "FileSelector.h"
#include "UIManager.h"
#include "TextField.h"
#include "ListBox.h"
#include "StaticText.h"
#include "Button.h"
#include "UIObject.h"

namespace fb
{
FileSelectorPtr FileSelector::Create(){
	FileSelectorPtr p(new FileSelector, [](FileSelector* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

FileSelector::FileSelector()
{	
}

FileSelector::~FileSelector()
{
}

void FileSelector::OnCreated(){
	SetProperty(UIProperty::BACK_COLOR, "0.15, 0.15, 0.15, 1.0");
	auto staticText = std::static_pointer_cast<StaticText>(AddChild(0.05f, 0.01f, 0.9f, 0.05f, ComponentType::StaticText));
	mStaticText = staticText;
	staticText->SetRuntimeChild(true);
	staticText->SetText(L"Select a file:");
	char buf[256];
	DWORD size = GetLogicalDriveStrings(256, buf);
	StringVector drives;
	size_t start = 0;
	for (DWORD i = 0; i<size; i++)
	{
		if (buf[i] == 0)
		{
			drives.push_back(std::string(buf + start, buf + i));
			start = i + 1;
			i++;
		}
	}
	float xpos = 0.05f;
	size_t numDrives = std::min((size_t)6, drives.size());
	for (size_t i = 0; i<numDrives; i++)
	{
		auto button = std::static_pointer_cast<Button>(AddChild(xpos, 0.07f, 0.08f, 0.05f, ComponentType::Button));
		mDriveButtons.push_back(button);
		button->SetRuntimeChild(true);
		button->SetProperty(UIProperty::BACK_COLOR, "0.30f, 0.30f, 0.30f, 1.0f");
		button->SetProperty(UIProperty::BACK_COLOR_OVER, "0.2, 0.2, 0.2, 1.0f");
		button->SetText(AnsiToWide(drives[i].c_str(), drives[i].size()));
		button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&FileSelector::OnDriveClick, this, std::placeholders::_1));
		xpos += 0.082f;
	}
	auto textField = std::static_pointer_cast<TextField>(
		AddChild(0.05f, 0.13f, 0.9f, 0.05f, ComponentType::TextField));
	mFileTextField = textField;
	textField->SetRuntimeChild(true);
	textField->SetProperty(UIProperty::BACK_COLOR, "0.10, 0.10, 0.10, 1.0");

	auto listBox = std::static_pointer_cast<ListBox>(
		AddChild(0.05f, 0.19f, 0.9f, 0.70f, ComponentType::ListBox));
	mListBox = listBox;
	listBox->SetRuntimeChild(true);
	listBox->SetProperty(UIProperty::BACK_COLOR, "0.10, 0.10, 0.10, 1.0");
	listBox->SetProperty(UIProperty::LISTBOX_COL, "1");
	listBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&FileSelector::OnListClick, this, std::placeholders::_1));
	listBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&FileSelector::OnListDoubleClick, this, std::placeholders::_1));

	auto button = std::static_pointer_cast<Button>(
		AddChild(0.2f, 0.93f, 0.3f, 0.05f, ComponentType::Button));
	mOKButton = button;
	button->SetRuntimeChild(true);
	button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&FileSelector::OnOK, this, std::placeholders::_1));
	button->SetProperty(UIProperty::TEXT_ALIGN, "center");
	button->SetText(L"OK");

	button = std::static_pointer_cast<Button>(
		AddChild(0.51f, 0.93f, 0.3f, 0.05f, ComponentType::Button));
	mCancelButton = button;
	button->SetRuntimeChild(true);
	button->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&FileSelector::OnCancel, this, std::placeholders::_1));
	button->SetProperty(UIProperty::TEXT_ALIGN, "center");
	button->SetText(L"Cancel");

	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

void FileSelector::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;

	__super::GatherVisit(v);
}

void FileSelector::OnOK(void* pButton)
{
	OnEvent(UIEvents::EVENT_FILE_SELECTOR_OK);
}
void FileSelector::OnCancel(void* pButton)
{
	mFile.clear();
	OnEvent(UIEvents::EVENT_FILE_SELECTOR_CANCEL);
}

void FileSelector::OnListClick(void* pList)
{
	ListBox* pListBox = (ListBox*)pList;
	mFile = FileSystem::ConcatPath(mFolder.c_str(), pListBox->GetSelectedString().c_str());	
	std::string unifiedPath = FileSystem::UnifyFilepath(mFile.c_str());
	
	if (!FileSystem::IsDirectory(unifiedPath.c_str()))
	{
		OnEvent(UIEvents::EVENT_FILE_SELECTOR_SELECTED);
		mFileTextField.lock()->SetText( AnsiToWide(unifiedPath.c_str()) );
	}
}

void FileSelector::OnListDoubleClick(void* pList)
{
	ListBox* pListBox = (ListBox*)pList;
	std::string strFilepath = FileSystem::ConcatPath(mFolder.c_str(), pListBox->GetSelectedString().c_str());
	std::string unifiedPath = FileSystem::UnifyFilepath(strFilepath.c_str());
	if ( FileSystem::IsDirectory(unifiedPath.c_str()) )
	{
		ListFiles( unifiedPath.c_str(), std::string(mFilter.c_str()).c_str() );
		UIManager::GetInstance().SetFocusUI(mListBox.lock());
	}
	else
	{
		OnEvent(UIEvents::EVENT_FILE_SELECTOR_OK);
		mFile = unifiedPath;
	}

	mFileTextField.lock()->SetText(AnsiToWide(unifiedPath.c_str()));
}

void FileSelector::SetTitle(const wchar_t* szTitle)
{
	assert(szTitle);
	mStaticText.lock()->SetText(szTitle);
}

void FileSelector::ListFiles(const char* folder, const char* filter)
{
	assert(folder);
	if (!folder || strlen(folder)==0)
		return;

	std::string folderBackup = mFolder;

	mFolder = folder;
	char buf[MAX_PATH] = {0};
	strcpy_s(buf, "haha");
	mFolder = FileSystem::Absolute(mFolder.c_str());	
	if (filter && strlen(filter)!=0)
	{
		mFilter = filter;
		ToLowerCase(mFilter);
	}	
	if (!FileSystem::Exists(folder)){
		Logger::Log(FB_ERROR_LOG_ARG, "Iterating dir is failed!");
		UIManager::GetInstance().DisplayMsg("Cannot open the directory.");
		mFolder = folderBackup;
		return;
	}	
	mFileTextField.lock()->SetText( AnsiToWide(mFolder.c_str(), mFolder.size() ) );
	mListBox.lock()->Clear();

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

	auto iterator = FileSystem::GetDirectoryIterator(folder, false);
	while(iterator->HasNext())
	{
		ListItem* pNewListItem = 0;
		bool isDir;
		std::string filename = FileSystem::GetFileName(iterator->GetNextFilePath(&isDir));
		if (isDir)
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
				const char* szExt = FileSystem::GetExtensionWithOutDot(filename.c_str());
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
	}

	for(auto& itDir: svDirs)
	{
		auto row = mListBox.lock()->InsertItem(itDir.c_str());
		mListBox.lock()->SetItem(Vec2I(row, 0), itDir.c_str(), ListItemDataType::String);
	}

	for(auto& itFile: svFiles)
	{
		auto row = mListBox.lock()->InsertItem(itFile.c_str());
		mListBox.lock()->SetItem(Vec2I(row, 0), itFile.c_str(), ListItemDataType::String);
	}
}

void FileSelector::OnDriveClick(void* pButton)
{
	Button* driveBtn = (Button*)pButton;
	ListFiles(WideToAnsi(driveBtn->GetText()), std::string(mFilter).c_str());
}
}