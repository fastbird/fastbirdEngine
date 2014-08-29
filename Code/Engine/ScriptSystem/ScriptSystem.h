#pragma once
#include <Engine/IScriptSystem.h>

struct lua_State;
namespace fastbird
{
	class ScriptSystem : public IScriptSystem
	{
	public:
		ScriptSystem();
		virtual ~ScriptSystem();

		virtual lua_State* GetLuaState() const;
		virtual bool RunScript(const char* filename);
		virtual bool ExecuteLua(const std::string& chunk);

		virtual std::string GetStringVariable(const char* name, 
			const std::string& def);
		virtual int GetIntVariable(const char* name, int def = 0);
		virtual float GetRealVariable(const char* name, float def = 0);
		

	protected:
		void InitLua();
		void CloseLua();
		

	private:
		lua_State* mLuaState;

	};
}