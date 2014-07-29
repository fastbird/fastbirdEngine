#pragma once
#include <CommonLib/SmartPtr.h>
struct lua_State;

namespace fastbird
{
	class IScriptSystem : public ReferenceCounter
	{
	public:
		virtual lua_State* GetLuaState() const = 0;
		virtual bool RunScript(const char* filename) = 0;
		virtual bool ExecuteLua(const std::string& chunk) = 0;
		virtual std::string GetStringVariable(const char* name, 
			const std::string& def =std::string()) = 0;
	};

	
}