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
#include "FileSystem.h"
#include "DirectoryIterator.h"
#include "FBDebugLib/Logger.h"
#include "FBStringLib/StringLib.h"
#include "FBCommonHeaders/Helpers.h"
using namespace fb;

static bool gLogginStarted = false;
static boost::filesystem::path gWorkingPath;
std::string gApplicationName;
void FileSystem::StartLoggingIfNot(){
	if (gLogginStarted)
		return;
	auto filepath = "_FBFileSystem.log";
	FileSystem::BackupFile(filepath, 5, "Backup_Log");
	Logger::Init(filepath);
	gLogginStarted = true;
}

void FileSystem::StopLogging(){
	if (!gLogginStarted)
		return;

	Logger::Release();
}

bool FileSystem::Exists(const char* path){
	return boost::filesystem::exists(path);
}

bool FileSystem::IsDirectory(const char* path){
	return boost::filesystem::is_directory(path);
}

int FileSystem::Rename(const char* path, const char* newpath){
	if (!Exists(path)){
		return RENAME_NO_SOURCE;
	}

	if (Exists(newpath)){
		return RENAME_DEST_EXISTS;
	}
	
	if (!SecurityOK(path) || !SecurityOK(newpath))
		return SECURITY_NOT_OK;
	try{
		boost::filesystem::rename(path, newpath);
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	
	return FB_NO_ERROR;
}

int FileSystem::CopyFile(const char* src, const char* dest, bool overwrite, bool supressErrorMsg){
	if (!Exists(src)){
		return COPYFILE_NO_SOURCE;
	}
	if (!overwrite && Exists(dest)){
		return COPYFILE_DEST_ALREADY_EXISTS;
	}
	if (overwrite){
		try{
			boost::filesystem::copy_file(src, dest, boost::filesystem::copy_option::overwrite_if_exists);
		}
		catch (boost::filesystem::filesystem_error& err){
			if (!supressErrorMsg)
				Logger::Log(FB_ERROR_LOG_ARG, err.what());
			return COPYFILE_ERROR;
		}
	}
	else{
		try{
			boost::filesystem::copy_file(src, dest);
		}
		catch (boost::filesystem::filesystem_error& err){
			if (!supressErrorMsg)
				Logger::Log(FB_ERROR_LOG_ARG, err.what());
			return COPYFILE_ERROR;
		}
	}

	return FB_NO_ERROR;
}

bool FileSystem::Remove(const char* path){	
	bool ret = true;
	try{
		ret = boost::filesystem::remove(path);
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}	
	return ret;
}

std::string FileSystem::ReplaceExtension(const char* path, const char* ext){
	boost::filesystem::path boostPath(path);
	boostPath.replace_extension(ext);
	return boostPath.generic_string();
}

std::string FileSystem::ReplaceFilename(const char* path, const char* newFilename){
	boost::filesystem::path boostPath(path);
	boostPath = boostPath.remove_filename();
	boostPath.append(newFilename);
	return boostPath.generic_string();
}

const char* FileSystem::GetExtension(const char* path){
	size_t len = strlen(path);
	if (len == 0)
		return "";
	while (--len)
	{
		if (path[len] == '.')
		{
			return &path[len];
		}
	}
	return "";
}

const char* FileSystem::GetExtensionWithOutDot(const char* path){
	size_t len = strlen(path);
	int count = 0;
	while (--len)
	{
		if (path[len] == '.')
		{
			if (count == 0)
				return "";
			else
				return &path[len+1];
		}
		++count;
	}
	return "";
}

std::string FileSystem::GetFileName(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.filename().generic_string();
}

std::string FileSystem::GetName(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.stem().generic_string();
}

std::string FileSystem::GetParentPath(const char* path){
	boost::filesystem::path filepath(path);
	return filepath.parent_path().generic_string();
}

std::string FileSystem::GetLastDirectory(const char* path){
	auto parent = GetParentPath(path);
	auto found = parent.find_last_of("/");
	if (found != std::string::npos){
		return parent.substr(found + 1);
	}
	return parent;

}

std::string FileSystem::ConcatPath(const char* path1, const char* path2){
	return boost::filesystem::path(path1).concat(path2).generic_string();
}

std::string FileSystem::UnifyFilepath(const char* path){
	return boost::filesystem::path(path).generic_string();
}

std::string FileSystem::Absolute(const char* path){
	std::string ret = boost::filesystem::absolute(path).generic_string();
	if (ret.size() > 2 && ret.back() == '.')
		ret.erase(ret.end() - 1);
	return ret;
}

std::string FileSystem::MakrEndingSlashIfNot(const char* directory){
	if (!ValidCStringLength(directory))
		return std::string();
	std::string ret = boost::filesystem::path(directory).generic_string();
	if (ret.back() != '/')
		ret.push_back('/');
	return ret;
}

std::string FileSystem::StripFirstDirectoryPath(const char* strFilepath, bool* outStripped){
	using namespace boost::filesystem;
	if(outStripped){
		*outStripped = false;
	}
	std::string ret = path(strFilepath).generic_string();
	auto pos = ret.find_first_of('/');
	if (pos != std::string::npos){
		if (outStripped){
			*outStripped = true;
		}
		ret.erase(ret.begin(), ret.begin() + pos + 1);
	}
	return ret;
}

void FileSystem::BackupFile(const char* filepath, unsigned numKeeping) {
	BackupFile(filepath, numKeeping, "./");
}

void FileSystem::BackupFile(const char* filepath, unsigned numKeeping, const char* directory){
	std::string strDirectory = MakrEndingSlashIfNot(directory);
	auto backupPath = FileSystem::ReplaceExtension(filepath, "");
	auto extension = FileSystem::GetExtension(filepath);
	for (int i = (int)numKeeping - 1; i > 0; --i){
		auto oldPath = FormatString("%s%s_bak%d%s", strDirectory.c_str(), backupPath.c_str(), i, extension);
		auto newPath = FormatString("%s%s_bak%d%s", strDirectory.c_str(), backupPath.c_str(), i + 1, extension);		
		boost::filesystem::create_directories(boost::filesystem::path(newPath).parent_path());
		FileSystem::Rename(oldPath.c_str(), newPath.c_str());
	}
	auto newPath = FormatString("%s%s_bak%d%s", strDirectory.c_str(), backupPath.c_str(), 1, extension);
	FileSystem::Rename(filepath, newPath.c_str());
}

int FileSystem::CompareFileModifiedTime(const char* file1, const char* file2){
	if (!Exists(file1) || !Exists(file2))
		return FILE_NO_EXISTS;	
	if (!SecurityOK(file1) || !SecurityOK(file2))
		return SECURITY_NOT_OK;

	try{
		auto time1 = boost::filesystem::last_write_time(file1);
		auto time2 = boost::filesystem::last_write_time(file2);
		if (time1 < time2)
			return -1;
		else if (time1 == time2)
			return 0;
		else
			return 1;

	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return 0;
}

bool FileSystem::SecurityOK(const char* filepath){
	auto cwd = GetCurrentDir();
	auto abspath = boost::filesystem::absolute(filepath);
	if (abspath.generic_string().find(cwd) != std::string::npos)
		return true;

	return false;
}

BinaryData FileSystem::ReadBinaryFile(const char* path, std::streamoff& outLength){
	std::ifstream is(path, std::ios_base::binary);
	if (is)
	{
		is.seekg(0, is.end);
		outLength = is.tellg();
		is.seekg(0, is.beg);

		BinaryData buffer = BinaryData(new char[(unsigned int)outLength], [](char* obj){ delete[] obj; });
		is.read(buffer.get(), outLength);
		if (!is)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Only %u could be read for the file (%s)", is.gcount(), path).c_str());
		}
		is.close();
		return buffer;
	}
	else
		return 0;
}

void FileSystem::WriteBinaryFile(const char* path, char* data, size_t length){
	if (!data || length == 0 || path == 0)
		return;

	if (!SecurityOK(path))
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: SaveBinaryFile to %s has security violation.", path).c_str());
		return;
	}

	std::ofstream ofs(path, std::ios_base::binary | std::ios_base::trunc);
	if (!ofs)
	{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("FileSystem: Cannot open a file(%s) for writing.", path).c_str());
		return;
	}
	ofs.write(data, length);
}

