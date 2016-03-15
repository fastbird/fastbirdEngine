/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "LuaUtils.h"
#include "LuaObject.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBCommonHeaders/RecursiveSpinLock.h"
#include "FBFileSystem/FileSystem.h"
using namespace fb;

// luawapper util

namespace fb
{
	const char* GetCWD() {
		static char buf[MAX_PATH];
#if defined(_PLATFORM_WINDOWS_)
		GetCurrentDirectory(MAX_PATH, buf);
#else
		assert(0 && "Not implemented");
#endif
		return buf;
	}

	static lua_State* sLuaState = 0;
	lua_State* LuaUtils::OpenLuaState(){
		auto L = luaL_newstate();
		luaL_openlibs(L);
		if (sLuaState == 0){
			sLuaState = L;
			auto filepath = "_FBLua.log";
			FileSystem::BackupFile(filepath, 5, "Backup_Log");
			Logger::Init(filepath);
		}
		return L;
	}

	lua_State* LuaUtils::GetLuaState(){
		return sLuaState;
	}

	void LuaUtils::CloseLuaState(lua_State* L){
		if (L == sLuaState || (L == 0 && sLuaState)){
			lua_close(sLuaState);
			sLuaState = 0;
		}
		else if (L){
			lua_close(L);
		}
	}


