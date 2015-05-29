#pragma once
// Reference : http://www.codeproject.com/Articles/41923/Get-the-call-stack-when-an-exception-is-being-caug

/*
//
// Description: This class is used to get the call stack when there is an exception being caught use SEH
//
// Author: Baiyan Huang
// Date: 8/30/2009
//
// Usage:
//		__try
//		{
//			// main functions...
//		}
//		__except(StackTracer::ExceptionFilter(GetExceptionInformation()))
//		{
//			// Your code to show or log the exception msg...
//		}
*/

namespace fastbird
{
	struct FunctionCall
	{
		std::string FunctionName;
		std::string FileName;
		int			LineNumber;
	};

	class StackTracer
	{
		unsigned mExceptionCode;
		std::vector<FunctionCall> mCallStack;
		typedef std::map<DWORD, const char*> CodeDescMap;
		CodeDescMap mCodeDesc;
		unsigned mMachineType;
		static std::string sLastMsg;

	private:
		static StackTracer sStackTracer;

	private:
		StackTracer(void);
		~StackTracer(void);

		// The main function to handle exception
		LONG __stdcall HandleException(LPEXCEPTION_POINTERS e);

		// Work through the stack upwards to get the entire call stack
		void TraceCallStack(CONTEXT* pContext);


	public:
		static int Filter(unsigned int code, LPEXCEPTION_POINTERS e);
		static const char* GetExceptionMsg();
		static unsigned GetECode();
		static std::vector<FunctionCall> GetExceptionCallStack();
		

	};

}