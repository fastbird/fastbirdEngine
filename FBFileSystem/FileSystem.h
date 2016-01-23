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

/**
\file FBFileSystem.h
Handling file operations. Implemented on the top of boost::filesystem
\author Jungwan Byun
\defgroup FBFileSystem
Handling file operations. Implemented on the top of boost::filesystem
*/
#pragma once
#include "FBCommonHeaders/String.h"
#include "DirectoryIterator.h"
#include <memory>
#include <string>
#undef CreateDirectory
#undef CopyFile
namespace fb{
	class DirectoryIterator;
	typedef std::shared_ptr<DirectoryIterator> DirectoryIteratorPtr;
	typedef std::weak_ptr<DirectoryIterator> DirectoryIteratorWeakPtr;

	/** Collection of file manipulators.
	\ingroup FBFileSystem
	*/
	class FB_DLL_FILESYSTEM FileSystem
	{
	public:
		/**Start logging.
		You can call this function several time but only the first call only takes the effect.
		*/
		static void StartLoggingIfNot();
		static void StopLogging();
		enum {
			FB_NO_ERROR = 0,
			RENAME_NO_SOURCE = 1,
			RENAME_DEST_EXISTS = 2,
			FILE_NO_EXISTS = 3,
			SECURITY_NOT_OK = 4,
			COPYFILE_NO_SOURCE=5,
			COPYFILE_DEST_ALREADY_EXISTS = 6,
			COPYFILE_ERROR = 7,
		};
		//---------------------------------------------------------------------------
		// File Operataions
		//---------------------------------------------------------------------------
		static bool Exists(const char* path);
		static bool IsDirectory(const char* path);
		static unsigned GetFileSize(const char* path);
		/** Rename a file.
		@remark  If \a path and \a newpath resolve to the same existing file, no action is taken. 
		Otherwise, if \a newpath resolves to an existing non-directory file, it is removed, 
		while if \a newpath resolves to an existing directory, it is removed if empty on ISO/IEC 9945 
		but is an error on Windows. A symbolic link is itself renamed, rather than the file it 
		resolves to being renamed.
		*/
		static int Rename(const char* path, const char* newpath);
		static int CopyFile(const char* src, const char* dest, bool overwrite, bool supressErrorMsg);
		/** Remove a file.
		@return 'false' if \b path did not exist in the first place, otherwise true.
		*/
		static bool Remove(const char* path);		
		/** If filepath exists, rename it to preserve. 
		\param numKeeping decides how many backup files need to be kept.
		*/
		static void BackupFile(const char* filepath, unsigned numKeeping);
		/** If filepath exists, rename it to preserve. 
		\param numKeeping decides how many backup files need to be kept.
		\param directory directory to move the original file.
		*/
		static void BackupFile(const char* filepath, unsigned numKeeping, const char* directory);
		/** Compares the last modified time
		\return -1, if a is older. 0, if the same. 1, if b is older. 
		If file1 or file2 does not exists, returns FILE_NO_EXISTS.
		*/
		static int CompareFileModifiedTime(const char* file1, const char* file2);
		static bool SecurityOK(const char* filepath);		
		static BinaryData ReadBinaryFile(const char* path, std::streamoff& outLength);
		static void WriteBinaryFile(const char* path, char* data, size_t length);

		//---------------------------------------------------------------------------
		// Directory Operataions
		//---------------------------------------------------------------------------
		static DirectoryIteratorPtr GetDirectoryIterator(const char* filepath, bool recursive);
		/** Create all directires not exists to resolve the \a filepath
		\return true if a new directory was created, otherwise false.
		*/
		static bool CreateDirectory(const char* filepath);

		//---------------------------------------------------------------------------
		// System Folders
		//---------------------------------------------------------------------------
		static void SetApplicationName(const char* name);
		static std::string GetAppDataFolder();
		static std::string GetCurrentDir();

		//---------------------------------------------------------------------------
		// File Path Manifulation
		//---------------------------------------------------------------------------
		/** Replaces the extension of the file
		If \a ext is an empty string, extension will be removed.
		If \a ext is not an empty string and doesn't have \a dot(.), it will be added. */
		static std::string ReplaceExtension(const char* path, const char* ext);
		static std::string ReplaceFilename(const char* path, const char* newFilename);
		/** If \a dot(.) is not in the \a path, empty string will be returned. 
		If the extension is found, '.' is included in the returned string.
		*/
		static const char* GetExtension(const char* path);	
		static const char* GetExtensionWithOutDot(const char* path);
		/** Get filename + extension*/
		static std::string GetFileName(const char* path);
		/** Get file name.
		Extension(from the last dot to end) and path are excluded.		
		*/
		static std::string GetName(const char* path);		
		/** Returns the parent path.
		ex) dir1/dir2/file.exe -> dir1/dir1
		*/
		static std::string GetParentPath(const char* path);
		static std::string GetLastDirectory(const char* path);
		static std::string ConcatPath(const char* path1, const char* path2);
		static std::string UnifyFilepath(const char* path);
		static std::string Absolute(const char* path);
		/** Add '/' if it doesn't exists at the end of the \a directory.
		*/
		static std::string MakrEndingSlashIfNot(const char* directory);
		/** Strips the first directory.
		if input is 'data/object/file.jpg' then it will return 'object/file.jpg'.
		if the first directory is stripped, \a outStripped will be true otherwise false.
		*/
		static std::string StripFirstDirectoryPath(const char* path, bool* outStripped);
	};
}