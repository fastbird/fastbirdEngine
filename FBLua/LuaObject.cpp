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
#include "LuaObject.h"
#include "LuaUtils.h"
#include "FBStringLib/StringConverter.h"
#include "luawrapperutil.hpp"
using namespace fb;

fb::VectorMap<int, unsigned> LuaObject::sUsedCount;
fb::SpinLock<true, false> LuaObject::sUsedCountGuard;
void LuaObject::AddUsedCount(int ref)
{
	if (ref == LUA_NOREF)
		return;
	sUsedCountGuard.Lock();
	sUsedCount[ref] += 1;
	sUsedCountGuard.Unlock();
}
bool LuaObject::ReleaseUsedCount(int ref)
{
	if (ref == LUA_NOREF)
		return false;
	sUsedCountGuard.Lock();
	auto itFind = sUsedCount.Find(ref);
	assert(itFind != sUsedCount.end());
	if (--(itFind->second) == 0)
	{
		sUsedCountGuard.Unlock();
		return true; // remove it
	}
	sUsedCountGuard.Unlock();

	return false; // still alive
}


LuaObject::LuaObject()
	:LuaObject((lua_State*)0)
{

}

LuaObject::LuaObject(lua_State* L, int index, bool pop)
	: LuaObject(L)
{
	// the vaile at index is not poped.
	if (!pop)
		lua_pushvalue(L, index);
	CheckType();
	mRef = luaL_ref(L, LUA_REGISTRYINDEX);
	AddUsedCount(mRef);
}

LuaObject::LuaObject(lua_State* L, const char* globalName)
	: LuaObject(L)
{
	assert(globalName != 0);
	auto splitted = Split(globalName, ".");
	LUA_STACK_CLIPPER clip(L);
	for (auto& name : splitted){
		if (!IsTable()){
			lua_getglobal(L, name.c_str());
			CheckType();
		}
		else{			
			lua_getfield(L, -1, name.c_str());
			CheckType();
		}
	}	
	mRef = luaL_ref(L, LUA_REGISTRYINDEX);
	AddUsedCount(mRef);	
	mName = globalName;
}

LuaObject::LuaObject(const char* globalName)
	:LuaObject(LuaUtils::GetLuaState(), globalName)
{

}

LuaObject::LuaObject(const LuaObject& other)
	: LuaObject(other.mL)
{
	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	mRef = other.mRef;
	mType = other.mType;
	AddUsedCount(mRef);
	mName = other.mName;
	if (other.mSelf)
	{
		mSelf = FB_NEW(LuaObject)(*other.mSelf);
	}
}

LuaObject& LuaObject::operator=(const LuaObject& other)
{
	mL = other.mL;
	mType = LUA_TNONE;
	mRef = LUA_NOREF;
	FB_SAFE_DELETE(mSelf);

	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	mRef = other.mRef;
	mType = other.mType;
	AddUsedCount(mRef);
	mName = other.mName;
	if (other.mSelf)
	{
		mSelf = FB_NEW(LuaObject)(*other.mSelf);
	}
	return *this;
}

LuaObject::LuaObject(lua_State* L)
	: mL(L)
	, mType(LUA_TNONE)
	, mRef(LUA_NOREF)
	, mSelf(0)
{
}

LuaObject::~LuaObject()
{
	Clear();
}

void LuaObject::SetSelf(const LuaObject& other)
{
	assert(other.IsTable());
	FB_SAFE_DELETE(mSelf);
	mSelf = FB_NEW(LuaObject)(other);
}

void LuaObject::FindFunction(lua_State* L, const char* funcName)
{
	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	auto splited = Split(funcName, ":");
	auto size = splited.size();
	const auto& last = splited[size - 1];
	auto f = splited[size - 1].find('(');
	if (f != std::string::npos)
	{
		splited[size - 1] = splited[size - 1].substr(0, f);
	}
	else
	{
		splited[size - 1] = splited[size - 1];
		//assert(0 && "You need to put () at the end of the function name.");
	}
	auto splited2 = Split(splited[0].c_str(), ".");
	size = splited2.size();
	LuaObject obj(L, splited2[0].c_str());
	if (!obj.IsValid())
		return;

	for (unsigned u = 1; u < size; ++u)
	{
		obj = obj.GetField(splited2[u].c_str());
	}
	FB_SAFE_DELETE(mSelf);
	if (splited.size() == 2)
	{
		*this = obj.GetField(splited[1].c_str());
		mSelf = FB_NEW(LuaObject)(obj);
	}
	else
	{
		*this = obj;
	}
	mName = funcName;
}

