#pragma once
#include <CommonLib/SmartPtr.h>
namespace fastbird
{
struct CVar;
struct ConsoleCommand;
class EngineCommand : public ReferenceCounter
{
public:
	EngineCommand();
	~EngineCommand();

	int UI_Debug;
	int r_noObjectConstants;
	int e_profile;

private:
	std::vector<CVar*> mCVars;
	std::vector<ConsoleCommand*> mCommands;
};
}