#include <CommonLib/StdAfx.h>
#include <CommonLib/StdOutRedirect.h>
//#include <CommonLib/CommonLib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#define FB_READ_FD 0
#define FB_WRITE_FD 1

namespace fastbird
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