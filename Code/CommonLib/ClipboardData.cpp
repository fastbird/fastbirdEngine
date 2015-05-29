#include <CommonLib/StdAfx.h>
#include <CommonLib/ClipboardData.h>
namespace fastbird
{
	std::string GetClipboardDataAsString(HWND hwnd)
	{
		if (OpenClipboard(hwnd))
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
					auto data = GlobalLock(handle);
					std::string str((const char*)data);
					GlobalUnlock(handle);
					CloseClipboard();
					return str;
				}
			}
			CloseClipboard();
		}
		return std::string();
	}

	void SetClipboardStringData(HWND hwnd, const char* data){
		if (OpenClipboard(hwnd))
		{
			EmptyClipboard();

			auto buf = GlobalAlloc(GMEM_MOVEABLE, strlen(data) + 1);
			if (buf == NULL){
				CloseClipboard();
				return;
			}
			auto dest = GlobalLock(buf);
			memcpy(dest, data, strlen(data) + 1);
			GlobalUnlock(buf);
			SetClipboardData(CF_TEXT, buf);
			CloseClipboard();
		}
	}
}