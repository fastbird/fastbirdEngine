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
#include "StdOutRedirect.h"
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#define FB_READ_FD 0
#define FB_WRITE_FD 1

namespace fb
{
	StdOutRedirect::StdOutRedirect(int bufferSize)
	{
		if (_pipe(mStdOutPipe, bufferSize, O_TEXT) != 0)
		{
			//treat error eventually
			assert(0);
		}
		mStdOut = _dup(_fileno(stdout));
	}

	StdOutRedirect::~StdOutRedirect()
	{
		Stop();
		_close(mStdOut);
		_close(mStdOutPipe[FB_WRITE_FD]);
		_close(mStdOutPipe[FB_READ_FD]);
	}

	int StdOutRedirect::Start()
	{
		fflush(stdout);
		if (_dup2(mStdOutPipe[FB_WRITE_FD], _fileno(stdout)) != 0)
			return -1;

		std::ios::sync_with_stdio();
		setvbuf(stdout, NULL, _IONBF, 0); // absolutely needed
		return 0;
	}

	int StdOutRedirect::Stop()
	{
		if (_dup2(mStdOut, _fileno(stdout)) != 0)
		{
			return -1;
		}
		std::ios::sync_with_stdio();
		return 0;
	}

	int StdOutRedirect::GetBuffer(char *buffer, int size)
	{
		// need non-blocking method;
		int nOutRead = _read(mStdOutPipe[FB_READ_FD], buffer, size);
		buffer[nOutRead] = '\0';
		return nOutRead;
	}
}