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
		virtual bool LoadConfig(const char* filename) = 0;
		virtual bool ExecuteLua(const std::string& chunk) = 0;
		virtual std::string GetStringVariable(const char* name, 
			const std::string& def =std::string()) = 0;
		virtual int GetIntVariable(const char* name, int def = 0) = 0;
		virtual float GetRealVariable(const char* name, float def = 0) = 0;
		virtual Vec2I GetVec2IVariable(const char* name, Vec2I def = Vec2I::ZERO) = 0;

		virtual void LockLua() = 0;
		virtual void UnlockLua() = 0;
	};

	
}