	void LuaUtils::CallLuaFunction(lua_State* L, const char* func, const char* sig, ...)
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
			case 'f':
				lua_pushnumber(L, (lua_Number)va_arg(vl, float));
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
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid arg(%c)", *(sig - 1)).c_str());
			}
		}
	endargs:

		nres = strlen(sig);

		if (lua_pcall(L, narg, nres, 0) != 0)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Error calling '%s':'%s'", func, lua_tostring(L, -1)).c_str());			
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
							Logger::Log(FB_ERROR_LOG_ARG, "Wrong result type.");
						*va_arg(vl, double*) = n;
						break;
			}
			case 'f':
			{
				int isnum;
				float n = (float)lua_tonumberx(L, nres, &isnum);
				if (!isnum)
					Logger::Log(FB_ERROR_LOG_ARG, "Wrong result type.");
				*va_arg(vl, float*) = n;				
				break;
			}
			case 'i':
			{
						int isnum;
						int n = lua_tointegerx(L, nres, &isnum);
						if (!isnum)
							Logger::Log(FB_ERROR_LOG_ARG, "Wrong result type.");
						*va_arg(vl, int*) = n;
						break;
			}

			case 's':
			{
						const char* s = lua_tostring(L, nres);
						if (s == NULL)
							Logger::Log(FB_ERROR_LOG_ARG, "Wrong result type.");
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
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid option(%c)", *(sig - 1)).c_str());				
			}
			nres++;
		}

		va_end(vl);
	}

	bool LuaUtils::CheckLuaGlobalExist(lua_State* L, const char* name)
	{
		LUA_STACK_WATCHER watcher(L, "bool CheckLuaGlobalExist(lua_State* L, const char* name)");
		lua_getglobal(L, name);
		bool exist = !lua_isnil(L, -1);
		lua_pop(L, 1);
		return exist;
	}

	void LuaUtils::PrintLuaErrorString(lua_State* L, const char* luaString)
	{
		if (!luaString)
			return;
		std::regex e(":(\\d+):");
		char buf[1024];
		sprintf_s(buf, "\n\n*****\n%s/%s (%s)\n", GetCWD(), std::regex_replace(luaString, e, "($1):").c_str(), "error");
		Logger::Log(FB_ERROR_LOG_ARG, buf);	
		Logger::Output(buf);
		PrintLuaDebugInfo(L, 0);
	}

	void LuaUtils::PrintLuaDebugInfo(lua_State* L, int level)
	{
		luaL_traceback(L, L, 0, 0);
		Logger::Log(FB_ERROR_LOG_ARG, lua_tostring(L, -1));
		lua_pop(L, 1);

	/*	lua_Debug ar;
		if (lua_getstack(L, level, &ar))
		{
			int i = 1;
			while (true)
			{
				const char* name = lua_getlocal(L, &ar, i++);
				if (!name)
					break;
				Error("[local variable] name = %s, value = %s", name, GetLuaValueAsString(L, -1).c_str());
				lua_pop(L, 1);
			}
		}*/
	}

	std::string LuaUtils::GetLuaValueAsString(lua_State* L, int idx)
	{
		switch (lua_type(L, idx))
		{
		case LUA_TBOOLEAN:
		{
			bool val = lua_toboolean(L, idx) != 0;
			if (val)
				return "true";
			else
				return "false";
					
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

	std::string LuaUtils::GetLuaVarAsString(lua_State* L, const char* varName, const char* luaFile)
	{
		auto luaObj = GetLuaVar(L, varName, luaFile);
		if (luaObj.IsValid())
		{
			return luaObj.GetString();
		}
		return std::string();
	}

	bool LuaUtils::GetLuaVarAsBoolean(lua_State* L, const char* varName)
	{
		LUA_STACK_CLIPPER w(L);
		lua_getglobal(L, varName);
		return lua_toboolean(L, -1)!=0;
	}

	Vec2ITuple LuaUtils::GetLuaVarAsVec2I(const char* varname){
		return GetLuaVarAsVec2I(sLuaState, varname);
	}

	Vec2ITuple LuaUtils::GetLuaVarAsVec2I(lua_State* L, const char* varname)
	{
		LUA_STACK_CLIPPER w(L);
		lua_getglobal(L, varname);
		assert(lua_istable(L, -1));
		Vec2ITuple ret = luaU_check<Vec2ITuple>(L, -1);

		lua_pop(L, 1);
		return ret;
	}

	float LuaUtils::GetLuaVarAsFloat(lua_State* L, const char* varName){
		LUA_STACK_CLIPPER w(L);
		lua_getglobal(L, varName);
		return (float)luaL_checknumber(L, -1);
	}

	unsigned LuaUtils::GetLuaVarAsUnsigned(lua_State* L, const char* varName){
		LUA_STACK_CLIPPER w(L);
		lua_getglobal(L, varName);
		return (unsigned)luaL_checknumber(L, -1);
	}

	void LuaUtils::SetLuaVar(lua_State* L, const char* varName, bool value)
	{
		LUA_STACK_WATCHER w(L, "void SetLuaVar(lua_State* L, const char* varName, bool value)");
		lua_pushboolean(L, value);
		lua_setglobal(L, varName);
	}

	bool LuaUtils::ExecuteLua(const char* chunk){
		return ExecuteLua(sLuaState, chunk);
	}

	bool LuaUtils::ExecuteLua(lua_State* L, const char* chunk){
		if (!ValidCStringLength(chunk))
			return false;
		if (!L){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid param.");
		}

		int error;
		error = luaL_loadbuffer(L, chunk, strlen(chunk), "line") || lua_pcall(L, 0, 0, 0);
		if (error)
		{
			const char* errorString = lua_tostring(L, -1);
			lua_pop(L, 1);
			PrintLuaErrorString(L, errorString);			
		}
		return true;
	}

	bool LuaUtils::DoFile(const char* filepath){
		if (sLuaState){
			return DoFile(sLuaState, filepath);
		}
		Logger::Log(FB_ERROR_LOG_ARG, "Main lua state is not prepared.");
		return true; // true when error.
	}

	bool LuaUtils::DoFile(lua_State* L, const char* filepath){
		bool error = luaL_dofile(L, filepath);
		if (error)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Running script(%s) is failed", filepath).c_str());			
			PrintLuaErrorString(L, tostring(L, -1));			
		}
		return error;
	}

	bool LuaUtils::LoadConfig(const char* filename){
		// load the chunk and then change the value of its first upvalue
		auto err = luaL_loadfile(sLuaState, filename); // func.
		if (err){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot load lua file(%s)", filename).c_str());
			return false;
		}
		lua_createtable(sLuaState, 0, 0); // func. {}
		const char* upvaluName = lua_setupvalue(sLuaState, -2, 1); // func.
		lua_pushvalue(sLuaState, -1); //func. func.

		// now the function has empty _ENV
		int error = lua_pcall(sLuaState, 0, 0, 0); // func.
		if (error)
		{
			// func. error
			const char* errorString = lua_tostring(sLuaState, -1);
			lua_pop(sLuaState, 1);
			PrintLuaErrorString(sLuaState, errorString);			
			return false;
		}

		const char* name = lua_getupvalue(sLuaState, -1, 1); // func. _ENV
		LuaObject env(sLuaState, -1);
		auto it = env.GetTableIterator();
		LuaTableIterator::KeyValue kv;
		while (it.GetNext(kv)){
			if (!kv.first.IsString())
				continue;
			// for security.
			if (kv.second.HasFunction())
				continue;

			kv.second.PushToStack();
			lua_setglobal(sLuaState, kv.first.GetString().c_str());
		}
		lua_pop(sLuaState, 2); // pop func. and _ENV

		return true;
	}

	int LuaUtils::Traceback(lua_State *L) {
		const char *msg = lua_tostring(L, 1);
		if (msg)
			luaL_traceback(L, L, msg, 1);
		//else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
		//	if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
		//		lua_pushliteral(L, "(no error message)");
		//}
		return 1;
	}

	void LuaUtils::pushnil(){
		lua_pushnil(sLuaState);
	}

	void LuaUtils::pushnil(lua_State* L){
		lua_pushnil(L);
	}

	const char* LuaUtils::pushstring(const char* str){
		return lua_pushstring(sLuaState, str);
	}

	const char* LuaUtils::pushstring(lua_State* L, const char* str){
		return lua_pushstring(L, str);
	}

	void LuaUtils::pushnumber(double number){
		lua_pushnumber(sLuaState, number);
	}

	void LuaUtils::pushnumber(lua_State* L, double number){
		lua_pushnumber(L, number);
	}

	void LuaUtils::pushinteger(int i){
		lua_pushinteger(sLuaState, i);
	}

	void LuaUtils::pushinteger(lua_State* L, int i){
		lua_pushinteger(L, i);
	}

	void LuaUtils::pushunsigned(unsigned u){
		lua_pushunsigned(sLuaState, u);
	}

	void LuaUtils::pushunsigned(lua_State* L, unsigned u){
		lua_pushunsigned(L, u);
	}

	void LuaUtils::pushboolean(bool b){
		lua_pushboolean(sLuaState, b ? 1 : 0);
	}

	void LuaUtils::pushboolean(lua_State* L, bool b){
		lua_pushboolean(L, b ? 1 : 0);
	}

	void LuaUtils::pushcfunction(lua_CFunction f){
		lua_pushcfunction(sLuaState, f);
	}

	void LuaUtils::pushVec2(const Vec2Tuple& data){
		luaU_push<Vec2Tuple>(sLuaState, data);
	}

	void LuaUtils::pushVec2(lua_State* L, const Vec2Tuple& data){
		luaU_push<Vec2Tuple>(L, data);
	}

	void LuaUtils::pushVec2I(const Vec2ITuple& data){
		luaU_push<Vec2ITuple>(sLuaState, data);
	}

	void LuaUtils::pushVec2I(lua_State* L, const Vec2ITuple& data){
		luaU_push<Vec2ITuple>(L, data);
	}

	void LuaUtils::pushVec3(const Vec3Tuple& data){
		luaU_push<Vec3Tuple>(sLuaState, data);
	}

	void LuaUtils::pushVec3(lua_State* L, const Vec3Tuple& data){
		luaU_push<Vec3Tuple>(L, data);
	}

	void LuaUtils::pushVec3I(const Vec3ITuple& data){
		luaU_push<Vec3ITuple>(sLuaState, data);
	}

	void LuaUtils::pushVec3I(lua_State* L, const Vec3ITuple& data){
		luaU_push<Vec3ITuple>(L, data);
	}

	void LuaUtils::pushVec4(const Vec4Tuple& data){
		luaU_push<Vec4Tuple>(sLuaState, data);
	}

	void LuaUtils::pushVec4(lua_State* L, const Vec4Tuple& data){
		luaU_push<Vec4Tuple>(L, data);
	}

	void LuaUtils::pushQuat(const QuatTuple& data){
		luaU_push<QuatTuple>(sLuaState, data);
	}

	void LuaUtils::pushQuat(lua_State* L, const QuatTuple& data){
		luaU_push<QuatTuple>(L, data);
	}

	void LuaUtils::pushTransformation(const TransformationTuple& data){
		luaU_push<TransformationTuple>(sLuaState, data);
	}

	void LuaUtils::pushTransformation(lua_State* L, const TransformationTuple& data){
		luaU_push<TransformationTuple>(L, data);
	}

	void LuaUtils::pushcfunction(lua_State* L, lua_CFunction f){
		lua_pushcfunction(L, f);
	}

	void LuaUtils::pushlightuserdata(void* p){
		lua_pushlightuserdata(sLuaState, p);
	}

	void LuaUtils::pushlightuserdata(lua_State* L, void* p){
		lua_pushlightuserdata(L, p);
	}

	const char* LuaUtils::tostring(int idx){
		if (sLuaState)
			return tostring(sLuaState, idx);
		return "";
	}

	const char* LuaUtils::tostring(lua_State* L, int idx){
		return lua_tostring(L, idx);
	}

	bool LuaUtils::toboolean(int idx){
		return lua_toboolean(sLuaState, idx) != 0;
	}

	bool LuaUtils::toboolean(lua_State* L, int idx){
		return lua_toboolean(L, idx) != 0;
	}

	int LuaUtils::tointeger(int idx){
		return lua_tointeger(sLuaState, idx);
	}

	int LuaUtils::tointeger(lua_State* L, int idx){
		return lua_tointeger(L, idx);
	}

	unsigned LuaUtils::tounsigned(int idx){
		return lua_tounsigned(sLuaState, idx);
	}

	unsigned LuaUtils::tounsigned(lua_State* L, int idx){
		return lua_tounsigned(L, idx);
	}

	double LuaUtils::tonumber(int idx){
		return lua_tonumber(sLuaState, idx);
	}

	double LuaUtils::tonumber(lua_State* L, int idx){
		return lua_tonumber(L, idx);
	}

	void* LuaUtils::touserdata(int index){
		return lua_touserdata(sLuaState, index);
	}

	void* LuaUtils::touserdata(lua_State* L, int index){
		return lua_touserdata(L, index);
	}

	const char* LuaUtils::checkstring(int index){
		return luaL_checkstring(sLuaState, index);
	}

	const char* LuaUtils::checkstring(lua_State* L, int index){
		return luaL_checkstring(L, index);
	}

	int LuaUtils::checkint(int index){
		return luaL_checkint(sLuaState, index);
	}

	int LuaUtils::checkint(lua_State* L, int index){
		return luaL_checkint(L, index);
	}

	unsigned LuaUtils::checkunsigned(int index){
		return luaL_checkunsigned(sLuaState, index);
	}

	unsigned LuaUtils::checkunsigned(lua_State* L, int index){
		return luaL_checkunsigned(L, index);
	}

	double LuaUtils::checknumber(int index){
		return luaL_checknumber(sLuaState, index);
	}

	double LuaUtils::checknumber(lua_State* L, int index){
		return luaL_checknumber(L, index);
	}

	void LuaUtils::checktype(int index, int luaType){
		luaL_checktype(sLuaState, index, luaType);
	}

	void LuaUtils::checktype(lua_State* L, int index, int luaType){
		luaL_checktype(L, index, luaType);
	}

	Vec2Tuple LuaUtils::checkVec2(int index){
		return luaU_check<Vec2Tuple>(sLuaState, index);
	}

	Vec2Tuple LuaUtils::checkVec2(lua_State* L, int index){
		return luaU_check<Vec2Tuple>(L, index);
	}

	Vec2ITuple LuaUtils::checkVec2I(int index){
		return luaU_check<Vec2ITuple>(sLuaState, index);
	}

	Vec2ITuple LuaUtils::checkVec2I(lua_State* L, int index){
		return luaU_check<Vec2ITuple>(L, index);
	}

	Vec3Tuple LuaUtils::checkVec3(int index){
		return luaU_check<Vec3Tuple>(sLuaState, index);
	}

	Vec3Tuple LuaUtils::checkVec3(lua_State* L, int index){
		return luaU_check<Vec3Tuple>(L, index);
	}

	Vec3ITuple LuaUtils::checkVec3I(int index){
		return luaU_check<Vec3ITuple>(sLuaState, index);
	}

	Vec3ITuple LuaUtils::checkVec3I(lua_State* L, int index){
		return luaU_check<Vec3ITuple>(L, index);
	}

	QuatTuple LuaUtils::checkQuat(int index){
		return luaU_check<QuatTuple>(sLuaState, index);
	}

	QuatTuple LuaUtils::checkQuat(lua_State* L, int index){
		return luaU_check<QuatTuple>(L, index);
	}

	TransformationTuple LuaUtils::checkTransformation(int index){
		return luaU_check<TransformationTuple>(sLuaState, index);
	}

	TransformationTuple LuaUtils::checkTransformation(lua_State* L, int index){
		return luaU_check<TransformationTuple>(L, index);
	}


	bool LuaUtils::isboolean(int index){
		return lua_isboolean(sLuaState, index);
	}

	bool LuaUtils::isboolean(lua_State* L, int index){
		return lua_isboolean(L, index);
	}

	bool LuaUtils::isnil(int index){
		return lua_isnil(sLuaState, index);
	}

	bool LuaUtils::isnil(lua_State* L, int index){
		return lua_isnil(L, index);
	}

	bool LuaUtils::isnumber(int index){
		return lua_isnumber(sLuaState, index) != 0;
	}

	bool LuaUtils::isnumber(lua_State* L, int index){
		return lua_isnumber(L, index) != 0;
	}

	bool LuaUtils::isstring(int index){
		return lua_isstring(sLuaState, index) != 0;
	}

	bool LuaUtils::isstring(lua_State* L, int index){
		return lua_isstring(L, index) != 0;
	}

	bool LuaUtils::istable(int index){
		return lua_istable(sLuaState, index);
	}

	bool LuaUtils::istable(lua_State* L, int index){
		return lua_istable(L, index);
	}

	bool LuaUtils::isuserdata(int index){
		return lua_isuserdata(sLuaState, index) != 0;
	}

	bool LuaUtils::isuserdata(lua_State* L, int index){
		return lua_isuserdata(L, index) != 0;
	}

	int LuaUtils::type(int index){
		return lua_type(sLuaState, index);
	}

	int LuaUtils::type(lua_State* L, int index){
		return lua_type(L, index);
	}

	const char* LuaUtils::luatypename(int index){
		return luaL_typename(sLuaState, index);
	}

	const char* LuaUtils::luatypename(lua_State* L, int index){
		return luaL_typename(L, index);
	}

	void LuaUtils::replace(int index){
		lua_replace(sLuaState, index);
	}

	void LuaUtils::replace(lua_State* L, int index){
		lua_replace(L, index);
	}

	void LuaUtils::pop(int n){
		lua_pop(sLuaState, n);
	}

	void LuaUtils::pop(lua_State* L, int n){
		lua_pop(L, n);
	}

	void LuaUtils::newtable(){
		lua_newtable(sLuaState);
	}

	void LuaUtils::newtable(lua_State* L){
		lua_newtable(L);
	}

	void LuaUtils::createtable(int narr, int nrec){
		lua_createtable(sLuaState, narr, nrec);
	}

	void LuaUtils::createtable(lua_State* L, int narr, int nrec){
		lua_createtable(L, narr, nrec);
	}

	/// Does the equivalent to t[key] = v, where t is the value at the given index and v is the value at the top of the stack.
	void LuaUtils::setfield(int tableindex, const char* key){
		lua_setfield(sLuaState, tableindex, key);
	}

	void LuaUtils::setfield(lua_State* L, int tableindex, const char* key){
		lua_setfield(L, tableindex, key);
	}

	/// Pushes onto the stack the value t[k], where t is the value at the given index. As in Lua, this function may trigger a metamethod for the "index" event
	void LuaUtils::getfield(int tableindex, const char* key){
		lua_getfield(sLuaState, tableindex, key);
	}

	void LuaUtils::getfield(lua_State* L, int tableindex, const char* key){
		lua_getfield(L, tableindex, key);
	}

	/// Pops a value from the stack and sets it as the new value of global name. 
	void LuaUtils::setglobal(const char* name){
		lua_setglobal(sLuaState, name);
	}

	void LuaUtils::setglobal(lua_State* L, const char* name){
		lua_setglobal(L, name);
	}

	/// Pushes onto the stack the value of the global \a key. 
	void LuaUtils::getglobal(const char* name){
		lua_getglobal(sLuaState, name);
	}

	void LuaUtils::getglobal(lua_State* L, const char* name){
		lua_getglobal(L, name);
	}

	void LuaUtils::gettable(int index){
		lua_gettable(sLuaState, index);
	}

	void LuaUtils::gettable(lua_State* L, int index){
		lua_gettable(L, index);
	}

	void LuaUtils::settable(int index){
		lua_settable(sLuaState, index);
	}

	void LuaUtils::settable(lua_State* L, int index){
		lua_settable(L, index);
	}

	/// Pushes a copy of the element at the given index onto the stack. 
	void LuaUtils::pushvalue(int index){
		lua_pushvalue(sLuaState, index);
	}

	void LuaUtils::pushvalue(lua_State* L, int index){
		lua_pushvalue(L, index);
	}

	/// Pops a table from the stack and sets it as the new metatable for the value at the given index. 
	void LuaUtils::setmetatable(int index){
		lua_setmetatable(sLuaState, index);
	}

	void LuaUtils::setmetatable(lua_State* L, int index){
		lua_setmetatable(L, index);
	}

	int LuaUtils::getmetatable(int index){
		return lua_getmetatable(sLuaState, index);
	}

	int LuaUtils::getmetatable(lua_State* L, int index){
		return lua_getmetatable(L, index);
	}

	void LuaUtils::Lgetmetatable(const char* tname){
		return luaL_getmetatable(sLuaState, tname);
	}

	void LuaUtils::Lgetmetatable(lua_State* L, const char* tname){
		return luaL_getmetatable(L, tname);
	}

	int LuaUtils::Lnewmetatable(const char* tname){
		return luaL_newmetatable(sLuaState, tname);
	}

	int LuaUtils::Lnewmetatable(lua_State* L, const char* tname){
		return luaL_newmetatable(L, tname);
	}

	/// Does the equivalent of t[n] = v, where t is the table at the given index and v is the value at the top of the stack. 
	void LuaUtils::rawseti(int tableindex, int n){
		lua_rawseti(sLuaState, tableindex, n);
	}

	void LuaUtils::rawseti(lua_State* L, int tableindex, int n){
		lua_rawseti(L, tableindex, n);
	}

	void LuaUtils::rawset(int index){
		lua_rawset(sLuaState, index);
	}

	void LuaUtils::rawset(lua_State* L, int index){
		lua_rawset(L, index);
	}

	int LuaUtils::gettop(){
		return lua_gettop(sLuaState);
	}

	int LuaUtils::gettop(lua_State* L){
		return lua_gettop(L);
	}

	void LuaUtils::settop(int index){
		lua_settop(sLuaState, index);
	}

	void LuaUtils::settop(lua_State* L, int index){
		lua_settop(L, index);
	}

	int LuaUtils::rawequal(int index1, int index2) {
		return lua_rawequal(sLuaState, index1, index2);
	}

	int LuaUtils::rawequal(lua_State* L, int index1, int index2){
		return lua_rawequal(L, index1, index2);
	}

	void LuaUtils::insert(int index){
		lua_insert(sLuaState, index);
	}

	void LuaUtils::insert(lua_State* L, int index){
		lua_insert(L, index);
	}

	void LuaUtils::remove(int index){
		lua_remove(sLuaState, index);
	}

	void LuaUtils::remove(lua_State* L, int index){
		lua_remove(L, index);
	}

	void* LuaUtils::newuserdata(size_t size){
		return lua_newuserdata(sLuaState, size);
	}

	void* LuaUtils::newuserdata(lua_State* L, size_t size){
		return lua_newuserdata(L, size);
	}

	void LuaUtils::call(int nargs, int nresults){
		lua_call(sLuaState, nargs, nresults);
	}

	void LuaUtils::call(lua_State* L, int nargs, int nresults){
		lua_call(L, nargs, nresults);
	}

	int LuaUtils::pcall(int nargs, int nresults, int msgh){
		return lua_pcall(sLuaState, nargs, nresults, msgh);
	}

	int LuaUtils::pcall(lua_State *L, int nargs, int nresults, int msgh){
		return lua_pcall(L, nargs, nresults, msgh);
	}

	int LuaUtils::next(int index){
		return lua_next(sLuaState, index);
	}

	int LuaUtils::next(lua_State* L, int index){
		return lua_next(L, index);
	}

	int LuaUtils::argerror(int arg, const char* extramsg){
		return luaL_argerror(sLuaState, arg, extramsg);
	}

	int LuaUtils::argerror(lua_State* L, int arg, const char* extramsg){
		return luaL_argerror(L, arg, extramsg);
	}

	int LuaUtils::error(int arg, const char* msg){
		return luaL_error(sLuaState, msg);
	}

	int LuaUtils::error(lua_State* L, const char* msg){
		return luaL_error(L, msg);
	}

	static RecursiveSpinLock<true, false> sLock;
	void LuaUtils::LockLua(){
		sLock.Lock();
	}

	void LuaUtils::UnlockLua(){
		sLock.Unlock();
	}

	//---------------------------------------------------------------------------
	LuaLock::LuaLock(){
		LuaUtils::LockLua();
	}
	
	LuaLock::~LuaLock(){
		LuaUtils::UnlockLua();
	}

	LuaLock::operator lua_State*() const{
		return LuaUtils::GetLuaState();
	}

	//---------------------------------------------------------------------------
	// Stack watcher
	//---------------------------------------------------------------------------
	LUA_STACK_WATCHER::LUA_STACK_WATCHER(lua_State* L, const char* name)
		: lua(L), mName(name)
	{
		assert(name != 0);
		top = lua_gettop(L);
	}

	LUA_STACK_WATCHER::~LUA_STACK_WATCHER()
	{
		int now = lua_gettop(lua);
		if (top != now)
		{
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString(
				"Stack is polluted(prev:%d, now:%d) - %s", top, now, mName).c_str());
		}
		assert(top == now);
	}

	LUA_STACK_CLIPPER::LUA_STACK_CLIPPER(lua_State* L)
	{
		lua = L;
		top = lua_gettop(L);
	}

	LUA_STACK_CLIPPER::~LUA_STACK_CLIPPER()
	{
		lua_settop(lua, top);
	}	
}



