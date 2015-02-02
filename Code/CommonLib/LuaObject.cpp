#include <CommonLib/StdAfx.h>
#include <CommonLib/LuaObject.h>
#include <CommonLib/StringUtils.h>
using namespace fastbird;

fastbird::VectorMap<int, unsigned> LuaObject::mUsedCount;

void LuaObject::AddUsedCount(int ref)
{
	if (ref == LUA_NOREF)
		return;
	mUsedCount[ref]++;
}
bool LuaObject::ReleaseUsedCount(int ref)
{
	if (ref == LUA_NOREF)
		return false;
	auto itFind = mUsedCount.Find(ref);
	assert(itFind != mUsedCount.end());
	if (--(itFind->second)==0)
	{
		return true; // remove it
	}

	return false; // still alive
}


LuaObject::LuaObject()
:LuaObject(0)
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
	lua_getglobal(L, globalName);
	CheckType();
	mRef = luaL_ref(L, LUA_REGISTRYINDEX);
	AddUsedCount(mRef);
	mName = globalName;
}

LuaObject::LuaObject(const LuaObject& other)
:LuaObject(other.mL)
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
	FB_SAFE_DEL(mSelf);

	if(ReleaseUsedCount(mRef))
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
	FB_SAFE_DEL(mSelf);
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
		assert(0 && "You need to put () at the end of the function name.");
	}
	auto splited2 = Split(splited[0].c_str(), ".");
	size = splited2.size();
	LuaObject obj(L, splited2[0].c_str());
	for (unsigned u = 1; u < size; ++u)
	{
		obj = obj.GetField(splited2[u].c_str());
	}
	FB_SAFE_DEL(mSelf);
	if (splited.size()==2)
	{
		*this = obj.GetField(splited[1].c_str());
		mSelf = FB_NEW(LuaObject)(obj);
	}
	else
	{
		*this = obj;
	}
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
	lua_rawgeti(mL, LUA_REGISTRYINDEX, mRef);
	if (mSelf)
	{
		assert(IsFunction());
		mSelf->PushToStack();
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
bool LuaObject::CallWithManualArgs(unsigned numArgs, unsigned numRets)
{
	LUA_PCALL_RET_FALSE(mL, numArgs, numRets);
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
	LUA_STACK_WATCHER watcher(mL);
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
LuaObject LuaObject::SetFieldTable(const char* fieldName)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_newtable(mL);
	LuaObject ret(mL, -1);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
	return ret;
}

void LuaObject::SetField(const char* fieldName, double num)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushnumber(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, int num)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushinteger(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, unsigned num)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushunsigned(mL, num);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, bool b)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushboolean(mL, b);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const char* str)
{
	assert(str); assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushstring(mL, str);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec3& v)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec3I& v)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

void LuaObject::SetField(const char* fieldName, const Vec2& v)
{
	assert(fieldName);
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	luaU_push(mL, v);
	lua_setfield(mL, -2, fieldName);
	lua_pop(mL, 1);
}

LuaObject LuaObject::SetSeqTable(int n) const
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_newtable(mL);
	LuaObject ret(mL, -1);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
	return ret;
}

LuaObject LuaObject::GetSeqTable(int n)
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_rawgeti(mL, -1, n);
	LuaObject ret(mL, -1, true);
	lua_pop(mL, 1);
	return ret;
}

void LuaObject::SetSeq(int n, const char* str)
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushstring(mL, str);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, unsigned num)
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushunsigned(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, float num)
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	lua_pushnumber(mL, num);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

void LuaObject::SetSeq(int n, const Vec4& val)
{
	LUA_STACK_WATCHER w(mL);
	PushToStack();
	luaU_push<Vec4>(mL, val);
	lua_rawseti(mL, -2, n);
	lua_pop(mL, 1);
}

double LuaObject::GetNumberAt(int index) const
{
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	lua_rawgeti(mL, -1, index);
	double number = lua_tonumber(mL, -1);
	lua_pop(mL, 2);
	return number;
}

//----------------------------------------------------------------
std::string LuaObject::GetString(std::string& def) const
{
	if(!IsString())
		return def;
	LUA_STACK_WATCHER watcher(mL);
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
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	success = true;
	return ret;
}