void LuaObject::NewTable(lua_State* L)
{
	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	mL = L;
	lua_newtable(L);
	CheckType();
	mRef = luaL_ref(L, LUA_REGISTRYINDEX);
	AddUsedCount(mRef);
}

bool LuaObject::IsValid(bool nilIsValid) const
{
	bool valid = mL != 0 && mRef != LUA_NOREF;
	if (!nilIsValid)
		valid = valid && mRef != LUA_REFNIL;
	return valid;
}

void LuaObject::SetGlobalName(lua_State* L, const char* globalName)
{
	ReleaseUsedCount(mRef);
	mRef = LUA_NOREF;

	mL = L;
	assert(globalName != 0);
	lua_getglobal(L, globalName);
	CheckType();
	mRef = luaL_ref(L, LUA_REGISTRYINDEX);
	AddUsedCount(mRef);
	mName = globalName;
}

void LuaObject::PushToStack() const
{
	if (IsValid()){
		lua_rawgeti(mL, LUA_REGISTRYINDEX, mRef);
		if (mSelf)
		{
			assert(IsFunction());
			mSelf->PushToStack();
		}
	}
}

bool LuaObject::Call()
{
	if (!IsFunction())
	{
		assert(0);
		return false;
	}
	PushToStack();
	LUA_PCALL_RET_FALSE(mL, IsMethod() ? 1 : 0, 0);
	return true;
}

// You need to push the function manually before call this function
// Push this LuaObject(Function) and then push args
bool LuaObject::CallWithManualArgs(unsigned arg, unsigned ret)
{
	if (mSelf)
	{
		arg++; // for self.
	}
	int cfuncbase = LuaUtils::gettop(mL) - (arg);  /* function index */
	LuaUtils::pushcfunction(mL, LuaUtils::Traceback);  /* push traceback function */
	LuaUtils::insert(mL, cfuncbase);  /* put it under chunk and args */
	if (int error = LuaUtils::pcall((mL), (arg), ret, cfuncbase))
	{
		LuaUtils::remove(mL, cfuncbase);
		const char* errorString = LuaUtils::tostring(mL, -1);
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to call lua function. Error(%d)", error).c_str());
		LuaUtils::PrintLuaErrorString(mL, errorString);
		LuaUtils::pop(mL, 1);
		assert(0);
		return false;
	}
	else{
		LuaUtils::remove(mL, cfuncbase);
	}
	return true;
}

bool LuaObject::IsFunction() const
{
	return mType == LUA_TFUNCTION;
}

bool LuaObject::IsMethod() const
{
	return mSelf != 0;
}

bool LuaObject::IsTable() const
{
	return mType == LUA_TTABLE;
}

bool LuaObject::IsString() const
{
	return mType == LUA_TSTRING;
}

bool LuaObject::IsNumber() const
{
	return mType == LUA_TNUMBER;
}
bool LuaObject::IsBool() const
{
	return mType == LUA_TBOOLEAN;
}

bool LuaObject::IsNil() const
{
	return mType == LUA_TNIL;
}

void LuaObject::CheckType()
{
	mType = lua_type(mL, -1);
}

LuaObject LuaObject::GetField(const char* fieldName) const
{
	assert(mL);
	assert(IsTable());
	if (!IsTable())
	{
		return LuaObject();
	}
	LUA_STACK_WATCHER watcher(mL, "LuaObject LuaObject::GetField(const char* fieldName) const");
	PushToStack();

	lua_getfield(mL, -1, fieldName);
	LuaObject fieldObject(mL, -1);
	lua_pop(mL, 2);
	return fieldObject;
}

