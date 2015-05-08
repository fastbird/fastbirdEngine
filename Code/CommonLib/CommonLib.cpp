#include <CommonLib/StdAfx.h>

namespace fastbird
{
	HMODULE LoadFBLibrary(const char* name)
	{
		if (!name)
			return 0;

		const char* dotPos = strchr(name, '.');
		if (!dotPos)
		{
			fastbird::Error("LoadFBLibrary(%s) failed. Invalid param.", name);
		}

		char buf[MAX_PATH] = { 0 };
		strncpy(buf, name, dotPos - name);

#ifdef _DEBUG
		strcat(buf, "_Debug.dll");
#else
		strcat(buf, "_Release.dll");
#endif

		HMODULE module = LoadLibrary(buf);
		if (!module)
		{
			module = LoadLibrary(name);
		}
		if (!module)
		{
			fastbird::Error("LoadFBLibrary(%s) failed", name);
		}
		return module;
	}

	void FreeFBLibrary(HMODULE module)
	{
		if (module)
			FreeLibrary(module);
	}
}