//---------------------------------------------------------------------------
// Tuple Helper
//---------------------------------------------------------------------------
// PushNumber
template <class T>
void PushNumbersElem(lua_State* L, int& n, const T& value){
	lua_pushnumber(L, value);
	lua_rawseti(L, -2, n++);
}

template <>
void PushNumbersElem(lua_State* L, int& n, const bool& value){
	lua_pushboolean(L, value);
	lua_rawseti(L, -2, n++);
}

template<class Tuple, std::size_t N>
struct TupleIteratorPush {
	static void iterate(lua_State* L, int& n, const Tuple& t)
	{
		TupleIteratorPush<Tuple, N - 1>::iterate(L, n, t);
		PushNumbersElem(L, n, std::get<N - 1>(t));
	}
};
template<class Tuple>
struct TupleIteratorPush<Tuple, 1>{
	static void iterate(lua_State* L, int& n, const Tuple& t)
	{
		PushNumbersElem(L, n, std::get<0>(t));
	}
};

template<class... Args>
void PushNumbers(lua_State* L, int& n, const std::tuple<Args...>& t)
{
	TupleIteratorPush<decltype(t), sizeof...(Args)>::iterate(L, n, t);
}

//---------------------------------------------------------------------------
// PullNumber
template <class T>
void PullNumbersElem(lua_State* L, int index, int& n, T& value){
	lua_rawgeti(L, index, n++);
	value = (T)lua_tonumber(L, -1);
	LuaUtils::pop(L, 1);
}
template <>
void PullNumbersElem(lua_State* L, int index, int& n, bool& value){
	lua_rawgeti(L, index, n++);
	value = lua_toboolean(L, -1) != 0;
	LuaUtils::pop(L, 1);
}