//---------------------------------------------------------------------------
// Directory Operataions
//---------------------------------------------------------------------------
DirectoryIteratorPtr FileSystem::GetDirectoryIterator(const char* filepath, bool recursive){
	return DirectoryIteratorPtr(new DirectoryIterator(filepath, recursive),
		[](DirectoryIterator* obj){delete obj; });
}

bool FileSystem::CreateDirectory(const char* filepath){
	bool ret = true;
	try{
		ret = boost::filesystem::create_directories(filepath);
	}
	catch (boost::filesystem::filesystem_error& err){
		Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return ret;
}


//---------------------------------------------------------------------------
// System Folders
//---------------------------------------------------------------------------
void FileSystem::SetApplicationName(const char* name){
	gApplicationName = name;
}

std::string FileSystem::GetAppDataFolder(){
#if defined(_PLATFORM_WINDOWS_)
	PWSTR path=0;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, 0, &path))){
		auto ret = std::string(WideToAnsi(path));
		if (!gApplicationName.empty())
			ret += "\\my games\\" + gApplicationName;
		CoTaskMemFree(path);
		boost::filesystem::path p(ret);
		return p.generic_string();		
	}	
#else
	assert(0 && "Not implemented");
#endif
	return std::string("./temp/");
}


std::string FileSystem::GetCurrentDir(){
	if (gWorkingPath.empty()){
		gWorkingPath = boost::filesystem::current_path(); // absolute
	}
	return gWorkingPath.generic_string();
}