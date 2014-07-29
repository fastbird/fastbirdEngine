#pragma once

struct lua_State;
namespace fastbird
{
	class LuaScript
	{
	public:
		LuaScript(const char* filename);
		virtual ~LuaScript();

		static void LuaError(int error);
		bool LoadScriptFile(const char* filename);


	private:
		lua_State* L;
		std::string mFilename;
	};
}