#include <Engine/StdAfx.h>
#include <Engine/Misc/EngineCommand.h>
#include <Engine/IConsole.h>
namespace fastbird
{

EngineCommand::EngineCommand()
{
	REGISTER_CVAR(UI_Debug, 0, CVAR_CATEGORY_CLIENT, "UI debug");
	REGISTER_CVAR(r_noObjectConstants, 0, CVAR_CATEGORY_CLIENT, "do not update object constans buffer");
	REGISTER_CVAR(e_profile, 1, CVAR_CATEGORY_CLIENT, "enable profiler");
}
EngineCommand::~EngineCommand()
{
	for each (auto p in mCVars)
	{
		delete p;
	}
}
}