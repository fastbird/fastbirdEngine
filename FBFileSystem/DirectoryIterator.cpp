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
#include "DirectoryIterator.h"
#include "FileSystem.h"
#include "FBDataPackLib/fba.h"

namespace fb{
	class DirectoryIterator::Impl{
	public:
		std::string mLastFile;		
		bool mRecursive;
		boost::filesystem::recursive_directory_iterator mRecursiveIterator;
		boost::filesystem::directory_iterator mIterator;

		const pack_datum* pack;
		std::string path;
		std::string pack_parent_path;
		unsigned current_pack_index;

		Impl() 
			: mRecursive(false)
			, current_pack_index(0)
			, pack(0)
		{
		}

		bool IsOpen() const{
			if (!pack) {
				return mIterator != boost::filesystem::directory_iterator();
			}
			else {
				return StartsWith(path, pack->pack_path);
			}
		}

		bool HasNext(){
			if (!pack) {
				if (mRecursive)
					return mRecursiveIterator != boost::filesystem::recursive_directory_iterator();
				return mIterator != boost::filesystem::directory_iterator();
			}
			else {				
				for (unsigned i = current_pack_index; i < pack->headers.size(); ++i) {
					auto& h = pack->headers[i];
					// pack path = Data/actors.fba
					// pack_parent_path = Data/
					// file_path = actors/something.lua
					// output_path = pack_parent_path + file_path = Data/actors/something.lua
					auto output_path = pack_parent_path + h.path;
					if (StartsWith(output_path.c_str(), path)) {
						if (!mRecursive) {
							auto file = output_path.substr(path.size());
							if (file.find('/') != std::string::npos) {
								continue;
							}
						}
						return true;
					}					
				}
				return false;
			}
		}

		const char* GetNext(bool* isDirectory){
			if (!HasNext())
				return "";

			if (!pack) {
				boost::filesystem::path path;
				if (mRecursive) {
					auto entity = *mRecursiveIterator;
					mRecursiveIterator++;
					path = entity.path();
				}
				else {
					auto entity = *mIterator;
					mIterator++;
					path = entity.path();
				}

				if (path.empty())
					return "";

				mLastFile = path._tgeneric_string();
				if (isDirectory)
					*isDirectory = boost::filesystem::is_directory(path);
				return mLastFile.c_str();
			}
			else {
				for (unsigned i = current_pack_index; i < pack->headers.size(); ++i) {
					auto& h = pack->headers[i];
					// pack path = Data/actors.fba
					// pack_parent_path = Data/
					// file_path = actors/something.lua
					// output_path = pack_parent_path + file_path = Data/actors/something.lua
					auto output_path = pack_parent_path + h.path;
					if (StartsWith(output_path, path)) {
						if (!mRecursive) {
							auto file = output_path.substr(path.size());
							if (file.find('/') != std::string::npos) {
								continue;
							}
						}
						current_pack_index = i+1;
						mLastFile = output_path;
						if (isDirectory)
							*isDirectory = false;
						return mLastFile.c_str();
					}
				}
				return "";
			}
		}
		
	};

	//---------------------------------------------------------------------------
	DirectoryIterator::DirectoryIterator(const char* directoryPath, bool recursive)
		:mImpl(new Impl)
	{
		if (directoryPath && _tstrlen(directoryPath) != 0){			
			mImpl->mRecursive = recursive;
			if (recursive){
				mImpl->mRecursiveIterator = boost::filesystem::recursive_directory_iterator(directoryPath);
			}
			else{
				mImpl->mIterator = boost::filesystem::directory_iterator(directoryPath);
			}
		}
	}

	DirectoryIterator::DirectoryIterator(const char* directoryPath, bool recursive, const pack_datum* pack_datum)
		: mImpl(new Impl)
	{
		if (!pack_datum) {
			const char* msg = "Do not create Directory Iterator with out pack_datum. Use other construct if you are opening actual directory.";
			Logger::Log(FB_ERROR_LOG_ARG, msg);
			throw std::invalid_argument(msg);
		}
		mImpl->pack = pack_datum;
		mImpl->path = FileSystem::AddEndingSlashIfNot(directoryPath);
		mImpl->pack_parent_path = FileSystem::GetParentPath(pack_datum->pack_path.c_str());
		mImpl->pack_parent_path = FileSystem::AddEndingSlashIfNot(mImpl->pack_parent_path.c_str());
		mImpl->mRecursive = recursive;
	}

	DirectoryIterator::~DirectoryIterator(){
	}

	bool DirectoryIterator::IsOpen() const{
		return mImpl->IsOpen();
	}

	bool DirectoryIterator::HasNext() const{
		return mImpl->HasNext();
	}
	const char* DirectoryIterator::GetNextFilePath(bool* isDirectory){
		return mImpl->GetNext(isDirectory);
	}

	const char* DirectoryIterator::GetNextFilePath(){
		return GetNextFilePath(0);
	}
}