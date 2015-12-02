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

#pragma once
#include "FBCommonHeaders/platform.h"
#include "FBStringLib/StringLib.h"
#include "FBDebugLib/Logger.h"
#include "FBCommonHeaders/Types.h"

#if !defined(LUA_TNONE)
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTAGS		9
#endif

struct lua_State;
typedef int (*lua_CFunction) (lua_State *L);
#if !defined(FBLua)
//---------------------------------------------------------------------------
// If Lua versions is changed, need to check these definitions.
//---------------------------------------------------------------------------
typedef struct luaL_Reg {
	const char *name;
	lua_CFunction func;
} luaL_Reg;

#define LUAI_MAXSTACK		1000000
#define LUAI_FIRSTPSEUDOIDX	(-LUAI_MAXSTACK - 1000)
#define LUA_REGISTRYINDEX	LUAI_FIRSTPSEUDOIDX
#else
struct luaL_Reg;
#endif
namespace fb
{ 
	const char* GetCWD();
}

#define CHECK_NUM_LUA_ARGS_FB(x) \
	int numLuaArgs = LuaUtils::gettop(L); \
if (numLuaArgs != (x)) \
{\
	assert(0); \
	return LuaUtils::error(L, FormatString("Got %d arguments, expected %d", numLuaArgs, (x)).c_str()); \
}

