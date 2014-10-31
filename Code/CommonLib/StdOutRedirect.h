#pragma once
namespace fastbird
{
	class StdOutRedirect
	{
	public:
		StdOutRedirect(int bufferSize);
		~StdOutRedirect();

		int Start();
		int Stop();
		int GetBuffer(char *buffer, int size);

	private:
		int mStdOutPipe[2];
		int mStdOut;
	};
}