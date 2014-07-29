#include <Engine/StdAfx.h>
#include <Engine/Misc/ToolConn.h>

using namespace fastbird;

#define BUF_SIZE 1024

bool ToolConn::bPrepared = false;
HANDLE ToolConn::hMapFileToEditor = NULL;
HANDLE ToolConn::hMapFileToEngine = NULL;
const char* ToolConn::pToEditor = 0;
const char* ToolConn::pToEngine = 0;
const char* ToolConn::pToEditorEnd = 0;
const char* ToolConn::pToEngineEnd = 0;

char* ToolConn::pToEditorWritePos = 0;
char* ToolConn::pToEditorReadPos = 0;
char* ToolConn::pToEngineWritePos = 0;
char* ToolConn::pToEngineReadPos = 0;

void ToolConn::Prepare()
{
	// to Editor
	if (hMapFileToEditor == NULL)
	{
		hMapFileToEditor = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 
			BUF_SIZE, TEXT("Global\\FastbirdToEditor"));

		if (hMapFileToEditor == NULL)
		{
			printf(TEXT("Could not create 'ToEditor' file mapping object(%d). \n"), GetLastError());
			return;
		}
		pToEditor = (const char*)MapViewOfFile(hMapFileToEditor, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
		pToEditorEnd = pToEditor+BUF_SIZE;
		pToEditorReadPos = pToEditorWritePos = (char*)pToEditor;
	}

	// to Engine
	if (hMapFileToEngine == NULL)
	{
		hMapFileToEngine = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
			BUF_SIZE, TEXT("Global\\FastbirdToEngine"));

		if (hMapFileToEngine == NULL)
		{
			printf(TEXT("Could not create 'ToEngine' file mapping object(%d). \n"), GetLastError());
			return;
		}
		pToEngine = (const char*)MapViewOfFile(hMapFileToEngine, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
		pToEngineEnd = pToEngine+BUF_SIZE;
		pToEngineReadPos = pToEngineWritePos = (char*)pToEngine;
	}
	bPrepared = true;
}

// for Engine
void ToolConn::WriteToEditor(TOOL_CONN_MSG msg, const char* pData, size_t len)
{
	if (pToEditorWritePos+len > pToEditorEnd)
	{
		assert(pToEditorReadPos!=0 && "buffer is not enough");
		pToEditorWritePos = (char*)pToEditor;
	}
	memcpy(pToEditorWritePos, &msg, sizeof(TOOL_CONN_MSG));
	memcpy(pToEditorWritePos + sizeof(TOOL_CONN_MSG), pData, len);
}
size_t ToolConn::ReadFromEditor(TOOL_CONN_MSG& msg, char* pdata, size_t maxLen) // return bytes read
{
	assert(0 && "Not implemented.");
}

// for Editor
void ToolConn::WriteToEngine(TOOL_CONN_MSG msg, const char* pData, size_t len)
{
}
size_t ToolConn::ReadFromEngine(TOOL_CONN_MSG& msg, char* pdata, size_t maxLen) // return bytes read
{

}