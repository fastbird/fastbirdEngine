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
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/SpinLock.h"
#include "LuaUtils.h"
namespace fb
{
	class FB_DLL_LUA LuaObject;
	//-----------------------------------------------------------------------------
	class FB_DLL_LUA LuaTableIterator
	{
		lua_State* mL;
	public:

		LuaTableIterator(const LuaObject& table);
		~LuaTableIterator();
		typedef std::pair<LuaObject, LuaObject> KeyValue;
		bool GetNext(KeyValue& outKeyValue);
	};

	//-----------------------------------------------------------------------------
	class FB_DLL_LUA LuaSequenceIterator
	{
		lua_State* mL;
		size_t mLen;
		size_t mCurIdx;

	public:
		LuaSequenceIterator(const LuaObject& sequence);
		~LuaSequenceIterator();
		bool GetNext(LuaObject& out);

	};

	//-----------------------------------------------------------------------------
	class FB_DLL_LUA LuaObject
	{
		int mRef;
		lua_State* mL;
		int mType;
		std::string mName;
		LuaObject* mSelf;
		static VectorMap<int, unsigned> sUsedCount;
		static SpinLock<true, false> sUsedCountGuard;

		static void AddUsedCount(int ref);
		static bool ReleaseUsedCount(int ref);

	public:
		LuaObject();
		LuaObject(lua_State* L);
		// index will not be popped.
		LuaObject(lua_State* L, int index, bool pop = false);
		LuaObject(lua_State* L, const char* globalName);
		LuaObject(const char* globalName);
		LuaObject(const LuaObject& other);
		LuaObject& operator=(const LuaObject& other);
		~LuaObject();

		void SetSelf(const LuaObject& other);
		void FindFunction(lua_State* L, const char* functName);

		lua_State* GetLuaState() const { return mL; }

		void NewTable(lua_State* L);

		void SetGlobalName(lua_State* L, const char* globalName);
		const char* GetGlobalName() const { return mName.c_str(); }
		bool IsFunction() const;
		bool IsMethod() const; // A method is also a function.
		bool IsTable() const;
		bool IsString() const;
		bool IsNumber() const;
		bool IsBool() const;
		bool IsNil() const;
		bool IsValid(bool nilIsValid = false) const;
		unsigned GetType() const { return mType; }
		LuaObject GetField(const char* fieldName) const;
		LuaTableIterator GetTableIterator() const;
		LuaSequenceIterator GetSequenceIterator() const;

		LuaObject SetFieldTable(const char* fieldName) const;
		void SetFieldTable(const char* fieldName, LuaObject& tableObj) const;
		void SetField(const char* fieldName, double num) const;
		void SetField(const char* fieldName, int num) const;
		void SetField(const char* fieldName, unsigned num) const;
		void SetField(const char* fieldName, bool b) const;
		void SetField(const char* fieldName, const char* str) const;
		void SetField(const char* fieldName, const Vec3Tuple& v) const; /// Vec3
		void SetField(const char* fieldName, const Vec3ITuple& v) const; /// Vec3I
		void SetField(const char* fieldName, const Vec4Tuple& v) const; // Vec4
		void SetField(const char* fieldName, const Vec2Tuple& v) const; // Vec2
		void SetField(const char* fieldName, const QuatTuple& v) const; // Quat
		void SetField(const char* fieldName, const std::tuple<int, int>& v) const; // Vec2I
		void SetField(const char* fieldName, const TransformationTuple& v) const; // Transformation
		void SetField(const char* fieldName, LuaObject& value) const;
		void SetField(LuaObject& key, LuaObject& value) const;

		LuaObject SetSeqTable(int n) const;
		LuaObject GetSeqTable(int n);
		void SetSeq(int n, const char* str)  const;
		void SetSeq(int n, char* str) const;
		void SetSeq(int n, unsigned num) const;
		void SetSeq(int n, int num) const;
		void SetSeq(int n, float num) const;
		void SetSeq(int n, double num) const;
		void SetSeq(int n, const Vec4Tuple& val) const;
		void SetSeq(int n, const Vec3ITuple& val) const;
		void SetSeq(int n, const Vec3Tuple& val) const;
		void SetSeq(int n, LuaObject& value) const;
		void SetSeqNil(int n) const;
		
