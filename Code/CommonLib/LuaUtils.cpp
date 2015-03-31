#include <CommonLib/StdAfx.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/LuaObject.h>
#include <CommonLib/FileSystem.h>
namespace fastbird
{
	const char* GetCWD()
	{
		return FileSystem::GetCWD();
	}

	void CallLuaFunction(lua_State* L, const char* func, const char* sig, ...)
	{
		va_list vl;
		int narg, nres;

		va_start(vl, sig);
		lua_getglobal(L, func);

		// push args
		for (narg = 0; *sig; narg++)
		{
			luaL_checkstack(L, 1, "too many arguments");
			switch (*sig++)
			{
			case 'd':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'i':
				lua_pushinteger(L, va_arg(vl, int));
				break;
			case 'u':
				lua_pushunsigned(L, va_arg(vl, unsigned));
				break;
			case 's':
				lua_pushstring(L, va_arg(vl, char*));
				break;

			case '>':
				goto endargs;

			default:
				fastbird::Error("invalid arg(%c)", *(sig - 1));
			}
		}
	endargs:

		nres = strlen(sig);

		if (lua_pcall(L, narg, nres, 0) != 0)
		{
			fastbird::Error("error calling '%s' : %s", func, lua_tostring(L, -1));
			return;
		}

		// retrieve results
		nres = -nres;
		while (*sig)
		{
			switch (*sig++)
			{
			case 'd':
			{
						int isnum;
						double n = lua_tonumberx(L, nres, &isnum);
						if (!isnum)
							fastbird::Error("wrong result type");
						*va_arg(vl, double*) = n;
						break;
			}

			case 'i':
			{
						int isnum;
						int n = lua_tointegerx(L, nres, &isnum);
						if (!isnum)
							fastbird::Error("wrong result type");
						*va_arg(vl, int*) = n;
						break;
			}

			case 's':
			{
						const char* s = lua_tostring(L, nres);
						if (s == NULL)
							fastbird::Error("wrong result type");
						*va_arg(vl, const char**) = s;
						break;
			}

			case'b':
			{
					   bool b = lua_toboolean(L, nres)!=0;
					   *va_arg(vl, bool*) = b;
					   break;
			}

			case 'u':
			{
						unsigned u = lua_tounsigned(L, nres);
						*va_arg(vl, unsigned*) = u;
						break;
			}

			default:
				fastbird::Error("invalid option(%c)", *(sig - 1));
			}
			nres++;
		}

		va_end(vl);
	}

	bool CheckLuaGlobalExist(lua_State* L, const char* name)
	{
		LUA_STACK_WATCHER watcher(L, "bool CheckLuaGlobalExist(lua_State* L, const char* name)");
		lua_getglobal(L, name);
		bool exist = !lua_isnil(L, -1);
		lua_pop(L, 1);
		return exist;
	}

	void PrintLuaDebugInfo(lua_State* L, int level)
	{
		luaL_traceback(L, L, 0, 0);
		Error(lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_Debug ar;
		if (lua_getstack(L, level, &ar))
		{
			int i = 1;
			while (true)
			{
				const char* name = lua_getlocal(L, &ar, i++);
				if (!name)
					break;
				Error("[local variable] name = %s, value = %s \n", name, GetLuaValueAsString(L, -1).c_str());
				lua_pop(L, 1);
			}
		}
	}

	std::string GetLuaValueAsString(lua_State* L, int idx)
	{
		switch (lua_type(L, idx))
		{
		case LUA_TBOOLEAN:
		{
							 bool b = lua_toboolean(L, idx) != 0;
							 return StringConverter::toString(b);
							 break;
		}
		case LUA_TFUNCTION:
		case LUA_TLIGHTUSERDATA:
		case LUA_TNIL:
		case LUA_TNONE:
		case LUA_TTHREAD:
		case LUA_TUSERDATA:
		{
						  return lua_typename(L, idx);
							  break;
		}

		case LUA_TNUMBER:
		case LUA_TSTRING:
		{
							return lua_tostring(L, idx);
							break;
		}
		case LUA_TTABLE:
		{
						   LuaObject t(L, idx);
						   auto it  = t.GetTableIterator();
						   LuaTableIterator::KeyValue kv;
						   std::string ret;
						   bool first = true;
						   while (it.GetNext(kv))
						   {
							   if (!first)
							   {
								   ret += "\n";
							   }
							   kv.first.PushToStack();
							   ret += GetLuaValueAsString(L, -1);
							   lua_pop(L, 1);
							   ret += " : ";
							   kv.second.PushToStack();
							   ret += GetLuaValueAsString(L, -1);
							   lua_pop(L, 1);
							   first = false;
						   }
						   return ret;
						   break;
		}
		default:
		{
				   assert(0);
		}
		
		}
		return std::string();
	}

	bool GetLuaVarAsBoolean(lua_State* L, const char* varName)
	{
		LUA_STACK_CLIPPER w(L);
		lua_getglobal(L, varName);
		return lua_toboolean(L, -1)!=0;
	}

	void SetLuaVar(lua_State* L, const char* varName, bool value)
	{
		LUA_STACK_WATCHER w(L, "void SetLuaVar(lua_State* L, const char* varName, bool value)");
		lua_pushboolean(L, value);
		lua_setglobal(L, varName);
	}
}