float	LuaObject::GetFloat(float def) const
{
	if(!IsNumber())
		return def;
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	float f = (float)lua_tonumber(mL, -1);
	lua_pop(mL, 1);
	return f;
}

float LuaObject::GetFloat(bool& success) const
{
	if (!IsNumber())
	{
		success = false;
		return 0.f;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	float f = (float)lua_tonumber(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return f;
}

int LuaObject::GetInt(int def) const
{
	if(!IsNumber())
		return def;

	LUA_STACK_WATCHER watcher(mL);
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

	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	int i = lua_tointeger(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return i;
}

unsigned LuaObject::GetUnsigned(unsigned def) const
{
	if (!IsNumber())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
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
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	unsigned u = lua_tounsigned(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return u;
}

unsigned LuaObject::GetUnsignedFromString(unsigned def) const
{
	if (!IsString())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	return StringConverter::parseUnsignedInt(ret.c_str(), def);
}

unsigned LuaObject::GetUnsignedFromString(bool& success) const
{
	if (!IsString())
	{
		success = false;
		return 0;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	std::string ret;
	const char* sz = lua_tostring(mL, -1);
	if (sz)
		ret = sz;
	lua_pop(mL, 1);
	success = true;
	return StringConverter::parseUnsignedInt(ret.c_str());
}

bool LuaObject::GetBoolWithDef(bool def) const
{
	if (!IsBool())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	bool b = lua_toboolean(mL, -1)!=0;
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
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	bool b = lua_toboolean(mL, -1) != 0;
	lua_pop(mL, 1);
	success = true;
	return b;
}

Vec3 LuaObject::GetVec3(const Vec3& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec3 ret = luaU_check<Vec3>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec3 LuaObject::GetVec3(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec3::ZERO;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec3 ret = luaU_check<Vec3>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec4 LuaObject::GetVec4(const Vec4& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec4 ret = luaU_check<Vec4>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec4 LuaObject::GetVec4(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec4::ZERO;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec4 ret = luaU_check<Vec4>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec3I LuaObject::GetVec3I(const Vec3I& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec3I ret = luaU_check<Vec3I>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec3I LuaObject::GetVec3I(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec3I::ZERO;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec3I ret = luaU_check<Vec3I>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec2 LuaObject::GetVec2(const Vec2& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec2 ret = luaU_check<Vec2>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Vec2 LuaObject::GetVec2(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec2::ZERO;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec2 ret = luaU_check<Vec2>(mL, -1);
	lua_pop(mL, 1);
	success = true;
	return ret;
}

Vec2I LuaObject::GetVec2I(const Vec2I& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec2I ret = luaU_check<Vec2I>(mL, -1);
	lua_pop(mL, 1);

	return ret;
}

Vec2I LuaObject::GetVec2I(bool& success) const
{
	if (!IsTable())
	{
		success = false;
		return Vec2I::ZERO;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Vec2I ret = luaU_check<Vec2I>(mL, -1);
	lua_pop(mL, 1);
	success = true;

	return ret;
}

Quat LuaObject::GetQuat(const Quat& def) const
{
	if (!IsTable())
	{
		return def;
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Quat ret = luaU_check<Quat>(mL, -1);
	lua_pop(mL, 1);
	return ret;
}

Quat LuaObject::GetQuat(bool& success)const
{
	if (!IsTable())
	{
		success = false;
		return Quat();
	}
	LUA_STACK_WATCHER watcher(mL);
	PushToStack();
	Quat ret = luaU_check<Quat>(mL, -1);
	lua_pop(mL, 1);
	success = true;

	return ret;
}

void LuaObject::Clear()
{
	if (ReleaseUsedCount(mRef))
		luaL_unref(mL, LUA_REGISTRYINDEX, mRef);

	FB_SAFE_DEL(mSelf);
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
fastbird::LuaObject fastbird::GetLuaVar(lua_State* L, const char* var, const char* file)
{
	if_assert_fail(var)
		return LuaObject();
	auto splitted = Split(var, ".");
	if (!CheckLuaGlobalExist(L, splitted[0].c_str()))
	{
		if (!file)
		{
			return LuaObject();
		}

		if (luaL_dofile(L, file))
		{
			Error(lua_tostring(L, -1));
			Error("Script error! %s", file);
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