#define LUA_SETCFUNCTION(lua, name) LuaUtils::pushcfunction((lua), (name));\
	LuaUtils::setglobal((lua), (#name));

#define LUA_PCALL(lua, arg, ret) if(int error = LuaUtils::pcall((lua), (arg), ret, 0)) \
{\
	const char* errorString = LuaUtils::tostring(lua, -1); \
	Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to call lua function. Error(%d)", error).c_str());\
	LuaUtils::PrintLuaErrorString(lua, errorString);\
	LuaUtils::pop(lua, 1); \
	assert(0);\
	return;\
}

#define LUA_PCALL_RET_FALSE(lua, arg, ret) \
int cfuncbase = LuaUtils::gettop(lua) - (arg);  /* function index */\
LuaUtils::pushcfunction(lua, LuaUtils::Traceback);  /* push traceback function */\
LuaUtils::insert(lua, cfuncbase);  /* put it under chunk and args */\
if(int error = LuaUtils::pcall((lua), (arg), ret, cfuncbase)) \
{\
	LuaUtils::remove(lua, cfuncbase);\
	const char* errorString = LuaUtils::tostring(lua, -1);\
	Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to call lua function. Error(%d)", error).c_str());\
	LuaUtils::PrintLuaErrorString(lua, errorString);\
	LuaUtils::pop(lua, 1); \
	assert(0); \
	return false; \
}\
else{\
	LuaUtils::remove(lua, cfuncbase);\
}

#define LUA_PCALL_NO_RET(lua, arg, ret) if(int error = LuaUtils::pcall((lua), arg, ret, 0)) \
{\
	const char* errorString = LuaUtils::tostring(lua, -1); \
	Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to call lua function. Error(%d)", error).c_str());\
	LuaUtils::PrintLuaErrorString(lua, errorString);\
	LuaUtils::pop(lua, 1); \
	assert(0); \
}

#define REGISTER_ENUM_TO_LUA(endIdx, enumName) \
	inline void RegisterToLua(lua_State* L)\
{\
	fb::LuaUtils::createtable(L, 0, (endIdx)); \
for (int i = 0; i <= (endIdx); ++i)\
{\
	fb::LuaUtils::pushinteger(L, i); \
	fb::LuaUtils::setfield(L, -2, ConvertToString(Enum(i))); \
}\
	fb::LuaUtils::getglobal(L, "NoNewMethod"); \
	fb::LuaUtils::setfield(L, -2, "__newindex"); \
	fb::LuaUtils::pushvalue(L, -1); \
	fb::LuaUtils::setmetatable(L, -2); \
	fb::LuaUtils::setglobal(L, #enumName); \
	\
	fb::LuaUtils::createtable(L, 0, (endIdx)); \
for (int i = 0; i <= (endIdx); ++i)\
{\
	fb::LuaUtils::pushstring(L, ConvertToString(Enum(i))); \
	fb::LuaUtils::rawseti(L, -2, i); \
}\
	fb::LuaUtils::getglobal(L, "NoNewMethod"); \
	fb::LuaUtils::setfield(L, -2, "__newindex"); \
	fb::LuaUtils::pushvalue(L, -1); \
	fb::LuaUtils::setmetatable(L, -2); \
	fb::LuaUtils::setglobal(L, #enumName "String");\
}

#define REGISTER_CLASS_ENUM_TO_LUA(classname, enumName, endIdx) \
	static void RegisterEnumToLua(lua_State* L)\
{\
	LuaUtils::getglobal(L, #classname); \
	if (LuaUtils::isnil(L, -1))\
		{\
		assert(0); \
		LuaUtils::pop(L, 1); \
		LuaUtils::createtable(L, 1, 0); \
		LuaUtils::setglobal(L, #classname); \
		LuaUtils::getglobal(L, #classname);\
		}\
\
	LuaUtils::createtable(L, 0, endIdx); \
	for (int i = 0; i <= endIdx; ++i)\
		{\
		LuaUtils::pushinteger(L, i);\
		LuaUtils::setfield(L, -2, ConvertToString(##enumName(i))); \
		}\
	LuaUtils::getglobal(L, "NoNewMethod");\
	LuaUtils::setfield(L, -2, "__newindex");\
	LuaUtils::pushvalue(L, -1);\
	LuaUtils::setmetatable(L, -2);\
	LuaUtils::setfield(L, -2, #enumName);\
	LuaUtils::pop(L, 1);\
}

namespace fb
{
	struct FB_DLL_LUA LUA_STACK_WATCHER
	{
		lua_State* lua;
		int top;
		const char* mName;

		LUA_STACK_WATCHER(lua_State* L, const char* name);
		~LUA_STACK_WATCHER();
		
	};

	struct FB_DLL_LUA LUA_STACK_CLIPPER
	{
		lua_State* lua;
		int top;

		LUA_STACK_CLIPPER(lua_State* L);
		~LUA_STACK_CLIPPER();		
	};

	//---------------------------------------------------------------------------
	// Generic lua call
	//---------------------------------------------------------------------------
	class FB_DLL_LUA LuaUtils{
	public:
		/** The first lua state created by this function will be the main state.
		*/
		static lua_State* OpenLuaState();
		/** Returns the main lua state.
		*/
		static lua_State* GetLuaState();
		/** Close lus state. If L is 0, then the main state will be closed.
		*/
		static void CloseLuaState(lua_State* L);
		static void CallLuaFunction(lua_State* L, const char* func, const char* sig, ...);
		static bool CheckLuaGlobalExist(lua_State* L, const char* name);
		static void PrintLuaErrorString(lua_State* L, const char* luaString);
		static void PrintLuaDebugInfo(lua_State* L, int level);
		static std::string GetLuaValueAsString(lua_State* L, int stackIndex);
		static std::string GetLuaVarAsString(lua_State* L, const char* varName, const char* luaFile = 0);
		static bool GetLuaVarAsBoolean(lua_State* L, const char* varName);
		static Vec2ITuple GetLuaVarAsVec2I(const char* varname);
		static Vec2ITuple GetLuaVarAsVec2I(lua_State* L, const char* varname);
		static float GetLuaVarAsFloat(lua_State* L, const char* varName);
		static unsigned GetLuaVarAsUnsigned(lua_State* L, const char* varName);
		static void SetLuaVar(lua_State* L, const char* varName, bool value);
		static bool ExecuteLua(const char* chunk);
		static bool ExecuteLua(lua_State* L, const char* chunk);
		static bool DoFile(lua_State* L, const char* filepath);
		static bool DoFile(const char* filepath);
		static bool LoadConfig(const char* filepath);
		static int Traceback(lua_State *L);

		/// push nil
		static void pushnil();
		static void pushnil(lua_State* L);
		/** Pushes the zero-terminated string pointed to by s onto the stack. 
		Lua makes (or reuses) an internal copy of the given string, so the memory 
		at str can be freed or reused immediately after the function returns.
		Returns a pointer to the internal copy of the string. If s is NULL, 
		pushes nil and returns NULL. 
		*/
		static const char* pushstring(const char* str);
		static const char* pushstring(lua_State* L, const char* str);
		static void pushnumber(double number);
		static void pushnumber(lua_State* L, double number);
		static void pushinteger(int i);
		static void pushinteger(lua_State* L, int i);
		static void pushunsigned(unsigned u);
		static void pushunsigned(lua_State* L, unsigned u);
		static void pushboolean(bool b);
		static void pushboolean(lua_State* L, bool b);
		static void pushcfunction(lua_CFunction f);
		static void pushcfunction(lua_State* L, lua_CFunction f);
		static void pushlightuserdata(void* p);
		static void pushlightuserdata(lua_State* L, void* p);
		static void pushVec2(const Vec2Tuple& data);
		static void pushVec2(lua_State* L, const Vec2Tuple& data);
		static void pushVec2I(const Vec2ITuple& data);
		static void pushVec2I(lua_State* L, const Vec2ITuple& data);
		static void pushVec3(const Vec3Tuple& data);
		static void pushVec3(lua_State* L, const Vec3Tuple& data);
		static void pushVec3I(const Vec3ITuple& data);
		static void pushVec3I(lua_State* L, const Vec3ITuple& data);
		static void pushVec4(const Vec4Tuple& data);
		static void pushVec4(lua_State* L, const Vec4Tuple& data);
		static void pushQuat(const QuatTuple& data);
		static void pushQuat(lua_State* L, const QuatTuple& data);
		static void pushTransformation(const TransformationTuple& data);
		static void pushTransformation(lua_State* L, const TransformationTuple& data);

		static const char* tostring(int index);
		static const char* tostring(lua_State* L, int index);
		static bool toboolean(int index);
		static bool toboolean(lua_State* L, int index);
		static int tointeger(int index);
		static int tointeger(lua_State* L, int index);
		static unsigned tounsigned(int index);
		static unsigned tounsigned(lua_State* L, int index);
		static double tonumber(int index);
		static double tonumber(lua_State* L, int index);
		static void* touserdata(int index);
		static void* touserdata(lua_State* L, int index);
		static const char* checkstring(int index);		
		static const char* checkstring(lua_State* L, int index);
		static int checkint(int index);
		static int checkint(lua_State* L, int index);
		static unsigned checkunsigned(int index);
		static unsigned checkunsigned(lua_State* L, int index);
		static double checknumber(int index);
		static double checknumber(lua_State* L, int index);
		static void checktype(int index, int luaType);
		static void checktype(lua_State* L, int index, int luaType);
		static Vec2Tuple checkVec2(int index);
		static Vec2Tuple checkVec2(lua_State* L, int index);
		static Vec2ITuple checkVec2I(int index);
		static Vec2ITuple checkVec2I(lua_State* L, int index);
		static Vec3Tuple checkVec3(int index);
		static Vec3Tuple checkVec3(lua_State* L, int index);
		static Vec3ITuple checkVec3I(int index);
		static Vec3ITuple checkVec3I(lua_State* L, int index);
		static QuatTuple checkQuat(int index);
		static QuatTuple checkQuat(lua_State* L, int index);
		static TransformationTuple checkTransformation(int index);
		static TransformationTuple checkTransformation(lua_State* L, int index);

		static bool isboolean(int index);
		static bool isboolean(lua_State* L, int index);
		static bool isnil(int index);
		static bool isnil(lua_State* L, int index);
		static bool isnumber(int index);
		static bool isnumber(lua_State* L, int index);
		static bool isstring(int index);
		static bool isstring(lua_State* L, int index);
		static bool istable(int index);
		static bool istable(lua_State* L, int index);
		static bool isuserdata(int index);
		static bool isuserdata(lua_State* L, int index);
		static int type(int index);
		static int type(lua_State* L, int index);
		static const char* luatypename(int index);
		static const char* luatypename(lua_State* L, int index);
		static void replace(int index);
		static void replace(lua_State* L, int index);
		static void pop(int n);
		static void pop(lua_State* L, int n);

		/// Creates a new empty table and pushes it onto the stack. It is equivalent to LuaUtils::createtable(L, 0, 0). 
		static void newtable();
		static void newtable(lua_State* L);
		/**	Creates a new empty table and pushes it onto the stack. 
		Parameter \a narr is a hint for how many elements the table will have as a sequence; 
		parameter \a nrec is a hint for how many other elements the table will have. 
		Lua may use these hints to preallocate memory for the new table. 
		This pre-allocation is useful for performance when you know in advance how many elements the table will have. 
		Otherwise you can use the function lua_newtable.
		*/
		static void createtable(int narr, int nrec);
		static void createtable(lua_State* L, int narr, int nrec);
		/// Does the equivalent to t[key] = v, where t is the value at the given index and v is the value at the top of the stack.
		static void setfield(int tableindex, const char* key);
		static void setfield(lua_State* L, int tableindex, const char* key);
		/// Pushes onto the stack the value t[k], where t is the value at the given index. As in Lua, this function may trigger a metamethod for the "index" event
		static void getfield(int tableindex, const char* key);
		static void getfield(lua_State* L, int tableindex, const char* key);
		/// Pops a value from the stack and sets it as the new value of global name. 
		static void setglobal(const char* name);
		static void setglobal(lua_State* L, const char* name);
		/// Pushes onto the stack the value of the global \a key. 
		static void getglobal(const char* name);
		static void getglobal(lua_State* L, const char* name);
		static void gettable(int index);
		static void gettable(lua_State* L, int index);
		static void settable(int index);
		static void settable(lua_State* L, int index);

		/// Pushes a copy of the element at the given index onto the stack. 
		static void pushvalue(int index);
		static void pushvalue(lua_State* L, int index);
		/// Pops a table from the stack and sets it as the new metatable for the value at the given index. 
		static void setmetatable(int index);
		static void setmetatable(lua_State* L, int index);
		static int getmetatable(int index);
		static int getmetatable(lua_State* L, int index);
		/// Pushes onto the stack the metatable associated with name \a tname in the registry 
		static void Lgetmetatable(const char* tname);
		static void Lgetmetatable(lua_State* L, const char* tname);
		static int Lnewmetatable(const char* tname);
		static int Lnewmetatable(lua_State* L, const char* tname);
		/// Does the equivalent of t[n] = v, where t is the table at the given index and v is the value at the top of the stack. 
		static void rawseti(int tableindex, int n);
		static void rawseti(lua_State* L, int tableindex, int n);
		/// Similar to lua_settable, but does a raw assignment (i.e., without metamethods). 
		/// Does the equivalent to t[k] = v, where t is the value at the given index, v is the value at the top of the stack, and k is the value just below the top. 
		static void rawset(int index);
		static void rawset(lua_State* L, int index);
		/// Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack). 
		static int gettop();
		static int gettop(lua_State* L);
		/// Accepts any index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed. 
		static void settop(int index);
		static void settop(lua_State* L, int index);
		/// Returns 1 if the two values in indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid. 
		static int rawequal(int index1, int index2);
		static int rawequal(lua_State* L, int index1, int index2);
		static void insert(int index);
		static void insert(lua_State* L, int index);
		static void remove(int index);
		static void remove(lua_State* L, int index);
		static void* newuserdata(size_t size);
		static void* newuserdata(lua_State* L, size_t size);
		static void call(int nargs, int nresults);
		static void call(lua_State* L, int nargs, int nresults);
		static int pcall(int nargs, int nresults, int msgh = 0);
		static int pcall(lua_State *L, int nargs, int nresults, int msgh = 0);
		static int next(int index);
		static int next(lua_State* L, int index);

		static int argerror(int arg, const char* extramsg);
		static int argerror(lua_State* L, int arg, const char* extramsg);
		static int error(int arg, const char* msg);
		static int error(lua_State* L, const char* msg);



		static void LockLua();
		static void UnlockLua();
	};	

	struct FB_DLL_LUA LuaLock{
		LuaLock();
		~LuaLock();

		operator lua_State*() const;
	};

#include "luawrapperutil.hpp"
	template<>
	struct FB_DLL_LUA luaU_Impl < std::string >
	{
		static std::string luaU_check(lua_State* L, int index);
		static std::string luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const std::string& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::Vec2ITuple >
	{
		static fb::Vec2ITuple luaU_check(lua_State* L, int index);
		static fb::Vec2ITuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::Vec2ITuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::Vec2Tuple >
	{
		static fb::Vec2Tuple luaU_check(lua_State* L, int index);
		static fb::Vec2Tuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::Vec2Tuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::Vec3ITuple >
	{
		static fb::Vec3ITuple luaU_check(lua_State* L, int index);
		static fb::Vec3ITuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::Vec3ITuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::Vec3Tuple >
	{
		static fb::Vec3Tuple luaU_check(lua_State* L, int index);
		static fb::Vec3Tuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::Vec3Tuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::Vec4Tuple >
	{
		static fb::Vec4Tuple luaU_check(lua_State* L, int index);
		static fb::Vec4Tuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::Vec4Tuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::QuatTuple >
	{
		static fb::QuatTuple luaU_check(lua_State* L, int index);
		static fb::QuatTuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::QuatTuple& val);
	};

	template<>
	struct FB_DLL_LUA luaU_Impl < fb::TransformationTuple >
	{
		static fb::TransformationTuple luaU_check(lua_State* L, int index);
		static fb::TransformationTuple luaU_to(lua_State* L, int index);
		static void luaU_push(lua_State* L, const fb::TransformationTuple& val);
	};
}