template<class Tuple, std::size_t N>
struct TupleIteratorPull {
	static void iterate(lua_State* L, int index, int& n, Tuple& t)
	{
		TupleIteratorPull<Tuple, N - 1>::iterate(L, index, n, t);
		PullNumbersElem(L, index, n, std::get<N - 1>(t));
	}
};
template<class Tuple>
struct TupleIteratorPull<Tuple, 1>{
	static void iterate(lua_State* L, int index, int& n, Tuple& t)
	{
		PullNumbersElem(L, index, n, std::get<0>(t));
	}
};

template<class... Args>
void PullNumbers(lua_State* L, int index, int& n, std::tuple<Args...>& t)
{
	TupleIteratorPull<decltype(t), sizeof...(Args)>::iterate(L, index, n, t);
}

namespace fb{
	//---------------------------------------------------------------------------
	// string
	//---------------------------------------------------------------------------
	std::string luaU_Impl<std::string>::luaU_check(lua_State* L, int index) {
		return std::string(luaL_checkstring(L, index));
	}

	std::string luaU_Impl<std::string>::luaU_to(lua_State* L, int index){
		return std::string(lua_tostring(L, index));
	}

	void luaU_Impl<std::string>::luaU_push(lua_State* L, const std::string& val){
		lua_pushstring(L, val.c_str());
	}

