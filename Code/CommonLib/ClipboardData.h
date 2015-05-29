#pragma once

namespace fastbird
{
	std::string GetClipboardDataAsString(HWND hwnd);
	void SetClipboardStringData(HWND hwnd, const char* data);
}