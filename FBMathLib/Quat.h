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

namespace fb
{
	class Mat33;
	class Vec3;
	class Quat
	{
	public:
		friend Quat operator* (Real fScalar, const Quat& qRot);

		Real w, x, y, z;

		//-------------------------------------------------------------------
		static const Real ms_fEpsilon;
		static const Quat ZERO;
		static const Quat IDENTITY;

		static Quat Slerp(Real fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);
		static Quat SlerpExtraSpins(Real fT, const Quat& qRotP, const Quat& qRotQ, int iExtraSpins);
		static void Intermediate(const Quat& qRot0, const Quat& qRot1, const Quat& qRot2, Quat& qRotA, Quat& qRotB);
		static Quat Squad(Real fT, const Quat& qRotP, const Quat& qRotA,
			const Quat& qRotB, const Quat& qRotQ, bool shortestPath = false);
		static Quat nlerp(Real fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);		

		//-------------------------------------------------------------------
		Quat();
		Quat(Real _w, Real _x, Real _y, Real _z);
		explicit Quat(const Mat33& rot);
		Quat(Real radian, const Vec3& axis);
		Quat(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis);
		explicit Quat(const Vec3& euler);
		Quat(const QuatTuple& t);

		//-------------------------------------------------------------------		
		Quat operator-(void) const;		
		Quat operator* (const Quat& qRot) const;
		Quat operator* (Real fScalar) const;
		Vec3 operator* (const Vec3& vec) const;		
		Quat& operator= (const Quat& other);
		bool operator== (const Quat& rhs) const;
		bool operator!= (const Quat& rhs) const;
		Real operator [] (const size_t i) const;
		Real& operator[] (const size_t i);
		operator QuatTuple() const;

		//-------------------------------------------------------------------
		void Swap(Quat& other);
		void FromRotationMatrix(const Mat33& rot);
		void ToRotationMatrix(Mat33& rot) const;
		void FromAngleAxis(const Real radian, const Vec3& axis);
		static Quat CreateFromAngleAxis(const Real radian, const Vec3& axis);
		void ToAngleAxis(Real& radian, Vec3& axis);
		void FromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis);
		void ToAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis);
		void FromDirection(const Vec3& dir);
		Vec3 xAxis() const;
		Vec3 yAxis() const;
		Vec3 zAxis() const;
		Real Dot(const Quat& other) const;
		Real Norm() const;
		Real Normalise();
		Quat Inverse() const;
		Quat UnitInverse() const;		
		Real GetRoll(bool reprojectAxis = true) const;
		Real GetPitch(bool reprojectAxis = true) const;
		Real GetYaw(bool reprojectAxis = true) const;
		bool Equals(const Quat& rhs, const Real toleranceRadian) const;
		bool IsNaN() const;	
	};

	void write(std::ostream& stream, const Quat& data);
	void read(std::istream& stream, Quat& data);
}

//-----------------------------------------------------------------------
// serialization
std::istream& operator>>(std::istream& stream, fb::Quat& v);
std::ostream& operator<<(std::ostream& stream, const fb::Quat& v);

//// luawapper util
//template<>
//struct luaU_Impl<fb::Quat>
//{
//	static fb::Quat luaU_check(lua_State* L, int index)
//	{
//		fb::LUA_STACK_WATCHER watcher(L, "static fb::Quat luaU_check(lua_State* L, int index)");
//		luaL_checktype(L, index, LUA_TTABLE);
//		fb::Quat ret;
//		lua_rawgeti(L, index, 1);
//		ret.w = (Real)luaL_checknumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.x = (Real)luaL_checknumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 3);
//		ret.y = (Real)luaL_checknumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 4);
//		ret.z = (Real)luaL_checknumber(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static fb::Quat luaU_to(lua_State* L, int index)
//	{
//		fb::LUA_STACK_WATCHER watcher(L, "static fb::Quat luaU_to(lua_State* L, int index)");
//		fb::Quat ret;
//		lua_rawgeti(L, index, 1);
//		ret.w = (Real)lua_tonumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.x = (Real)lua_tonumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 3);
//		ret.y = (Real)lua_tonumber(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 4);
//		ret.z = (Real)lua_tonumber(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static void luaU_push(lua_State* L, const fb::Quat& val)
//	{
//		lua_createtable(L, 0, 3);
//		lua_pushnumber(L, val.w);
//		lua_rawseti(L, -1, 1);
//		lua_pushnumber(L, val.x);
//		lua_rawseti(L, -1, 2);
//		lua_pushnumber(L, val.y);
//		lua_rawseti(L, -1, 3);
//		lua_pushnumber(L, val.z);
//		lua_rawseti(L, -1, 4);
//	}
//};