LuaTableIterator LuaObject::GetTableIterator() const
{
	return LuaTableIterator(*this);
}

LuaSequenceIterator LuaObject::GetSequenceIterator() const
{
	return LuaSequenceIterator(*this);
}

//----------------------------------------------------------------
LuaObject LuaObject::SetFieldTable(const char* fieldName) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "LuaObject LuaObject::SetFieldTable(const char* fieldName)");
	PushToStack();
	lua_newtable(mL);
	LuaObject ret(mL, -1);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
	return ret;
}

void LuaObject::SetField(const char* fieldName, double num) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, double num)");
	PushToStack();
	lua_pushnumber(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, int num) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, int num)");
	PushToStack();
	lua_pushinteger(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, unsigned num) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, unsigned num)");
	PushToStack();
	lua_pushunsigned(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, bool b) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, bool b)");
	PushToStack();
	lua_pushboolean(mL, b);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const char* str) const
{
	assert(str); assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const char* str)");
	PushToStack();
	lua_pushstring(mL, str);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec3Tuple& v) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec3& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec3ITuple& v) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec3I& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec4Tuple& v) const{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec4& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec2Tuple& v) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec2& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const QuatTuple& v) const // Quat
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec2& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec2ITuple& v) const{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec2& v)");
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const TransformationTuple& t) const
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetField(const char* fieldName, const Vec2& v)");
	PushToStack();
	luaU_push(mL, t);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, LuaObject& value) const{
	if (value.IsTable()){
		auto newTable = SetFieldTable(fieldName);
		newTable.AppendTable(value);
	}
	else if (value.IsBool()){
		SetField(fieldName, value.GetBoolWithDef());
	}
	else if (value.IsNumber()){
		SetField(fieldName, value.GetDouble());
	}
	else if (value.IsString()){
		SetField(fieldName, value.GetString().c_str());
	}
	else{
		assert(0);
	}
}

void LuaObject::SetField(LuaObject& key, LuaObject& value) const{
	if (key.IsString()){
		SetField(key.GetString().c_str(), value);
	}
	else{
		SetSeq(key.GetUnsigned(), value);
	}
}

LuaObject LuaObject::SetSeqTable(int n) const
{
	LUA_STACK_WATCHER w(mL, "LuaObject LuaObject::SetSeqTable(int n) const");
	PushToStack();
	lua_newtable(mL);
	LuaObject ret(mL, -1);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
	return ret;
}

LuaObject LuaObject::GetSeqTable(int n)
{
	LUA_STACK_WATCHER w(mL, "LuaObject LuaObject::GetSeqTable(int n)");
	PushToStack();
	lua_rawgeti(mL, -1, n);
	LuaObject ret(mL, -1, true);
	lua_pop(mL, 1);
	return ret;
}

void LuaObject::SetSeq(int n, const char* str)  const
{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, const char* str)");
	PushToStack();
	lua_pushstring(mL, str);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, char* str) const
{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, char* str)");
	PushToStack();
	lua_pushstring(mL, str);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, unsigned num) const
{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, unsigned num)");
	PushToStack();
	lua_pushunsigned(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, int num) const{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, int num)");
	PushToStack();
	lua_pushinteger(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, float num) const
{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, float num)");
	PushToStack();
	lua_pushnumber(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, double num) const{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, double num)");
	PushToStack();
	lua_pushnumber(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, const Vec4Tuple& val) const
{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, const Vec4& val)");
	PushToStack();
	luaU_push<Vec4Tuple>(mL, val);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, const Vec3ITuple& val) const{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, const Vec3I& val)");
	PushToStack();
	luaU_push<Vec3ITuple>(mL, val);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, const Vec3Tuple& val) const{
	LUA_STACK_WATCHER w(mL, "void LuaObject::SetSeq(int n, const Vec3& val)");
	PushToStack();
	luaU_push<Vec3Tuple>(mL, val);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, LuaObject& value) const{
	if (value.IsTable()){
		auto newTable = SetSeqTable(n);;
		newTable.AppendTable(value);
	}
	else if (value.IsBool()){
		SetSeq(n, value.GetBoolWithDef());
	}
	else if (value.IsNumber()){
		SetSeq(n, value.GetDouble());
	}
	else if (value.IsString()){
		SetSeq(n, value.GetString().c_str());
	}
	else{
		assert(0);
	}
}


