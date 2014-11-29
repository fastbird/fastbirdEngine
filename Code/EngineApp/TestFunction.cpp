#include "StdAfx.h"

int TestFunction()
{
	int count = 0;
	std::vector<int*> pointers;
	float elapsedTime = 0;
	LARGE_INTEGER start, end, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	try
	{
		while(1)
		{
			int* p = FB_NEW(int);
			if (p==0)
			{
				size_t size = pointers.size();
				for(size_t i=0; i<size; i++)
				{
					FB_SAFE_DEL(pointers[i]);
				}
				pointers.clear();
				break;
			}
			pointers.push_back(p);
			count++;
		}
	}
	catch(...)
	{
		size_t size = pointers.size();
		for(size_t i=0; i<size; i++)
		{
			FB_SAFE_DEL(pointers[i]);
		}
		pointers.clear();
	}

	QueryPerformanceCounter(&end);
	elapsedTime = (end.QuadPart - start.QuadPart) / (float)freq.QuadPart;
	char msg[255];
	sprintf_s(msg, 255, "takes %f\n", elapsedTime);
	OutputDebugString(msg);
	return 0;
}