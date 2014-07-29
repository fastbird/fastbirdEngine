#pragma once

namespace fastbird
{
class CLASS_DECLSPEC_UI IFileSelector
{
public:
	virtual void SetTitle(const wchar_t* szTitle) = 0;
	virtual void ListFiles(const char* folder, const char* filter) = 0;
	virtual std::string GetFile() const = 0;
};

}