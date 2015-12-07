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

#include "stdafx.h"
#include "FileMonitor.h"
using namespace fb;
namespace fb {
	static const unsigned FILE_CHANGE_BUFFER_SIZE = 8000;
	class FileChangeMonitorThread : public Thread{		
		HANDLE mExitFileChangeThread;
		std::vector<BYTE> mFileChangeBuffer;
		OVERLAPPED	mOverlapped; // platform dependent.
		HANDLE mMonitoringDirectory;
		std::string mWatchDir;
		FB_CRITICAL_SECTION mChangedFilesGuard;
		std::set<std::string> mChangedFiles;
		bool mHasChangedFiles;
		bool mExiting;
	public:
		FileChangeMonitorThread()
			: mExiting(false)			
			, mHasChangedFiles(false)
			, mMonitoringDirectory(INVALID_HANDLE_VALUE)
		{
			mExitFileChangeThread = CreateEvent(0, FALSE, FALSE, "FileChangeMonitorExitEvent");
			mFileChangeBuffer.resize(FILE_CHANGE_BUFFER_SIZE);
			memset(&mOverlapped, 0, sizeof(OVERLAPPED));
			mOverlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);
		}
		~FileChangeMonitorThread(){
			CloseHandle(mExitFileChangeThread);
			if (mOverlapped.hEvent)
				CloseHandle(mOverlapped.hEvent);
		}

		void SetWatchDir(std::string path){
			mWatchDir = path;
			if (mWatchDir.empty())
				mWatchDir = "./";
			else
				mWatchDir = FileSystem::MakrEndingSlashIfNot(mWatchDir.c_str());
		}

		void Exit() {			
			CleanFileChangeMonitor();	
			CloseHandle(mMonitoringDirectory);
			ResetEvent(mMonitoringDirectory);
			mFileChangeBuffer.clear();
			if (mOverlapped.hEvent)
				CloseHandle(mOverlapped.hEvent);
			memset(&mOverlapped, 0, sizeof(OVERLAPPED));
			mOverlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);
		}

		void Stop(){
			mExiting = true;
			SetEvent(mExitFileChangeThread);
		}

		bool Run() {
			auto handleBackup = mOverlapped.hEvent;
			memset(&mOverlapped, 0, sizeof(OVERLAPPED));
			mOverlapped.hEvent = handleBackup;
			bool success = MonitorFileChange();
			if (!success)
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Fild change monitoring failed!");
				return false;
			}

			DWORD dwNumBytes;
			success = GetOverlappedResult(mMonitoringDirectory,
				&mOverlapped, &dwNumBytes, false) != 0;
			if (!success || dwNumBytes == 0)
			{
				return !mExiting;
			}
			assert(dwNumBytes >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

			if (!mExiting)
				ProcessFileChange();
			return !mExiting;
		}

		void CleanFileChangeMonitor()
		{
			::CancelIo(mMonitoringDirectory);
			::CloseHandle(mMonitoringDirectory);
			mMonitoringDirectory = 0;
		}

		void ProcessFileChange()
		{
			FILE_NOTIFY_INFORMATION* pFNI = (FILE_NOTIFY_INFORMATION*)&mFileChangeBuffer[0];
			while (pFNI)
			{
				switch (pFNI->Action)
				{
				case FILE_ACTION_MODIFIED:
				{
					char fileName[MAX_PATH];
					int count = WideCharToMultiByte(CP_ACP, 0, pFNI->FileName, pFNI->FileNameLength / sizeof(WCHAR),
						fileName, _ARRAYSIZE(fileName) - 1, 0, 0);
					fileName[count] = 0;
					auto unifiedPath = FileSystem::UnifyFilepath(fileName);
					LOCK_CRITICAL_SECTION lock(mChangedFilesGuard);
					mChangedFiles.insert(unifiedPath);
					mHasChangedFiles = true;

					for (int i = 0; i < 2; ++i){
						FileMonitor::GetInstance().OnChangeDetected();						
					}
				}
				break;
				}
				pFNI = pFNI->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)(((char*)pFNI) + pFNI->NextEntryOffset) : 0;
			}
		}

		bool MonitorFileChange()
		{
			//::CancelIo(mMonitoringDirectory);
			//::CloseHandle(mMonitoringDirectory);
			if (mMonitoringDirectory == INVALID_HANDLE_VALUE)
			{
				mMonitoringDirectory = CreateFile(mWatchDir.c_str(), FILE_LIST_DIRECTORY,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					0, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
				if (mMonitoringDirectory == INVALID_HANDLE_VALUE)
				{
					Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open the shader watch directory(%s)!", mWatchDir.c_str()).c_str());
					return false;
				}
			}

			DWORD writtenBytes = 0;
			memset(&mFileChangeBuffer[0], 0, FILE_CHANGE_BUFFER_SIZE);

			BOOL successful = ReadDirectoryChangesW(mMonitoringDirectory,
				&mFileChangeBuffer[0], FILE_CHANGE_BUFFER_SIZE,
				true, FILE_NOTIFY_CHANGE_LAST_WRITE, &writtenBytes,
				&mOverlapped, 0);
			if (!successful)
			{
				Logger::Log(FB_ERROR_LOG_ARG, "ReadDirectoryChangesW Failed!");
				DWORD lastError = GetLastError();
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("\t error code = %d", lastError).c_str());
				return false;
			}
			else
			{
				HANDLE handles[] = { mOverlapped.hEvent, mExitFileChangeThread };
				WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			}
			return true;
		}

		bool HasChangedFiles() const { return mHasChangedFiles; }
		void GetChangedFiles(std::vector<std::pair<std::string, std::string> >& files){
			LOCK_CRITICAL_SECTION lock(mChangedFilesGuard);
			for (auto& str : mChangedFiles){
				files.push_back(std::make_pair(mWatchDir, str));
			}
			mChangedFiles.clear();
			mHasChangedFiles = false;
		}
	};
}