void LuaObject::AppendTable(LuaObject& table) const{
	assert(table.IsTable());
	if (!table.IsTable())
		return;

	auto it = table.GetTableIterator();
	LuaTableIterator::KeyValue kv;
	while (it.GetNext(kv)){
		SetField(kv.first, kv.second);
	}
}

double LuaObject::GetNumberAt(int index) const
{
	LUA_STACK_WATCHER watcher(mL, "double LuaObject::GetNumberAt(int index) const");
	PushToStack();
	lua_rawgeti(mL, -1, index);
	double number = lua_tonumber(mL, -1);
	lua_pop(mL, 2);
	return number;
}

unsigned LuaObject::GetUnsignedAt(int index) const
{
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsignedAt(int index) const");
	PushToStack();
	lua_rawgeti(mL, -1, index);
	if (lua_isnumber(mL, -1))
	{
		unsigned v = lua_tounsigned(mL, -1);
		lua_pop(mL, 2);
		return v;
	}
	else{
		Logger::Log(FB_ERROR_LOG_ARG, "No element found.");
	}
	return 0;
}

LuaObject LuaObject::GetTableAt(int index) const
{
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsignedAt(int index) const");
	PushToStack();
	lua_rawgeti(mL, -1, index);
	if (lua_istable(mL, -1))
	{
		LuaObject table(mL, -1);
		unsigned v = lua_tounsigned(mL, -1);
		lua_pop(mL, 2);
		return table;
	}
	lua_pop(mL, 2);
	return LuaObject();
}

const char* LuaObject::GetStringAt(int index) const{
	LUA_STACK_WATCHER watcher(mL, __FUNCTION__);
	PushToStack();
	lua_rawgeti(mL, -1, index);
	if (lua_isstring(mL, -1))
	{
		auto str = lua_tostring(mL, -1);
		lua_pop(mL, 2);
		return str;
	}
	else{
		Logger::Log(FB_ERROR_LOG_ARG, "No element found.");
	}
	return "";
}

std::string LuaObject::GetString() const{
	return GetString(std::string());
}

//----------------------------------------------------------------
std::string LuaObject::GetString(std::string& def) const
{
	if (!IsString())
		return def;
	LUA_STACK_WATCHER watcher(mL, "std::string LuaObject::GetString(std::string& def) const");
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	return ret;
}

std::string LuaObject::GetString(bool& success) const
{
	if (!IsString())
	{
		success = false;
		return std::string();
	}
	LUA_STACK_WATCHER watcher(mL, "std::string LuaObject::GetString(bool& success) const");
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	success = true;
	return ret;
}

float LuaObject::GetFloat() const{
	return GetFloat(0.f);
}

float	LuaObject::GetFloat(float def) const
{
	if (!IsNumber())
		return def;
	LUA_STACK_WATCHER watcher(mL, "float	LuaObject::GetFloat(float def) const");
	PushToStack();
	float f = (float)lua_tonumber(mL, -1);
	lua_pop(mL, 1);
	return f;
}

double LuaObject::GetDouble() const{
	return GetDouble(0.);
}

double LuaObject::GetDouble(double def) const{
	if (!IsNumber())
		return def;
	LUA_STACK_WATCHER watcher(mL, "float	LuaObject::GetDouble(double def) const");
	PushToStack();
	double d = lua_tonumber(mL, -1);
	lua_pop(mL, 1);
	return d;
}

