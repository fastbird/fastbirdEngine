#pragma once
#include <CommonLib/luawrapperutil.hpp>
#include <CommonLib/Math/Vec2I.h>
namespace fastbird
{
	class Vec2
	{
	public:
		Vec2()
		{
		}

		Vec2(float _x, float _y)
			:x(_x), y(_y)
		{
		}

		Vec2(const Vec2I& v)
			:x((float)v.x), y((float)v.y)
		{

		}

		Vec2(const Vec2& other)
			: x(other.x), y(other.y)
		{
		}

		inline float Length() const
		{
			return sqrt(x*x + y*y);
		}

		inline float DistanceTo(const Vec2& other) const
		{
			return (*this - other).Length();
		}

		//-------------------------------------------------------------------
		Vec2 operator* (float s) const
		{
			return Vec2(x*s, y*s);
		}

		Vec2 operator/ (float s) const
		{
			return Vec2(x/s, y/s);
		}

		Vec2 operator+ (float s) const
		{
			return Vec2(x+s, y+s);
		}

		Vec2 operator- (float s) const
		{
			return Vec2(x-s, y-s);
		}

		//-------------------------------------------------------------------
		inline Vec2 operator- () const
		{
			return Vec2(-x, -y);
		}

		//-------------------------------------------------------------------
		Vec2 operator* (const Vec2& v) const
		{
			return Vec2(x*v.x, y*v.y);
		}

		Vec2& operator*= (float s)
		{
			x*=s;
			y*=s;
			return *this;
		}

		Vec2& operator+= (float s)
		{
			x+=s;
			y+=s;
			return *this;
		}

		Vec2& operator-= (float s)
		{
			x-=s;
			y-=s;
			return *this;
		}

		Vec2& operator+= (const Vec2& s)
		{
			x+=s.x;
			y+=s.y;
			return *this;
		}

		Vec2& operator/= (const Vec2& other)
		{
			x /= other.x;
			y /= other.y;
			return *this;
		}

		Vec2 operator/ (const Vec2& v) const
		{
			return Vec2(x/v.x, y/v.y);
		}

		Vec2 operator+ (const Vec2& v) const
		{
			return Vec2(x+v.x, y+v.y);
		}

		Vec2 operator- (const Vec2& v) const
		{
			return Vec2(x-v.x, y-v.y);
		}

		bool operator== (const Vec2& other) const
		{
			return x==other.x && y==other.y;
		}

		bool operator!= (const Vec2& other) const
		{
			return x != other.x || y != other.y;
		}

		inline float Normalize()
		{
			float length = sqrt(x*x + y*y);
			if (length > 0.0f)
			{
				float invLength = 1.f / length;
				x *= invLength;
				y *= invLength;
			}

			return length;
		}

		inline Vec2 NormalizeCopy() const
		{
			Vec2 result = *this;
			result.Normalize();
			return result;
		}

		bool operator<(const Vec2& other) const;

		float x, y;

		static const Vec2 UNIT_X;
		static const Vec2 UNIT_Y;
		static const Vec2 ZERO;


	};
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Vec2>
{
	static fastbird::Vec2 luaU_check(lua_State* L, int index)
	{
		luaL_checktype(L, index, LUA_TTABLE);
		fastbird::Vec2 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static fastbird::Vec2 luaU_to(lua_State* L, int index)
	{
		fastbird::Vec2 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Vec2& val)
	{
		lua_createtable(L, 2, 0);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -2, 2);
	}
};