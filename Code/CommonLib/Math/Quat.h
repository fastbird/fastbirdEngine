#pragma once

#include <assert.h>
#include <ostream>

namespace fastbird
{
	class Mat33;
	class Vec3;
	class Quat
	{
	public:
		Quat()
			: w(1), x(0), y(0), z(0)
		{
		}

		Quat(float _w, float _x, float _y, float _z)
			: w(_w), x(_x), y(_y), z(_z)
		{
		}

		Quat(const Mat33& rot)
		{
			FromRotationMatrix(rot);
		}

		Quat(float radian, const Vec3& axis)
		{
			FromAngleAxis(radian, axis);
		}

		Quat(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis)
		{
			FromAxes(xAxis, yAxis, zAxis);
		}

		Quat(const Vec3& euler) {
			float sx = sin(euler.x*.5f);
			float sy = sin(euler.y*.5f);
			float sz = sin(euler.z*.5f);

			float cx = cos(euler.x*.5f);
			float cy = cos(euler.y*.5f);
			float cz = cos(euler.z*.5f);			

			// xyz order
			this->w = cx * cy * cz + sx * sy * sz;
			this->x = sx * cy * cz - cx * sy * sz;
			this->y = cx * sy * cz + sx * cy * sz;
			this->z = cx * cy * sz - sx * sy * cz;
		}

		void Swap(Quat& other)
		{
			std::swap(w, other.w);
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
		}

		inline float operator [] ( const size_t i ) const
		{
			assert( i < 4 );
			return *(&w+i);
		}

		inline float& operator[] (const size_t i)
		{
			assert(i<4);
			return *(&w+i);
		}

		void FromRotationMatrix(const Mat33& rot);
		void ToRotationMatrix(Mat33& rot) const;
		void FromAngleAxis(const float radian, const Vec3& axis);
		void ToAngleAxis(float& radian, Vec3& axis);
		void FromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis);
		void ToAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis);
		void FromDirection(const Vec3& dir);

		Vec3 xAxis() const;
		Vec3 yAxis() const;
		Vec3 zAxis() const;

		inline Quat& operator= (const Quat& other)
		{
			w = other.w;
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		Quat operator+ (const Quat& qRot) const;
		Quat operator- (const Quat& qRot) const;
		Quat operator* (const Quat& qRot) const;
		Quat operator* (float fScalar) const;
		friend Quat operator* (float fScalar, const Quat& qRot);
		Quat operator- () const;
		inline bool operator== (const Quat& rhs) const
		{
			return w == rhs.w && x == rhs.x &&
				y == rhs.y && z == rhs.z;
		}

		inline bool operator!= (const Quat& rhs) const
		{
			return !operator==(rhs);
		}

		float Dot(const Quat& other) const;
		float Norm() const;
		float Normalise();
		Quat Inverse() const
		{
			return Quat(w, -x, -y, -z);
		}
		Quat UnitInverse() const;
		Quat Exp() const;
		Quat Log() const;

		Vec3 operator* (const Vec3& vec) const;

		float GetRoll(bool reprojectAxis = true) const;
		float GetPitch(bool reprojectAxis = true) const;
		float GetYaw(bool reprojectAxis = true) const;
		bool Equals(const Quat& rhs, const float toleranceRadian) const;

		static Quat Slerp(float fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);
		static Quat SlerpExtraSpins(float fT, const Quat& qRotP, const Quat& qRotQ, int iExtraSpins);
		static void Intermediate(const Quat& qRot0, const Quat& qRot1, const Quat& qRot2, Quat& qRotA, Quat& qRotB);
		static Quat Squad(float fT, const Quat& qRotP, const Quat& qRotA, 
			const Quat& qRotB, const Quat& qRotQ, bool shortestPath = false);
		static Quat nlerp(float fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);

		static const float ms_fEpsilon;
		static const Quat ZERO;
		static const Quat IDENTITY;

		inline bool IsNaN() const;
	

	public:

		float w, x, y, z;
	};
}

// serialization

inline std::istream& operator>>(std::istream& stream, fastbird::Quat& v)
{
	stream >> v.w >> v.x >> v.y >> v.z;
	return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const fastbird::Quat& v)
{
	stream << v.w << v.x << v.y << v.z;
	return stream;
}

// luawapper util
template<>
struct luaU_Impl<fastbird::Quat>
{
	static fastbird::Quat luaU_check(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Quat luaU_check(lua_State* L, int index)");
		luaL_checktype(L, index, LUA_TTABLE);
		fastbird::Quat ret;
		lua_rawgeti(L, index, 1);
		ret.w = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.x = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.y = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);
		ret.z = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static fastbird::Quat luaU_to(lua_State* L, int index)
	{
		fastbird::LUA_STACK_WATCHER watcher(L, "static fastbird::Quat luaU_to(lua_State* L, int index)");
		fastbird::Quat ret;
		lua_rawgeti(L, index, 1);
		ret.w = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);
		ret.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		ret.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);
		ret.z = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return ret;
	}

	static void luaU_push(lua_State* L, const fastbird::Quat& val)
	{
		lua_createtable(L, 0, 3);
		lua_pushnumber(L, val.w);
		lua_rawseti(L, -1, 1);
		lua_pushnumber(L, val.x);
		lua_rawseti(L, -1, 2);
		lua_pushnumber(L, val.y);
		lua_rawseti(L, -1, 3);
		lua_pushnumber(L, val.z);
		lua_rawseti(L, -1, 4);
	}
};