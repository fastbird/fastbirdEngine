#pragma once
#include <CommonLib/Math/MathDefines.h>
#include <CommonLib/Math/Vec2.h>
#include <CommonLib/luawrapperutil.hpp>
#include <sstream>
namespace fastbird
{
	class Mat33;
	class Vec3I;
	class Vec3
	{
	public:
		Vec3() {}
		Vec3(float _x, float _y, float _z)
			: x(_x), y(_y), z(_z)
		{			
		}

		Vec3(Vec2 v2, float _z)
		{
			x = v2.x;
			y = v2.y;
			z = _z;
		}

		explicit Vec3(float s)
			: x(s), y(s), z(s)
		{
		}

		Vec3(const Vec3I& v);

		Vec3(const char* s);

		inline float Dot(const Vec3& vec) const
        {
            return x * vec.x + y * vec.y + z * vec.z;
        }

		float DistanceTo(const Vec3& other) const
		{
			return (*this - other).Length();
		}

		float DistanceToSQ(const Vec3&other) const
		{
			return (*this - other).LengthSQ();
		}

		inline float operator[] ( const size_t i ) const
        {
            assert( i < 3 );

            return *(&x+i);
        }

		inline float& operator[] ( const size_t i )
        {
            assert( i < 3 );

            return *(&x+i);
        }

		inline Vec3 operator- () const
        {
            return Vec3(-x, -y, -z);
        }

		inline Vec3 operator* (float scalar) const
		{
			return Vec3(x * scalar, y * scalar, z * scalar);
		}
		inline Vec3 operator* (const Vec3& v) const
		{
			return Vec3(x * v.x, y* v.y, z*v.z);
		}
		inline Vec3 operator/ (float scalar) const
		{
			return Vec3(x / scalar, y / scalar, z / scalar);
		}

		inline Vec3 operator/ (const Vec3& v) const
		{
			return Vec3(x / v.x, y / v.y, z / v.z);
		}

		// v * m : means m is transposed.
		Vec3 operator* (const Mat33& r) const;

		inline Vec3 operator+ (const Vec3& r) const
		{
			return Vec3(x + r.x, y + r.y, z + r.z);
		}

		inline Vec3 operator+ (float f) const
		{
			return Vec3(x+f, y+f, z+f);
		}

		inline Vec3 operator- (const Vec3& r) const
		{
			return Vec3(x - r.x, y - r.y, z - r.z);
		}

		inline bool operator == ( const Vec3& r ) const
		{
			return ( abs(x - r.x) < EPSILON && 
				abs(y - r.y) < EPSILON &&
				abs(z - r.z) < EPSILON );
		}
		inline bool operator != (const Vec3& r ) const
		{
			return !operator==(r);
		}

		bool operator< (const Vec3& other) const;

		inline Vec3& operator+= (const Vec3& r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
			return *this;
		}

		inline Vec3& operator-= (const Vec3& r)
		{
			x -= r.x;
			y -= r.y;
			z -= r.z;
			return *this;
		}

		inline Vec3& operator*= (const Vec3& r)
		{
			x *= r.x;
			y *= r.y;
			z *= r.z;
			return *this;
		}

