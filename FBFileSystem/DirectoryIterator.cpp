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

namespace fb{
	class DirectoryIterator::Impl{
	public:
		std::string mLastFile;
		bool mRecursive;
		boost::filesystem::recursive_directory_iterator mRecursiveIterator;
		boost::filesystem::directory_iterator mIterator;

		Impl() :mRecursive(false){
		}

		bool IsOpen() const{
			return mIterator != boost::filesystem::directory_iterator();
		}

		bool HasNext(){
			if (mRecursive)
				return mRecursiveIterator != boost::filesystem::recursive_directory_iterator();
			return mIterator != boost::filesystem::directory_iterator();
		}

		const char* GetNext(bool* isDirectory){
			if (!HasNext())
				return "";

			boost::filesystem::path path;
			if (mRecursive){
					auto entity = *mRecursiveIterator;
					mRecursiveIterator++;
					path = entity.path();
			}
			else{
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