static const int NumMaximumMonitor = 10;
class FileMonitor::Impl{
public:
	FileMonitor* mSelf;
	FileChangeMonitorThread mFileMonitorThread[NumMaximumMonitor];
	std::ofstream mErrorStream;
	std::streambuf* mStdErrorStream;
	std::vector<std::pair<std::string, std::string> > mChangedFiles;
	std::set<std::string> mIgnoreFileChanges;
	INT64 mLastCheckedTime;

	Impl(FileMonitor* self)
		: mLastCheckedTime(0)
		, mSelf(self)
	{
	}

	~Impl(){
		for (auto& it : mFileMonitorThread){
			it.Stop();
			it.Join();
		}
	}

	void StartMonitor(const char* dirPath){		
		if (!ValidCStringLength(dirPath))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		}
		int idx = 0;
		for (idx = 0; idx < NumMaximumMonitor; ++idx){
			if (!mFileMonitorThread[idx].IsJoinable()){
				break;
			}
		}
		if (idx >= NumMaximumMonitor){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot create a file monitor any more.");
			return;
		}
		auto& thread = mFileMonitorThread[idx];				
		thread.SetWatchDir(dirPath);
		thread.CreateThread(1024, FormatString("FileMonitorThread%d", idx).c_str());
	}
	void OnChangeDetected(){
		for (int i = 0; i < 2; ++i){
			auto& observers = mSelf->mObservers_[i]; //FileChange_Engine and FileChange_Game
			for (auto oit = observers.begin(); oit != observers.end(); /**/){
				IteratingWeakContainer(observers, oit, observer);				
				observer->OnChangeDetected();				
			}
		}
	}
	bool Check(){
		for (auto& it : mFileMonitorThread){
			if (it.HasChangedFiles()){								
				it.GetChangedFiles(mChangedFiles);				
			}
		}

		if (gpTimer->GetTickCount() - mLastCheckedTime > 500)
		{
			for (auto it = mChangedFiles.begin(); it != mChangedFiles.end();)
			{
				std::string filepath = it->second;
				std::string filefullpath = it->first + it->second;
				auto strs = Split(filepath, "~");
				if (strs.size() >= 2)
				{
					filepath = strs[0];
				}
				const char* extension = FileSystem::GetExtension(filepath.c_str());
				/*bool shader = _stricmp(extension, "hlsl") == 0 || _stricmp(extension, "h") == 0;
				bool material = _stricmp(extension, "material") == 0;
				bool texture = _stricmp(extension, "png") == 0 || _stricmp(extension, "dds") == 0;
				bool particle = _stricmp(extension, "particle") == 0;
				bool xml = _stricmp(extension, "xml") == 0;*/

				bool hasExtension = strlen(extension) != 0;
				bool sdfFile = _stricmp(extension, "sdf") == 0;
				bool canOpen = true;
				bool throwAway = false;
				auto ignoreIt = mIgnoreFileChanges.find(filepath);
				if (ignoreIt != mIgnoreFileChanges.end()){
					mIgnoreFileChanges.erase(ignoreIt);
					throwAway = true;
				}
				if (!hasExtension || sdfFile || throwAway)
				{
					it = mChangedFiles.erase(it);
					continue;
				}
				FILE* file = 0;
				errno_t err = fopen_s(&file, filefullpath.c_str(), "r");
				if (!err && file)
				{
					fclose(file);
					err = fopen_s(&file, filefullpath.c_str(), "a+");
					if (err){
						canOpen = false;
					}
				}
				if (file)
					fclose(file);

				if (canOpen)
				{
					/*int startEnum = shader || material || texture || particle || xml ?
						IFileChangeObserver::FileChange_Engine : IFileChangeObserver::FileChange_Game;*/
					for (int i = 0; i < 2; ++i){
						auto& observers = mSelf->mObservers_[i]; //FileChange_Engine and FileChange_Game
						for (auto oit = observers.begin(); oit != observers.end(); /**/){
							auto observer = oit->lock();
							if (!observer){
								oit = observers.erase(oit);
								continue;
							}
							++oit;
							std::string loweredStr(FileSystem::GetExtension(filepath.c_str()));
							ToLowerCase(loweredStr);
							bool processed = observer->OnFileChanged(it->first.c_str(), filepath.c_str(), loweredStr.c_str());
							if (processed)
								break;
						}
					}
					/*if (shader)
						IShader::ReloadShader(filepath.c_str());
					else if (material)
						IMaterial::ReloadMaterial(filepath.c_str());
					else if (texture)
						ITexture::ReloadTexture(filepath.c_str());
					else if (particle)
						ParticleManager::GetParticleManager().ReloadParticle(filepath.c_str());
					else if (xml && gFBEnv->pRenderer)
						mRenderer->ReloadTextureAtlas(filepath.c_str());*/

					it = mChangedFiles.erase(it);
					
				}
				else
				{
					++it;
				}
			}
		}
		return !mChangedFiles.empty();
	}

	void IgnoreMonitoringOnFile(const char* filepath){
		mIgnoreFileChanges.insert(filepath);
	}

	void ResumeMonitoringOnFile(const char* filepath){
		auto it = mIgnoreFileChanges.find(filepath);
		if (it != mIgnoreFileChanges.end())
			mIgnoreFileChanges.erase(it);
	}
};

