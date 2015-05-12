#include <CommonLib/StdAfx.h>
#include <CommonLib/StackTracer.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
namespace fastbird{

StackTracer StackTracer::sStackTracer;
std::string StackTracer::sLastMsg;
const int CALLSTACK_DEPTH = 20;
#define CODE_DESCR(code) CodeDescMap::value_type(code, #code)


// static functions
int StackTracer::Filter(unsigned int code, LPEXCEPTION_POINTERS e){
	return sStackTracer.HandleException(e);
}
const char* StackTracer::GetExceptionMsg(){
	std::ostringstream  m_ostringstream;

	// Exception Code
	CodeDescMap::iterator itc = sStackTracer.mCodeDesc.find(sStackTracer.mExceptionCode);

	if (itc != sStackTracer.mCodeDesc.end())
		m_ostringstream << "Exception: " << itc->second << "\r\n";
	else
		m_ostringstream << "Unknown Exception...\r\n";

	m_ostringstream << "------------------------------------------------------------------\r\n";

	// Call Stack
	std::vector<FunctionCall>::iterator itbegin = sStackTracer.mCallStack.begin();
	std::vector<FunctionCall>::iterator itend = sStackTracer.mCallStack.end();
	std::vector<FunctionCall>::iterator it;
	for (it = itbegin; it < itend; it++)
	{
		std::string strFunctionName = it->FunctionName.empty() ? "UnkownFunction" : it->FunctionName;
		std::string strFileName = it->FileName.empty() ? "UnkownFile" : it->FileName;

		m_ostringstream << "Function : " << strFunctionName << "\r\n";
		m_ostringstream << "    [Source File : " << strFileName << "]\r\n";
		m_ostringstream << "    [Source Line : " << it->LineNumber << "]\r\n";
	}
	sLastMsg = m_ostringstream.str();
	return sLastMsg.c_str();
}
unsigned StackTracer::GetECode(){
	return sStackTracer.mExceptionCode;
}

std::vector<FunctionCall> StackTracer::GetExceptionCallStack(){
	return sStackTracer.mCallStack;
}


// private
StackTracer::StackTracer(void)
	:mExceptionCode(0)
{	
	// Get machine type
	mMachineType = 0;
	char* szProcessor = ::getenv("PROCESSOR_ARCHITECTURE");
	if (szProcessor)
	{
		if ((!strcmp("EM64T", szProcessor)) || !strcmp("AMD64", szProcessor))
		{
			mMachineType = IMAGE_FILE_MACHINE_AMD64;
		}
		else if (!strcmp("x86", szProcessor))
		{
			mMachineType = IMAGE_FILE_MACHINE_I386;
		}
	}

	// Exception code description
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_ACCESS_VIOLATION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_DATATYPE_MISALIGNMENT));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_BREAKPOINT));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_SINGLE_STEP));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_ARRAY_BOUNDS_EXCEEDED));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_DENORMAL_OPERAND));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_DIVIDE_BY_ZERO));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_INEXACT_RESULT));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_INVALID_OPERATION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_OVERFLOW));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_STACK_CHECK));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_FLT_UNDERFLOW));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_INT_DIVIDE_BY_ZERO));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_INT_OVERFLOW));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_PRIV_INSTRUCTION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_IN_PAGE_ERROR));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_ILLEGAL_INSTRUCTION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_NONCONTINUABLE_EXCEPTION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_STACK_OVERFLOW));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_INVALID_DISPOSITION));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_GUARD_PAGE));
	mCodeDesc.insert(CODE_DESCR(EXCEPTION_INVALID_HANDLE));
	//m_mapCodeDesc.insert(CODE_DESCR(EXCEPTION_POSSIBLE_DEADLOCK));      
	// Any other exception code???
}
StackTracer::~StackTracer(void){

}

LONG __stdcall StackTracer::HandleException(LPEXCEPTION_POINTERS e){
	mExceptionCode = e->ExceptionRecord->ExceptionCode;
	mCallStack.clear();

	HANDLE hProcess = INVALID_HANDLE_VALUE;

	// Initializes the symbol handler
	if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
	{
		SymCleanup(hProcess);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// Work through the call stack upwards.
	TraceCallStack(e->ContextRecord);

	// ...
	SymCleanup(hProcess);

	return(EXCEPTION_EXECUTE_HANDLER);
}

// Work through the stack upwards to get the entire call stack
void StackTracer::TraceCallStack(CONTEXT* pContext){
	// Initialize stack frame
	STACKFRAME64 sf;
	memset(&sf, 0, sizeof(STACKFRAME));

#if defined(_WIN64)
	sf.AddrPC.Offset = pContext->Rip;
	sf.AddrStack.Offset = pContext->Rsp;
	sf.AddrFrame.Offset = pContext->Rbp;
#elif defined(WIN32)
	sf.AddrPC.Offset = pContext->Eip;
	sf.AddrStack.Offset = pContext->Esp;
	sf.AddrFrame.Offset = pContext->Ebp;
#endif
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	if (0 == mMachineType)
		return;

	// Walk through the stack frames.
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	while (StackWalk64(mMachineType, hProcess, hThread, &sf, pContext, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
	{
		if (sf.AddrFrame.Offset == 0 || mCallStack.size() >= CALLSTACK_DEPTH)
			break;

		// 1. Get function name at the address
		const int nBuffSize = (sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64);
		ULONG64 symbolBuffer[nBuffSize];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;

		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		FunctionCall curCall = { "", "", 0 };

		DWORD64 dwSymDisplacement = 0;
		if (SymFromAddr(hProcess, sf.AddrPC.Offset, &dwSymDisplacement, pSymbol))
		{
			curCall.FunctionName = std::string(pSymbol->Name);
		}

		//2. get line and file name at the address
		IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
		DWORD dwLineDisplacement = 0;

		if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo))
		{
			curCall.FileName = std::string(lineInfo.FileName);
			curCall.LineNumber = lineInfo.LineNumber;
		}

		// Call stack stored
		mCallStack.push_back(curCall);
	}
}

}
