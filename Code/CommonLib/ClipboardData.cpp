#include <CommonLib/StdAfx.h>
#include <CommonLib/ClipboardData.h>
namespace fastbird
{
	std::string GetClipbardDataAsString()
	{
		if (OpenClipboard(NULL))
		{
			unsigned t = EnumClipboardFormats(0);
			while (t != CF_TEXT && t!=0)
			{
				t = EnumClipboardFormats(t);
			}
			if (t == CF_TEXT)
			{
				auto handle = GetClipboardData(CF_TEXT);
				if (handle)
				{
					GlobalLock(handle);
					std::string str((const char*)handle);
					GlobalUnlock(handle);
					CloseClipboard();
					return str;
				}
			}
			CloseClipboard();
		}
		return std::string();
	}
}