//---------------------------------------------------------------------------
FileMonitorWeakPtr sMonitor;
FileMonitorPtr FileMonitor::Create(){
	if (sMonitor.expired()){
		FileMonitorPtr p(new FileMonitor, [](FileMonitor* obj){ delete obj; });
		sMonitor = p;
		return p;
	}
	return sMonitor.lock();
}

FileMonitor& FileMonitor::GetInstance(){
	if (sMonitor.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "FileMonitor is deleted. Program will crash...");
	}
	return *sMonitor.lock();
}

bool FileMonitor::HasInstance(){
	return !sMonitor.expired();
}


FileMonitor::FileMonitor()
: mImpl(new Impl(this))
{

}
FileMonitor::~FileMonitor(){

}

void FileMonitor::StartMonitor(const char* dirPath){
	mImpl->StartMonitor(dirPath);
}

bool FileMonitor::Check(){
	return mImpl->Check();
}

void FileMonitor::IgnoreMonitoringOnFile(const char* filepath){
	mImpl->IgnoreMonitoringOnFile(filepath);
}

void FileMonitor::ResumeMonitoringOnFile(const char* filepath){
	mImpl->ResumeMonitoringOnFile(filepath);
}

void FileMonitor::OnChangeDetected(){
	mImpl->OnChangeDetected();
}