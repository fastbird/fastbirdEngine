#pragma once
#include <Engine/IConsole.h>
namespace fastbird
{
	class UICommands : public ICVarListener
	{
	public:
		UICommands();
		~UICommands();

		virtual bool OnChangeCVar(CVar* pCVar);

	private:
		std::vector<CVar*> mCVars;
		std::vector<ConsoleCommand*> mCommands;
	};
}