		template<class T>
		void SetSeq(int n, T* val) const
		{
			LUA_STACK_WATCHER w(mL, "void SetSeq(int n, T* val)");
			PushToStack();
			luaW_push(mL, val);
			LuaUtils::rawseti(mL, -2, n);
			LuaUtils::pop(mL, 1);
		}
		template <class T>
		void SetSeqTemplate(int n, T v) const
		{
			LUA_STACK_WATCHER w(mL, "void SetSeqTemplate(int n, T v)");
			PushToStack();
			luaW_push(mL, v);
			LuaUtils::rawseti(mL, -2, n);
			LuaUtils::pop(mL, 1);
		}

		void AppendTable(LuaObject& table) const;

		double GetNumberAt(int index) const;
		unsigned GetUnsignedAt(int index) const;
		int GetIntAt(int index) const;
		LuaObject GetTableAt(int index) const;
		const char* GetStringAt(int index) const;
		std::string GetString() const;
		std::string GetString(std::string& def) const;
		float GetFloat() const;
		float GetFloat(float def) const;
		double GetDouble() const;
		double GetDouble(double def) const;
		int GetInt() const;
		int GetInt(int def) const;
		unsigned GetUnsigned() const;
		unsigned GetUnsigned(unsigned def) const;
		unsigned GetUnsignedFromString() const;
		unsigned GetUnsignedFromString(unsigned def) const;
		bool GetBoolWithDef() const;
		bool GetBoolWithDef(bool def) const;

		template <class T>
		T* GetUserdata() const {
			LUA_STACK_WATCHER watcher(mL, __FUNCTION__);
			PushToStack();
			auto userdata = luaW_check<T>(mL, -1);
			LuaUtils::pop(mL, 1);
			return userdata;
		}

		Vec3Tuple GetVec3() const;
		Vec3Tuple GetVec3(const Vec3Tuple& def) const;
		Vec4Tuple GetVec4() const;
		Vec4Tuple GetVec4(const Vec4Tuple& def) const;
		Vec3ITuple GetVec3I() const;
		Vec3ITuple GetVec3I(const Vec3ITuple& def) const;
		Vec2Tuple GetVec2() const;
		Vec2Tuple GetVec2(const Vec2Tuple& def) const;
		Vec2ITuple GetVec2I() const;
		Vec2ITuple GetVec2I(const Vec2ITuple& def) const;
		QuatTuple GetQuat() const;
		QuatTuple GetQuat(const QuatTuple& def)const;
		TransformationTuple GetTransformation() const;
		TransformationTuple GetTransformation(const TransformationTuple& def) const;

		std::string GetString(bool& success) const;
		float GetFloat(bool& success) const;
		int GetInt(bool& success) const;
		unsigned GetUnsigned(bool& success) const;
		unsigned GetUnsignedFromString(bool& success) const;
		bool GetBool(bool& success) const;
		Vec3Tuple GetVec3(bool& success) const;
		Vec4Tuple GetVec4(bool& success) const;
		Vec3ITuple GetVec3I(bool& success) const;
		Vec2Tuple GetVec2(bool& success) const;
		Vec2ITuple GetVec2I(bool& success) const;
		QuatTuple GetQuat(bool& success)const;
		TransformationTuple GetTransformation(bool& success) const;


		// this is for table
		unsigned GetElementCount() const;

		void PushToStack() const;
		bool Call();
		bool CallWithManualArgs(unsigned numArgs, unsigned numRets);

		void Clear();

		unsigned GetLen() const;

		bool operator==(const LuaObject& other) const;

		bool HasFunction() const;

	private:
		void CheckType();
	};

	FB_DLL_LUA LuaObject GetLuaVar(lua_State* L, const char* var, const char* file = 0);

}