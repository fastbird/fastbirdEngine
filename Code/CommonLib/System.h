#pragma once
namespace fastbird
{
	int GetNumProcessors();
	HMODULE GetCurrentModule();
	//------------------------------------------------------------------------
	void LogLastError(const char* file, int line, const char* function);
	//------------------------------------------------------------------------
	const char* GetLastErrorString(const char* file, int line, const char* function);

	//------------------------------------------------------------------------
	#define FB_LOG_LAST_ERROR() fastbird::LogLastError(__FILE__, __LINE__, __FUNCTION__)
	#define FB_LAST_ERROR_STRING() fastbird::GetLastErrorString(__FILE__, __LINE__, __FUNCTION__)
	
	//------------------------------------------------------------------------
	const char* FBGetComputerName();
}