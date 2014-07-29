#pragma once

namespace fastbird
{
	class ToolConn
	{
	public:
		enum TOOL_CONN_MSG
		{
			TOOL_CONN_MSG_CAMERA_POS, // float x, float y, float z
			TOOL_CONN_MSG_DWORD = 0xffffffff
		};
		static void Prepare();

		static bool bPrepared;
		static HANDLE hMapFileToEditor, hMapFileToEngine;
		static const char *pToEditor, *pToEngine, *pToEditorEnd, *pToEngineEnd;
		static char *pToEditorWritePos, *pToEditorReadPos;
		static char *pToEngineWritePos, *pToEngineReadPos;

		// for engine
		static void WriteToEditor(TOOL_CONN_MSG msg, const char* pData, size_t len);
		static size_t ReadFromEditor(TOOL_CONN_MSG& msg, char* pdata, size_t maxLen); // return bytes read

		// for tool
		static void WriteToEngine(TOOL_CONN_MSG msg, const char* pData, size_t len);
		static size_t ReadFromEngine(TOOL_CONN_MSG& msg, char* pdata, size_t maxLen); // return bytes read
	};
}