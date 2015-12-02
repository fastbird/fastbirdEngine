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

#pragma once
#include <memory>
#include "FBCommonHeaders/String.h"
#include "FBCommonHeaders/Types.h"
namespace fb{
	class DirectoryIterator;
	typedef std::shared_ptr<DirectoryIterator> DirectoryIteratorPtr;
	typedef std::weak_ptr<DirectoryIterator> DirectoryIteratorWeakPtr;

	/** Read file in a directory.
	Every time you call \b GetNextFilePath() you will get a file name in the
	directory. Order is not defined. You will get empty string _T("") if you get
	all file names. This function is not thread safe.
	*/
	class FB_DLL_FILESYSTEM DirectoryIterator{
		FB_DECLARE_PIMPL_NON_COPYABLE(DirectoryIterator);

	public:
		DirectoryIterator(const char* directoryPath, bool recursive);
		~DirectoryIterator();

		bool IsOpen() const;
		bool HasNext() const;
		/** Get next file path
		Order is undefined.
		\return a file path.
		returned.
		*/
		const char* GetNextFilePath(bool* outIsDirectory);
		const char* GetNextFilePath();
	};
}