#include <UI/StdAfx.h>
#include <UI/UICommands.h>
#include <UI/UIManager.h>
using namespace fastbird;

static void StartUIEditor(StringVector& arg);
static void KillUIEditor(StringVector& arg);

static ConsoleCommand ccStartUIEditor("StartUIEditor", StartUIEditor, "Start ui editor");
static ConsoleCommand ccKillUIEditor("KillUIEditor", KillUIEditor, "Kill ui editor");

UICommands::UICommands()
{
	REGISTER_CC(&ccStartUIEditor);
	REGISTER_CC(&ccKillUIEditor);
}

UICommands::~UICommands()
{
}

bool UICommands::OnChangeCVar(CVar* pCVar)
{
	return false;
}


typedef int(__cdecl *StartProc)(GlobalEnv* pEnv, int x, int y);

void StartUIEditor(StringVector& arg)
{
	if (gFBEnv->pUIManager->GetUIEditorModuleHandle())
	{
		Log("Alreay started!");
		return;
	}
	auto moduleHandle = LoadLibrary("FBUIEditor.dll");
	if (moduleHandle)
	{		
		StartProc startFunc;
		startFunc = (StartProc)GetProcAddress(moduleHandle, "StartUIEditor");
		if (startFunc)
		{
			gFBEnv->pUIManager->SetUIEditorModuleHandle(moduleHandle);
			startFunc(gFBEnv, 1620, 0);
		}

	}
}
void KillUIEditor(StringVector& arg)
{
	if (gFBEnv->pUIManager->GetUIEditorModuleHandle())
	{
		FreeLibrary(gFBEnv->pUIManager->GetUIEditorModuleHandle());
		gFBEnv->pUIManager->SetUIEditorModuleHandle(0);
	}
}