		inline Vec3& operator*= (float f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		inline Vec3& operator-= (const float f)
		{
			x -= f;
			y -= f;
			z -= f;
			return *this;
		}

		inline Vec3& operator/= (const Vec3& r)
		{
			x /= r.x;
			y /= r.y;
			z /= r.z;
			return *this;
		}

		inline Vec3& operator/= (float s)
		{
			x /= s;
			y /= s;
			z /= s;
			return *this;
		}

		inline bool operator >= (const Vec3& other) const
		{
			return x >= other.x && y >= other.y && z >= other.z;
		}

		inline bool operator <= (const Vec3& other) const
		{
			return x <= other.x && y <= other.y && z <= other.z;
		}

		inline void KeepGreater(const Vec3& other)
		{
			if (other.x > x) x = other.x;
			if (other.y > y) y = other.y;
			if (other.z > z) z = other.z;
		}

		inline void KeepLesser(const Vec3& other)
		{
			if (other.x < x) x = other.x;
			if (other.y < y) y = other.y;
			if (other.z < z) z = other.z;
		}

		// Because the magnitude of the cross product goes by the sine of the angle between its arguments, 
		// the cross product can be thought of as a measure of ¡®perpendicularity¡¯ in the same way that 
		// the dot product is a measure of ¡®parallelism¡¯. 
		// Given two unit vectors, their cross product has a magnitude of 1 if the two are perpendicular 
		// and a magnitude of zero if the two are parallel. The opposite is true for the dot product of two unit vectors.
		inline Vec3 Cross( const Vec3& rVector ) const
        {
            return Vec3(
                y * rVector.z - z * rVector.y,
                z * rVector.x - x * rVector.z,
                x * rVector.y - y * rVector.x);
        }

		inline int MaxAxis() const 
		{
			return x < y ? (y < z ? 2 : 1) : (x <z ? 2 : 0);
		}

		void SafeNormalize();

		inline float Normalize()
		{
			float length = sqrt(x*x + y*y + z*z);
			if (length > 0.0f)
			{
				float invLength = 1.f / length;
				x*= invLength;
				y*= invLength;
				z*= invLength;
			}

			return length;
		}

		inline Vec3 NormalizeCopy() const
		{
			Vec3 result = *this;
			result.Normalize();
			return result;
		}

		inline float Length() const
		{
			return sqrt(x*x + y*y + z*z);
		}

		inline float LengthSQ() const
		{
			return x*x + y*y + z*z;
		}

		float AngleBetween(const Vec3& v) const;

		inline Vec3 xyz()
		{
			return *this;
		}

		inline Vec3 yxy()
		{
			return Vec3(y, x, y);
		}

		inline Vec3 zzx()
		{
			return Vec3(z, z, x);
		}

		Vec2 xy(){
			return Vec2(x, y);
		}

	public:
		float x, y, z;

		static const Vec3 UNIT_X;
		static const Vec3 UNIT_Y;
		static const Vec3 UNIT_Z;
		static const Vec3 ZERO;
		static const Vec3 ONE;
		static const Vec3 MAX;
		static const Vec3 MIN;
	};

	inline Vec3 operator* (float l, const Vec3& r)
	{
		return Vec3(r.x*l, r.y*l, r.z*l);
	}

	inline bool IsEqual(const Vec3& l, const Vec3& r, float ep = EPSILON)
	{
		Vec3 t = l-r;
		if (abs(t.x) >= ep || abs(t.y) >= ep || abs(t.z) >= ep)
			return false;

		return true;
	}

	inline Vec3 Sign(const Vec3& v)
	{
		return Vec3(v.x < 0.0f ? -1 : (v.x == 0.0f ? 0.0f : 1.0f), 
			v.y < 0.0f ? -1 : (v.y == 0.0f ? 0.0f : 1.0f), 
			v.z < 0.0f ? -1 : (v.z == 0.0f ? 0.0f : 1.0f));
	}

	inline Vec3 Floor(const Vec3& v)
	{
		return Vec3(floor(v.x), floor(v.y), floor(v.z));
	}

	inline Vec3 Step(const Vec3& edge, const Vec3& v)
	{
		return Vec3(edge.x > v.x ? 0.0f : 1.0f,
			edge.y > v.y ? 0.0f : 1.0f,
			edge.z > v.z ? 0.0f : 1.0f);
	}
}

// serialization

inline std::istream& operator>>(std::istream& stream, fastbird::Vec3& v)
{
	stream >> v.x >> v.y >> v.z;
	return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const fastbird::Vec3& v)
{
	stream << v.x << v.y << v.z;
	return stream;
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Vec3>
{
	static fastbird::Vec3 luaU_check(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec3 luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		fastbird::Vec3 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)luaL_checknumber(L, -1);		
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static fastbird::Vec3 luaU_to(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Vec3 luaU_to(lua_State* L, int index)");
		fastbird::Vec3 ret;
		lua_rawgeti(L, index, 1);
		ret.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.z = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Vec3& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, val.z);
		lua_rawseti(L, -2, 3);
	}

	static void luaU_push(lua_State* L, fastbird::Vec3& val)
	{
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, val.z);
		lua_rawseti(L, -2, 3);
	}
};