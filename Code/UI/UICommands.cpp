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
	gFBEnv->pConsole->RemoveListener(this);
	for (auto p : mCVars)
	{
		gFBEnv->pConsole->UnregisterVariable(p);
		FB_SAFE_DEL(p);
	}

	for (const auto& p : mCommands)
	{
		gFBEnv->pConsole->UnregisterCommand(p);
	}
}

bool UICommands::OnChangeCVar(CVar* pCVar)
{
	return false;
}


typedef int(__cdecl *StartProc)(GlobalEnv* pEnv);
static bool uiEditorInitialized = false;
void StartUIEditor(StringVector& arg)
{
	if (uiEditorInitialized)
	{
		Log("Alreay started!");
		return;
	}
	auto moduleHandle = gFBEnv->pUIManager->GetUIEditorModuleHandle();
	if (!moduleHandle)
	{
		moduleHandle = LoadLibrary("FBUIEditor.dll");;
		gFBEnv->pUIManager->SetUIEditorModuleHandle(moduleHandle);
	}
	if (moduleHandle)
	{		
		StartProc startFunc;
		startFunc = (StartProc)GetProcAddress(moduleHandle, "StartUIEditor");
		if (startFunc)
		{
			gFBEnv->pUIManager->SetUIEditorModuleHandle(moduleHandle);
			startFunc(gFBEnv);
			uiEditorInitialized = true;
		}

	}
}

typedef void(__cdecl *FinalizeProc)();
void KillUIEditor(StringVector& arg)
{
	if (!uiEditorInitialized)
		return;

	auto moduleHandle = gFBEnv->pUIManager->GetUIEditorModuleHandle();
	if (moduleHandle)
	{
		FinalizeProc finalizeFunc;
		finalizeFunc = (FinalizeProc)GetProcAddress(moduleHandle, "KillUIEditor");
		if (finalizeFunc)
			finalizeFunc();

		//FreeLibrary(gFBEnv->pUIManager->GetUIEditorModuleHandle());
		//gFBEnv->pUIManager->SetUIEditorModuleHandle(0);
	}
	uiEditorInitialized = false;
}