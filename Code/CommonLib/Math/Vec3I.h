#pragma once
#include <CommonLib/Math/Vec3.h>
namespace fastbird
{
	class Vec3I
	{
	public:
		int x, y, z;

		Vec3I()
		{
		}

		Vec3I(int _x, int _y, int _z)
			: x(_x), y(_y), z(_z)
		{
		}

		Vec3I(const Vec3& v)
		{
			x = (int)v.x;
			y = (int)v.y;
			z = (int)v.z;
		}

		//-------------------------------------------------------------------
		Vec3I operator* (int s) const
		{
			return Vec3I(x*s, y*s, z*s);
		}

		Vec3 operator*(float s) const
		{
			return Vec3(x*s, y*s, z*s);
		}

		Vec3I operator/ (int s) const
		{
			return Vec3I(x/s, y/s, z*s);
		}

		Vec3I operator+ (int s) const
		{
			return Vec3I(x+s, y+s, z+s);
		}

		Vec3I operator- (int s) const
		{
			return Vec3I(x-s, y-s, y-s);
		}

		//-------------------------------------------------------------------
		Vec3I operator* (const Vec3I& v) const
		{
			return Vec3I(x*v.x, y*v.y, z*v.z);
		}

		Vec3I operator/ (const Vec3I& v) const
		{
			return Vec3I(x/v.x, y/v.y, z/v.z);
		}

		Vec3I operator+ (const Vec3I& v) const
		{
			return Vec3I(x+v.x, y+v.y, z+v.z);
		}

		Vec3I operator- (const Vec3I& v) const
		{
			return Vec3I(x-v.x, y-v.y, z-v.z);
		}

		//-------------------------------------------------------------------
		bool operator== (const Vec3I& v) const
		{
			return x == v.x && y == v.y && z==v.z;
		}

		bool operator!=(const Vec3I& v) const
		{
			return !operator==(v);
		}

		float length() const
		{
			return (float)sqrt(x*x + y*y + z*z);
		}

		float lengthSQ() const
		{
			return (float)(x*x + y*y + z*z);
		}

		static const Vec3I UNIT_X;
		static const Vec3I UNIT_Y;
		static const Vec3I UNIT_Z;
		static const Vec3I ZERO;
	};
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Vec3I>
{
	static fastbird::Vec3I luaU_check(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec3I luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		fastbird::Vec3I ret;
		lua_rawgeti(L, index, 1);
		ret.x = luaL_checkint(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = luaL_checkint(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = luaL_checkint(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static fastbird::Vec3I luaU_to(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec3I luaU_to(lua_State* L, int index)");
		fastbird::Vec3I ret;
		lua_rawgeti(L, index, 1);
		ret.x = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Vec3I& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushinteger(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, val.y);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, val.z);
		lua_rawseti(L, -2, 3);
	}
};