float LuaObject::GetFloat(bool& success) const
{
	if (!IsNumber())
	{
		success = false;
		return 0.f;
	}
	LUA_STACK_WATCHER watcher(mL, "float LuaObject::GetFloat(bool& success) const");
	PushToStack();
	float f = (float)lua_tonumber(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return f;
}

int LuaObject::GetInt() const{
	return GetInt(0);
}

int LuaObject::GetInt(int def) const
{
	if (!IsNumber())
		return def;

	LUA_STACK_WATCHER watcher(mL, "int LuaObject::GetInt(int def) const");
	PushToStack();
	int i = lua_tointeger(mL, -1);
	lua_pop(mL, 1);
	return i;
}

int LuaObject::GetInt(bool& success) const
{
	if (!IsNumber())
	{
		success = false;
		return 0;
	}

	LUA_STACK_WATCHER watcher(mL, "int LuaObject::GetInt(bool& success) const");
	PushToStack();
	int i = lua_tointeger(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return i;
}

unsigned LuaObject::GetUnsigned() const{
	return GetUnsigned(0);
}

unsigned LuaObject::GetUnsigned(unsigned def) const
{
	if (!IsNumber())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsigned(unsigned def) const");
	PushToStack();
	unsigned u = lua_tounsigned(mL, -1);
	lua_pop(mL, 1);
	return u;
}

unsigned LuaObject::GetUnsigned(bool& success) const
{
	if (!IsNumber())
	{
		success = false;
		return 0;
	}
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsigned(bool& success) const");
	PushToStack();
	unsigned u = lua_tounsigned(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return u;
}

unsigned LuaObject::GetUnsignedFromString() const{
	return GetUnsignedFromString(0);
}

unsigned LuaObject::GetUnsignedFromString(unsigned def) const
{
	if (!IsString())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsignedFromString(unsigned def) const");
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	return StringConverter::ParseUnsignedInt(ret.c_str(), def);
}

unsigned LuaObject::GetUnsignedFromString(bool& success) const
{
	if (!IsString())
	{
		success = false;
		return 0;
	}
	LUA_STACK_WATCHER watcher(mL, "unsigned LuaObject::GetUnsignedFromString(bool& success) const");
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	success = true;
	return StringConverter::ParseUnsignedInt(ret.c_str());
}

bool LuaObject::GetBoolWithDef() const{
	return GetBoolWithDef(0);
}

bool LuaObject::GetBoolWithDef(bool def) const
{
	if (!IsBool())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "bool LuaObject::GetBoolWithDef(bool def) const");
	PushToStack();
	bool b = lua_toboolean(mL, -1) != 0;
	lua_pop(mL, 1);
	return b;
}

bool LuaObject::GetBool(bool& success) const
{
	if (!IsBool())
	{
		success = false;
		return false;
	}
	LUA_STACK_WATCHER watcher(mL, "bool LuaObject::GetBool(bool& success) const");
	PushToStack();
	bool b = lua_toboolean(mL, -1) != 0;
	lua_pop(mL, 1);
	success = true;
	return b;
}

Vec3Tuple LuaObject::GetVec3() const{
	return GetVec3(Vec3Tuple(0.f, 0.f, 0.f));
}

Vec3Tuple LuaObject::GetVec3(const Vec3Tuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Vec3 LuaObject::GetVec3(const Vec3& def) const");
	PushToStack();
	Vec3Tuple ret = luaU_check<Vec3Tuple>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec3Tuple LuaObject::GetVec3(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec3Tuple(0.f, 0.f, 0.f);
	}
	LUA_STACK_WATCHER watcher(mL, "Vec3 LuaObject::GetVec3(bool& success) const");
	PushToStack();
	Vec3Tuple ret = luaU_check<Vec3Tuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec4Tuple LuaObject::GetVec4() const{
	return GetVec4(Vec4Tuple(0.f, 0.f, 0.f, 0.f));
}

Vec4Tuple LuaObject::GetVec4(const Vec4Tuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Vec4 LuaObject::GetVec4(const Vec4& def) const");
	PushToStack();
	Vec4Tuple ret = luaU_check<Vec4Tuple>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec4Tuple LuaObject::GetVec4(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec4Tuple(0.f, 0.f, 0.f, 0.f);
	}
	LUA_STACK_WATCHER watcher(mL, "Vec4 LuaObject::GetVec4(bool& success) const");
	PushToStack();
	Vec4Tuple ret = luaU_check<Vec4Tuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec3ITuple LuaObject::GetVec3I() const{
	return GetVec3I(Vec3ITuple(0, 0, 0));
}

Vec3ITuple LuaObject::GetVec3I(const Vec3ITuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Vec3I LuaObject::GetVec3I(const Vec3I& def) const");
	PushToStack();
	Vec3ITuple ret = luaU_check<Vec3ITuple>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec3ITuple LuaObject::GetVec3I(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec3ITuple(0, 0, 0);
	}
	LUA_STACK_WATCHER watcher(mL, "Vec3I LuaObject::GetVec3I(bool& success) const");
	PushToStack();
	Vec3ITuple ret = luaU_check<Vec3ITuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec2Tuple LuaObject::GetVec2() const{
	return GetVec2(Vec2Tuple(0.f, 0.f));
}

Vec2Tuple LuaObject::GetVec2(const Vec2Tuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Vec2 LuaObject::GetVec2(const Vec2& def) const");
	PushToStack();
	Vec2Tuple ret = luaU_check<Vec2Tuple>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec2Tuple LuaObject::GetVec2(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec2Tuple(0.f, 0.f);
	}
	LUA_STACK_WATCHER watcher(mL, "Vec2 LuaObject::GetVec2(bool& success) const");
	PushToStack();
	Vec2Tuple ret = luaU_check<Vec2Tuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec2ITuple LuaObject::GetVec2I() const{
	return GetVec2I(Vec2ITuple(0, 0));
}


Vec2ITuple LuaObject::GetVec2I(const Vec2ITuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Vec2I LuaObject::GetVec2I(const Vec2I& def) const");
	PushToStack();
	Vec2ITuple ret = luaU_check<Vec2ITuple>(mL, -1);
	lua_pop(mL, 1);

	return ret;
}

Vec2ITuple LuaObject::GetVec2I(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec2ITuple(0, 0);
	}
	LUA_STACK_WATCHER watcher(mL, "Vec2I LuaObject::GetVec2I(bool& success) const");
	PushToStack();
	Vec2ITuple ret = luaU_check<Vec2ITuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;

	return ret;
}

QuatTuple LuaObject::GetQuat()const{
	return GetQuat(QuatTuple(0.f, 0.f, 0.f, 0.f));
}

QuatTuple LuaObject::GetQuat(const QuatTuple& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Quat LuaObject::GetQuat(const Quat& def) const");
	PushToStack();
	QuatTuple ret = luaU_check<QuatTuple>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

TransformationTuple LuaObject::GetTransformation() const{
	return GetTransformation(TransformationTuple(
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f,
		1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f,
		1.f, 1.f, 1.f,
		true, true, true
		));
}

TransformationTuple LuaObject::GetTransformation(const TransformationTuple& def) const{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL, "Transformation LuaObject::GetTransformation(const Transformation& def) const");
	PushToStack();
	TransformationTuple ret = luaU_check<TransformationTuple>(mL, -1);
	lua_pop(mL, 1);

	return ret;
}

QuatTuple LuaObject::GetQuat(bool& success)const
{
	if (!IsTable())
	{
		success = false;
		return QuatTuple(1., 0., 0., 0.);
	}
	LUA_STACK_WATCHER watcher(mL, "Quat LuaObject::GetQuat(bool& success)const");
	PushToStack();
	QuatTuple ret = luaU_check<QuatTuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;

	return ret;
}

TransformationTuple LuaObject::GetTransformation(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return TransformationTuple(
			1.f, 0.f, 0.f,
			0.f, 1.f, 0.f,
			0.f, 0.f, 1.f,
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f,
			1.f, 1.f, 1.f,
			true, true, true
			);
	}
	LUA_STACK_WATCHER watcher(mL, "Transformation LuaObject::GetTransformation(bool& success) const");
	PushToStack();
	TransformationTuple ret = luaU_check<TransformationTuple>(mL, -1);
	lua_pop(mL, 1);
	success = true;

	return ret;
}

void LuaObject::Clear()
{
	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	FB_SAFE_DELETE(mSelf);
}

unsigned LuaObject::GetLen() const
{
	LUA_STACK_CLIPPER w(mL);
	PushToStack();
	lua_len(mL, -1);
	unsigned len = luaL_checkunsigned(mL, -1);
	return len;
}

bool LuaObject::operator == (const LuaObject& other) const
{
	if (mRef == other.mRef && mType == other.mType)
	{
		return true;
	}
	if (mRef == LUA_NOREF || other.mRef == LUA_NOREF)
		return false;

	lua_rawgeti(mL, LUA_REGISTRYINDEX, mRef);
	lua_rawgeti(mL, LUA_REGISTRYINDEX, other.mRef);
	bool equal = lua_rawequal(mL, -1, -2) == 1;
	lua_pop(mL, 2);

	return equal;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LuaTableIterator::LuaTableIterator(const LuaObject& table)
{
	assert(table.IsTable());
	mL = table.GetLuaState();
	table.PushToStack();
	lua_pushnil(mL); // T, nil
}

LuaTableIterator::~LuaTableIterator()
{
	lua_pop(mL, 1); // remove table from the stack
}

bool LuaTableIterator::GetNext(KeyValue& outKeyValue)
{
	bool result = lua_next(mL, -2) != 0;
	if (result)
	{
		outKeyValue.first = LuaObject(mL, -2);
		outKeyValue.second = LuaObject(mL, -1);
		lua_pop(mL, 1);
	}

	return result;
}

//---------------------------------------------------------------------------
LuaSequenceIterator::LuaSequenceIterator(const LuaObject& sequence)
	: mCurIdx(0)
{
	assert(sequence.IsTable());
	mL = sequence.GetLuaState();
	sequence.PushToStack();
	mLen = luaL_len(mL, -1);
}

LuaSequenceIterator::~LuaSequenceIterator()
{
	lua_pop(mL, 1); // remove sequence from the stack
}

bool LuaSequenceIterator::GetNext(LuaObject& out)
{
	++mCurIdx;
	if (mCurIdx > mLen)
		return false;

	lua_rawgeti(mL, -1, mCurIdx);
	out = LuaObject(mL, -1);
	lua_pop(mL, 1);
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
fb::LuaObject fb::GetLuaVar(lua_State* L, const char* var, const char* file)
{
	if (!var)
		return LuaObject();
	auto splitted = Split(var, ".");
	if (!LuaUtils::CheckLuaGlobalExist(L, splitted[0].c_str()))
	{
		if (!file)
		{
			return LuaObject();
		}

		if (luaL_dofile(L, file))
		{
			Logger::Log(FB_ERROR_LOG_ARG, lua_tostring(L, -1));
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Script error! %s", file).c_str());
			lua_pop(L, 1);
			return LuaObject();
		}
	}

	LuaObject ret(L, splitted[0].c_str());
	if (ret.IsValid())
	{
		auto depth = splitted.size();
		for (size_t i = 1; i < depth; i++)
		{
			ret = ret.GetField(splitted[i].c_str());
			if (!ret.IsValid())
				break;
		}
	}
	return ret;
}

bool LuaObject::HasFunction() const
{
	if (IsFunction())
		return true;

	if (!IsTable())
		return false;

	auto it = GetTableIterator();
	LuaTableIterator::KeyValue kv;
	while (it.GetNext(kv))
	{
		if (kv.first.IsFunction())
			return true;
		if (kv.second.IsFunction())
			return true;

		if (kv.first.IsTable())
		{
			bool has = kv.first.HasFunction();
			if (has)
				return true;
		}

		if (kv.second.IsTable())
		{
			bool has = kv.second.HasFunction();
			if (has)
				return true;
		}
	}
	return false;
}

unsigned LuaObject::GetElementCount() const{
	if (!IsTable()){
		Logger::Log(FB_ERROR_LOG_ARG, "Not a table.");
		return 0;
	}

	auto iter = GetTableIterator();
	LuaTableIterator::KeyValue kv;
	unsigned num = 0;
	while (iter.GetNext(kv)){
		++num;
	}
	return num;
}