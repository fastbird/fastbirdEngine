#pragma once
#include "stdafx.h"
#include "File.h"
#include "FileSystem.h"
using namespace fb;

class File::Impl {
public:
	boost::filesystem::path mPath;	
	std::string mStrPath;
	Impl(const char* path)
		: mPath(path)
	{		
		mStrPath = mPath.generic_string();
	}
};


File::File(const char* path) 
	: mImpl(new Impl(path))
{
}

File::~File() {

}

FilePtr File::From(const char* path) {
	return FilePtr(new File(path), [](File* p) {delete p; });
}

bool File::Exists() const {
	return boost::filesystem::exists(mImpl->mPath);
}

FilePtr File::Parent() const {
	return File::From(mImpl->mPath.parent_path().generic_string().c_str());
}

std::string File::Absolute() const {
	std::string ret = boost::filesystem::absolute(mImpl->mPath).generic_string();
	if (ret.size() > 2 && ret.back() == '.')
		ret.erase(ret.end() - 1);
	return ret;
}

bool File::IsAbsolute() const
{
	return mImpl->mPath.is_absolute();
}

bool File::IsDir() const {
	return boost::filesystem::is_directory(mImpl->mPath);
}

bool File::MakeDir() const {
	bool ret = false;
	try {
		ret = boost::filesystem::create_directories(mImpl->mPath);
	}
	catch (boost::filesystem::filesystem_error& err) {
		 Logger::Log(FB_ERROR_LOG_ARG, err.what());
	}
	return ret;
}

bool File::operator==(const File& other) const {
	if (mImpl->mPath.empty() || other.mImpl->mPath.empty())
		return false;

	return mImpl->mPath == other.mImpl->mPath;
}

const std::string& File::Path() const {
	return mImpl->mStrPath;
}

const char* File::c_str() const {
	return mImpl->mStrPath.c_str();
}

File::PtrArray File::ListFiles() const {
	PtrArray files;
	if (!IsDir())
		return files;

	DirectoryIterator it(mImpl->mPath.generic_string().c_str(), false);
	while (it.HasNext()) {
		std::string path(it.GetNextFilePath());
		if (path != "." || path != "..") {
			files.push_back(File::From(path.c_str()));
		}
	}
	return files;
}

void File::DeleteOnExit() {
	FileSystem::DeleteOnExit(mImpl->mPath.generic_string().c_str());
}

bool File::HasExtension(const char* ext) {
	return FileSystem::HasExtension(mImpl->mPath.generic_string().c_str(), ext);
}

time_t File::LastModified() const {
	return boost::filesystem::last_write_time(mImpl->mPath);
}

ByteArray File::ReadBinary() const {
	return FileSystem::ReadBinaryFile(mImpl->mStrPath.c_str());
}