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
#include "Vec3.h"
namespace fb
{
	class Vec3I
	{
	public:
		int x, y, z;

		static const Vec3I UNIT_X;
		static const Vec3I UNIT_Y;
		static const Vec3I UNIT_Z;
		static const Vec3I ZERO;
		static const Vec3I MAX;

		//-------------------------------------------------------------------
		Vec3I();
		Vec3I(int _x, int _y, int _z);
		explicit Vec3I(const Vec3& v);
		Vec3I(const Vec3ITuple& t);

		//-------------------------------------------------------------------
		Vec3I operator+ (int s) const;
		Vec3I operator+ (const Vec3I& v) const;
		Vec3I operator- (int s) const;
		Vec3I operator- (const Vec3I& v) const;
		Vec3I operator* (int s) const;
		Vec3 operator*(Real s) const;
		Vec3I operator* (const Vec3I& v) const;
		Vec3I operator/ (int s) const;		
		Vec3I operator/ (const Vec3I& v) const;		
		bool operator== (const Vec3I& v) const;
		bool operator!=(const Vec3I& v) const;
		bool operator<(const Vec3I& other) const;
		operator Vec3ITuple() const;

		//-------------------------------------------------------------------
		Real length() const;
		Real lengthSQ() const;
		Real distance(const Vec3I& to) const;
		Real distanceSQ(const Vec3I& to) const;
	};
}

//// luawapper util
//template<>
//struct luaU_Impl<fb::Vec3I>
//{
//	static fb::Vec3I luaU_check(lua_State* L, int index)
//	{
//		fb::LUA_STACK_WATCHER watcher(L, "static fb::Vec3I luaU_check(lua_State* L, int index)");
//		luaL_checktype(L, index, LUA_TTABLE);
//		fb::Vec3I ret;
//		lua_rawgeti(L, index, 1);
//		ret.x = luaL_checkint(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.y = luaL_checkint(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 3);
//		ret.z = luaL_checkint(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static fb::Vec3I luaU_to(lua_State* L, int index)
//	{
//		fb::LUA_STACK_WATCHER watcher(L, "static fb::Vec3I luaU_to(lua_State* L, int index)");
//		fb::Vec3I ret;
//		lua_rawgeti(L, index, 1);
//		ret.x = lua_tointeger(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.y = lua_tointeger(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 3);
//		ret.z = lua_tointeger(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static void luaU_push(lua_State* L, const fb::Vec3I& val)
//	{
//		lua_createtable(L, 3, 0);
//		lua_pushinteger(L, val.x);
//		lua_rawseti(L, -2, 1);
//		lua_pushinteger(L, val.y);
//		lua_rawseti(L, -2, 2);
//		lua_pushinteger(L, val.z);
//		lua_rawseti(L, -2, 3);
//	}
//};

// serialization
std::istream& operator>>(std::istream& stream, fb::Vec3I& v);
std::ostream& operator<<(std::ostream& stream, const fb::Vec3I& v);