#pragma once
#include <CommonLib/VectorMap.h>
#include <CommonLib/LuaUtils.h>
#include <CommonLib/SpinLock.h>
namespace fastbird
{
	class LuaObject;
	//-----------------------------------------------------------------------------
	class LuaTableIterator
	{
		lua_State* mL;
	public:

		LuaTableIterator(const LuaObject& table);
		~LuaTableIterator();
		typedef std::pair<LuaObject, LuaObject> KeyValue;
		bool GetNext(KeyValue& outKeyValue);
	};

	//-----------------------------------------------------------------------------
	class LuaSequenceIterator
	{
		lua_State* mL;
		size_t mLen;
		size_t mCurIdx;

	public :
		LuaSequenceIterator(const LuaObject& sequence);
		~LuaSequenceIterator();
		bool GetNext(LuaObject& out);

	};

	//-----------------------------------------------------------------------------
	class LuaObject
	{
		int mRef;
		lua_State* mL;
		int mType;
		std::string mName;
		LuaObject* mSelf;
		static fastbird::VectorMap<int, unsigned> sUsedCount;
		static fastbird::SpinLock<true, false> sUsedCountGuard;

		static void AddUsedCount(int ref);
		static bool ReleaseUsedCount(int ref);

	public:
		LuaObject();
		LuaObject(lua_State* L);
		// index will not be popped.
		LuaObject(lua_State* L, int index, bool pop = false);
		LuaObject(lua_State* L, const char* globalName);
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

		LuaObject SetFieldTable(const char* fieldName);
		void SetField(const char* fieldName, double num);
		void SetField(const char* fieldName, int num);
		void SetField(const char* fieldName, unsigned num);
		void SetField(const char* fieldName, bool b);
		void SetField(const char* fieldName, const char* str);
		void SetField(const char* fieldName, const Vec3& v);
		void SetField(const char* fieldName, const Vec3I& v);
		void SetField(const char* fieldName, const Vec2& v);
		void SetField(const char* fieldName, const Vec2I& v);
		void SetField(const char* fieldName, const Transformation& t);

		LuaObject SetSeqTable(int n) const;
		LuaObject GetSeqTable(int n);
		void SetSeq(int n, const char* str);
		void SetSeq(int n, char* str);
		void SetSeq(int n, unsigned num);
		void SetSeq(int n, float num);
		void SetSeq(int n, const Vec4& val);
		template<class T>
		void SetSeq(int n, T* val)
		{
			LUA_STACK_WATCHER w(mL, "void SetSeq(int n, T* val)");
			PushToStack();
			luaW_push(mL, val);
			lua_rawseti(mL, -2, n);
			lua_pop(mL, 1);
		}
		template <class T>
		void SetSeqTemplate(int n, T v)
		{
			LUA_STACK_WATCHER w(mL, "void SetSeqTemplate(int n, T v)");
			PushToStack();
			luaW_push(mL, v);
			lua_rawseti(mL, -2, n);
			lua_pop(mL, 1);
		}

		double GetNumberAt(int index) const;
		unsigned GetUnsignedAt(int index) const;
		LuaObject GetTableAt(int index) const;
		std::string GetString(std::string& def = std::string()) const;		
		float GetFloat(float def = 0.f) const;
		int GetInt(int def = 0) const;
		unsigned GetUnsigned(unsigned def = 0) const;
		unsigned GetUnsignedFromString(unsigned def = 0) const;
		bool GetBoolWithDef(bool def = false) const;
		Vec3 GetVec3(const Vec3& def = Vec3::ZERO) const;
		Vec4 GetVec4(const Vec4& def = Vec4::ZERO) const;
		Vec3I GetVec3I(const Vec3I& def = Vec3I(0, 0, 0)) const;
		Vec2 GetVec2(const Vec2& def = Vec2(0, 0)) const;
		Vec2I GetVec2I(const Vec2I& def = Vec2I(0, 0)) const;
		Quat GetQuat(const Quat& def = Quat())const;

		std::string GetString(bool& success) const;
		float GetFloat(bool& success) const;
		int GetInt(bool& success) const;
		unsigned GetUnsigned(bool& success) const;
		unsigned GetUnsignedFromString(bool& success) const;
		bool GetBool(bool& success) const;
		Vec3 GetVec3(bool& success) const;
		Vec4 GetVec4(bool& success) const;
		Vec3I GetVec3I(bool& success) const;
		Vec2 GetVec2(bool& success) const;
		Vec2I GetVec2I(bool& success) const;
		Quat GetQuat(bool& success)const;
		Transformation GetTransformation(bool& success) const;

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

	fastbird::LuaObject GetLuaVar(lua_State* L, const char* var, const char* file = 0);

}