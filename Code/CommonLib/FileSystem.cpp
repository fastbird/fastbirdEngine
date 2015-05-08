#include <CommonLib/StdAfx.h>
#include <CommonLib/FileSystem.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
	std::string FileSystem::mRootAbs;

	void FileSystem::Initialize()
	{
		char buf[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buf);
		char absbuf[MAX_PATH];
		_fullpath(absbuf, buf, MAX_PATH);
		char unified[MAX_PATH];
		UnifyFilepath(unified, absbuf);
		mRootAbs = unified;
	}
	void FileSystem::Finalize()
	{

	}

	int FileSystem::CompareLastFileWrite(const char* filepath1, const char* filepath2)
	{
		HANDLE file1 = CreateFile(filepath1, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file1 == INVALID_HANDLE_VALUE)
		{
			return 2;
		}

		HANDLE file2 = CreateFile(filepath2, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file2 == INVALID_HANDLE_VALUE)
		{
			CloseHandle(file1);
			return 3;
		}

		FILETIME writeTime1;
		if (!GetFileTime(file1, 0, 0, &writeTime1))
		{
			CloseHandle(file1);
			CloseHandle(file2);
			return 4;
		}
		FILETIME writeTime2;
		if (!GetFileTime(file2, 0, 0, &writeTime2))
		{
			CloseHandle(file1);
			CloseHandle(file2);
			return 5;
		}
		
		CloseHandle(file1);
		CloseHandle(file2);
		return CompareFileTime(&writeTime1, &writeTime2);


	}

	BinaryData FileSystem::ReadBinaryFile(const char* filepath, std::streamoff& outLength)
	{
		std::ifstream is(filepath, std::ios_base::binary);
		if (is)
		{
			is.seekg(0, is.end);
			outLength = is.tellg();
			is.seekg(0, is.beg);

			BinaryData buffer = new char[(unsigned int)outLength];
			is.read(buffer, outLength);
			if (!is)
			{
				Error("FileSystem: Only %u could be read for the file (%s)", is.gcount(), filepath);
			}
			is.close();
			return buffer;
		}
		else
			return 0;
		
	}
	
	void FileSystem::FinishBinaryFile(BinaryData data)
	{
		delete[] data;
	}

	void FileSystem::SaveBinaryFile(const char* filepath, BinaryData data, size_t length)
	{
		if (!data || length == 0 || filepath==0)
			return;

		if (!TestSecurity(filepath))
		{
			Error("FileSystem: SaveBinaryFile to %s has security violation.", filepath);
			return;
		}

		std::ofstream ofs(filepath, std::ios_base::binary | std::ios_base::trunc);
		if (!ofs)
		{
			Error("FileSystem: Cannot open a file(%s) for writing.", filepath);
		}
		ofs.write(data, length);
	}

	bool FileSystem::TestSecurity(const char* filepath)
	{
		char absbuf[MAX_PATH];
		_fullpath(absbuf, filepath, MAX_PATH);
		char unified[MAX_PATH];
		UnifyFilepath(unified, absbuf);
		if (strstr(unified, mRootAbs.c_str()))
			return true;

		return false;
	}
	bool FileSystem::IsFileExisting(const char* filepath)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE handle = FindFirstFile(filepath, &FindFileData);
		bool found = handle != INVALID_HANDLE_VALUE;
		if (found)
		{
			FindClose(handle);
		}
		return found;
	}

}