#pragma once

#include <CommonLib/Math/Mat33.h>
#include <CommonLib/Math/Mat44.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Quat.h>
#include <CommonLib/Math/Plane3.h>
#include <CommonLib/Math/Ray3.h>
#include <CommonLib/luawrapperutil.hpp>

namespace fastbird
{
	class Plane;
	class Transformation
	{
	public:
		Transformation();
		~Transformation();

		Transformation(const Quat& q);

		void MakeIdentity ();
		void MakeUnitScale ();
		inline bool IsIdentity () const
		{
			return mIdentity;
		}
		inline bool IsRSSeperated() const
		{
			return mRSSeperated;
		}
		inline bool IsUniformScale () const
		{
			return mRSSeperated && mUniformScale;
		}

		void SetRotation (const Mat33& r);
		void SetRotation (const Quat& r);
		void SetDir(const Vec3& dir);
		void SetDirAndRight(const Vec3& dir, const Vec3& right);
		void AddRotation (const Quat& addR);
		inline const Quat& GetRotation() const
		{
			assert(mRSSeperated);
			return mR;
		}
		void SetMatrix (const Mat33& mat);
		inline const Mat33& GetMatrix () const
		{
			assert(mRSSeperated);
			return mMat;
		}
		void SetTranslation(const Vec3& t);
		void AddTranslation(const Vec3& addT);
		inline const Vec3& GetTranslation() const
		{
			return mT;
		}
		void SetScale (const Vec3& s);
		void AddScale(const Vec3& s);
		inline const Vec3& GetScale () const
		{
			assert(mRSSeperated);
			return mS;
		}
		void SetUniformScale (float fScale);
		inline float GetUniformScale () const
		{
			assert(mRSSeperated && mUniformScale);
			return mS.x;
		}

		float GetNorm () const;
		Vec3 ApplyForward (const Vec3& p) const;
		void ApplyForward (int iQuantity, const Vec3* points,
        Vec3* output) const;

		// X = M^{-1}*(Y-T) where Y is the input and X is the output.
		Vec3 ApplyInverse (const Vec3& p) const;
		void ApplyInverse (int iQuantity, const Vec3* points,
	        Vec3* output) const;
		Ray3 ApplyInverse(const Ray3& r) const;

		Vec3 InvertVector (const Vec3& v) const;

		Plane3 ApplyForward (const Plane3& p) const;
		AABB ApplyForward(const AABB& aabb) const;

		void Product (const Transformation& a, const Transformation& b);

		Transformation operator* (const Transformation& t) const;
		
		void Inverse (Transformation& t) const;
		void GetHomogeneous (Mat44& hm) const;

		Vec3 GetRight() const;
		Vec3 GetForward() const;
		Vec3 GetUp() const;	

		bool operator==(const Transformation& other) const;

		static const Transformation IDENTITY;

		// for serialization
		Mat33& _GetMat33() { return mMat; }
		Quat& _GetQuat() { return mR; }
		Vec3& _GetT() { return mT; }
		Vec3& _GetS() { return mS; }
		bool& _GetIdentity() { return mIdentity; }
		bool& _GetRSSeperated() { return mRSSeperated; }
		bool& _GetUniformScale() { return mUniformScale; }

	private:
		Mat33 mMat;
		Quat mR;
		Vec3 mT;
		Vec3 mS;
		bool mIdentity, mRSSeperated, mUniformScale;
	};
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Transformation>
{
	static fastbird::Transformation luaU_check(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Transformation luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		int n = 1;
		fastbird::Transformation ret;
		auto& mat33 = ret._GetMat33();
		for (int r = 0; r < 3; ++r)
			for (int c = 0; c < 3; ++c){
				lua_rawgeti(L, index, n++);
				mat33.m[r][c] = (float)luaL_checknumber(L, -1);
				lua_pop(L, 1);
			}

		auto& rot = ret._GetQuat();
		for (int i = 0; i < 4; ++i){
			lua_rawgeti(L, index, n++);
			*(&rot.w + i) = (float)luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}

		auto& t = ret._GetT();
		for (int i = 0; i < 3; ++i){
			lua_rawgeti(L, index, n++);
			*(&t.x + i) = (float)luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}

		auto& s = ret._GetS();
		for (int i = 0; i < 3; ++i){
			lua_rawgeti(L, index, n++);
			*(&s.x + i) = (float)luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}

		auto& bi = ret._GetIdentity();
		lua_rawgeti(L, index, n++);
		assert(lua_isboolean(L, -1));
		bi = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		auto& brs = ret._GetRSSeperated();
		lua_rawgeti(L, index, n++);		
		assert(lua_isboolean(L, -1));
		brs = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		auto& bu = ret._GetUniformScale();
		lua_rawgeti(L, index, n++);		
		assert(lua_isboolean(L, -1));
		bu = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		return ret;
	}

	static fastbird::Transformation luaU_to(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Transformation luaU_to(lua_State* L, int index)");
		int n = 1;
		fastbird::Transformation ret;
		auto& mat33 = ret._GetMat33();
		for (int r = 0; r < 3; ++r)
			for (int c = 0; c < 3; ++c){
				lua_rawgeti(L, index, n++);
				mat33.m[r][c] = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}

		auto& rot = ret._GetQuat();
		for (int i = 0; i < 4; ++i){
			lua_rawgeti(L, index, n++);
			*(&rot.w + i) = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);
		}

		auto& t = ret._GetT();
		for (int i = 0; i < 3; ++i){
			lua_rawgeti(L, index, n++);
			*(&t.x + i) = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);
		}

		auto& s = ret._GetS();
		for (int i = 0; i < 3; ++i){
			lua_rawgeti(L, index, n++);
			*(&s.x + i) = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);
		}

		auto& bi = ret._GetIdentity();
		lua_rawgeti(L, index, n++);
		bi = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		auto& brs = ret._GetRSSeperated();
		lua_rawgeti(L, index, n++);
		brs = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		auto& bu = ret._GetUniformScale();
		lua_rawgeti(L, index, n++);
		bu = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Transformation& val)
	{
		fastbird::Transformation& val2 = const_cast<fastbird::Transformation&>(val);
		luaU_push(L, val2);
	}

	static void luaU_push(lua_State* L, fastbird::Transformation& val)
	{
		lua_createtable(L, 22, 0);

		int n = 1;		
		auto& mat33 = val._GetMat33();
		for (int r = 0; r < 3; ++r)
			for (int c = 0; c < 3; ++c){
				lua_pushnumber(L, mat33.m[r][c]);
				lua_rawseti(L, -2, n++);
			}

		auto& rot = val._GetQuat();
		for (int i = 0; i < 4; ++i){
			lua_pushnumber(L, *(&rot.w+i));
			lua_rawseti(L, -2, n++);			
		}

		auto& t = val._GetT();
		for (int i = 0; i < 3; ++i){
			lua_pushnumber(L, *(&t.x + i));
			lua_rawseti(L, -2, n++);			
		}

		auto& s = val._GetS();
		for (int i = 0; i < 3; ++i){
			lua_pushnumber(L, *(&s.x + i));
			lua_rawseti(L, -2, n++);			
		}

		auto& bi = val._GetIdentity();
		lua_pushboolean(L, bi);
		lua_rawseti(L, -2, n++);		

		auto& brs = val._GetRSSeperated();
		lua_pushboolean(L, brs);
		lua_rawseti(L, -2, n++);

		auto& bu = val._GetUniformScale();
		lua_pushboolean(L, bu);
		lua_rawseti(L, -2, n++);
	}
};