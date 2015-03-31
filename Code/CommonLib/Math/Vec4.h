#pragma once

#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{
	class Vec4
	{
	public:
		Vec4() {}
		Vec4(float _x, float _y, float _z, float _w)
			: x(_x), y(_y), z(_z), w(_w)
		{			
		}
		Vec4(const Vec3& xyz, float _w)
			: x(xyz.x), y(xyz.y), z(xyz.z), w(_w)
		{
		}
		Vec4(const Vec3& xyz)
			:x(xyz.x), y(xyz.y), z(xyz.z), w(1.f)
		{}

		Vec4(const char* s);

		float Dot( const Vec4& other ) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		inline Vec4 operator* (float scalar) const
		{
			return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
		}

		inline Vec4& operator*= (float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		inline Vec4& operator*= (const Vec4& r)
		{
			x *= r.x;
			y *= r.y;
			z *= r.z;
			w *= r.w;
			return *this;
		}

		inline Vec4& operator/=(float scalar)
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}

		inline Vec4 operator* (const Vec4& r) const
		{
			return Vec4(x*r.x, y*r.y, z*r.z, w*r.w);
		}

		inline Vec4 operator+ (const Vec4& r) const
		{
			return Vec4(x + r.x, y + r.y, z + r.z, w + r.w);
		}

		inline Vec4 operator+(float v)
		{
			return Vec4(v +x, v + y, v + z, v + w);
		}

		bool operator== (const Vec4& other) const
		{
			return (IsEqual(x, other.x) && IsEqual(y, other.y) && IsEqual(z, other.z) && IsEqual(w, other.w));
		}

		inline void SetXYZ(const Vec3& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}

		inline Vec3 ToVec3() const
		{
			return Vec3(x, y, z);
		}

	public:
		float x, y, z, w;

		static const Vec4 UNIT_X;
		static const Vec4 UNIT_Y;
		static const Vec4 UNIT_Z;
		static const Vec4 ZERO;
	};
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Vec4>
{
	static fastbird::Vec4 luaU_check(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec4 luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		fastbird::Vec4 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);
		ret.w = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static fastbird::Vec4 luaU_to(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec4 luaU_to(lua_State* L, int index)");
		fastbird::Vec4 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);
		ret.w = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Vec4& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, val.z);
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, val.w);
		lua_rawseti(L, -2, 4);
	}

	static void luaU_push(lua_State* L, fastbird::Vec4& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, val.z);
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, val.w);
		lua_rawseti(L, -2, 4);
	}
};