	//---------------------------------------------------------------------------
	// Vec2ITuple
	//---------------------------------------------------------------------------
	fb::Vec2ITuple luaU_Impl< fb::Vec2ITuple >::luaU_check(lua_State* L, int index){
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::Vec2ITuple luaU_Impl< fb::Vec2ITuple >::luaU_to(lua_State* L, int index)
	{
		fb::Vec2ITuple ret;
		lua_rawgeti(L, index, 1);
		std::get<0>(ret) = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		std::get<1>(ret) = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	void luaU_Impl< fb::Vec2ITuple >::luaU_push(lua_State* L, const fb::Vec2ITuple& val)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, std::get<0>(val));
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, std::get<1>(val));
		lua_rawseti(L, -2, 2);
	}

	//---------------------------------------------------------------------------
	// Vec2Tuple
	//---------------------------------------------------------------------------
	fb::Vec2Tuple luaU_Impl< fb::Vec2Tuple >::luaU_check(lua_State* L, int index)
	{
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::Vec2Tuple luaU_Impl< fb::Vec2Tuple >::luaU_to(lua_State* L, int index)
	{
		fb::Vec2Tuple ret;
		lua_rawgeti(L, index, 1);
		std::get<0>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		std::get<1>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	void luaU_Impl< fb::Vec2Tuple >::luaU_push(lua_State* L, const fb::Vec2Tuple& val)
	{
		lua_createtable(L, 2, 0);
		lua_pushnumber(L, std::get<0>(val));
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, std::get<1>(val));
		lua_rawseti(L, -2, 2);
	}

	//---------------------------------------------------------------------------
	// Vec3ITuple
	//---------------------------------------------------------------------------
	fb::Vec3ITuple luaU_Impl< fb::Vec3ITuple >::luaU_check(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec3I luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::Vec3ITuple luaU_Impl< fb::Vec3ITuple >::luaU_to(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec3I luaU_to(lua_State* L, int index)");
		fb::Vec3ITuple ret;
		lua_rawgeti(L, index, 1);
		std::get<0>(ret) = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		std::get<1>(ret) = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		std::get<2>(ret) = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	void luaU_Impl< fb::Vec3ITuple >::luaU_push(lua_State* L, const fb::Vec3ITuple& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushinteger(L, std::get<0>(val));
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, std::get<1>(val));
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, std::get<2>(val));
		lua_rawseti(L, -2, 3);
	}

	//---------------------------------------------------------------------------
	// Vec3Tuple
	//---------------------------------------------------------------------------
	fb::Vec3Tuple luaU_Impl< fb::Vec3Tuple >::luaU_check(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec3Tuple luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::Vec3Tuple luaU_Impl< fb::Vec3Tuple >::luaU_to(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec3Tuple luaU_to(lua_State* L, int index)");
		fb::Vec3Tuple ret;
		lua_rawgeti(L, index, 1);
		std::get<0>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		std::get<1>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		std::get<2>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	void luaU_Impl< fb::Vec3Tuple >::luaU_push(lua_State* L, const fb::Vec3Tuple& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, std::get<0>(val));
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, std::get<1>(val));
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, std::get<2>(val));
		lua_rawseti(L, -2, 3);
	}

	//---------------------------------------------------------------------------
	// Vec4Tuple
	//---------------------------------------------------------------------------
	fb::Vec4Tuple luaU_Impl< fb::Vec4Tuple >::luaU_check(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec4Tuple luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::Vec4Tuple luaU_Impl< fb::Vec4Tuple >::luaU_to(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::Vec4Tuple luaU_to(lua_State* L, int index)");
		fb::Vec4Tuple ret;
		lua_rawgeti(L, index, 1);
		std::get<0>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		std::get<1>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		std::get<2>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);
		std::get<3>(ret) = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	void luaU_Impl< fb::Vec4Tuple >::luaU_push(lua_State* L, const fb::Vec4Tuple& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, std::get<0>(val));
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, std::get<1>(val));
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, std::get<2>(val));
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, std::get<3>(val));
		lua_rawseti(L, -2, 4);
	}

	//---------------------------------------------------------------------------
	// QuatTuple
	//---------------------------------------------------------------------------
	fb::QuatTuple luaU_Impl< fb::QuatTuple>::luaU_check(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, " fb::QuatTuple luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::QuatTuple luaU_Impl< fb::QuatTuple>::luaU_to(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, " fb::QuatTuple luaU_to(lua_State* L, int index)");
		fb::QuatTuple ret;
		int n = 1;
		PullNumbers(L, index, n, ret.value);
		return ret;
	}

	void luaU_Impl< fb::QuatTuple>::luaU_push(lua_State* L, const fb::QuatTuple& val)
	{
		lua_createtable(L, 0, 3);
		int n = 1;
		PushNumbers(L, n, val.value);
	}

	//---------------------------------------------------------------------------
	// TransformationTuple
	//---------------------------------------------------------------------------
	fb::TransformationTuple luaU_Impl< fb::TransformationTuple>::luaU_check(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::TransformationTuple luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		return luaU_to(L, index);
	}

	fb::TransformationTuple luaU_Impl< fb::TransformationTuple>::luaU_to(lua_State* L, int index)
	{
		fb::LUA_STACK_WATCHER watcher(L, "fb::TransformationTuple luaU_to(lua_State* L, int index)");
		int n = 1;
		fb::TransformationTuple ret;
		PullNumbers(L, index, n, ret);
		return ret;
	}

	void luaU_Impl< fb::TransformationTuple>::luaU_push(lua_State* L, const fb::TransformationTuple& val)
	{
		lua_createtable(L, 22, 0);

		int n = 1;
		PushNumbers(L, n, val);
	}

}