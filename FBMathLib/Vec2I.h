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
#include "FBCommonHeaders/Types.h"

namespace fb
{
	class Vec2;
	class Vec2I
	{
	public:
		int x, y;

		static const Vec2I ZERO;

		//-------------------------------------------------------------------
		Vec2I();
		Vec2I(int _x, int _y);
		Vec2I(const Vec2& v);
		Vec2I(const Vec2ITuple& t);

		//-------------------------------------------------------------------
		Vec2I operator+ (int s) const;
		Vec2I operator+ (const Vec2I& v) const;
		Vec2I& operator+=(const Vec2I& v);
		Vec2I operator- (int s) const;
		Vec2I operator- (const Vec2I& v) const;
		Vec2I& operator-=(const Vec2I& v);
		Vec2I operator* (int s) const;
		Vec2I operator* (const Vec2I& v) const;
		Vec2I operator/ (int s) const;
		Vec2I operator/ (const Vec2I& v) const;	
		bool operator== (const Vec2I& v) const;
		bool operator!=(const Vec2I& v) const;		
		bool operator<(const Vec2I& v) const;
		operator Vec2ITuple() const;

		int Cross(const Vec2I& right);
		int Dot(const Vec2I& right);
		Real DistanceTo(const Vec2I& d) const;
		Real Length() const;
		Vec2I Scale(Real f) const;
	};

	Vec2I operator*(const Vec2I& left, unsigned right);
}

//// luawapper util
//template<>
//struct luaU_Impl<fb::Vec2I>
//{
//	static fb::Vec2I luaU_check(lua_State* L, int index)
//	{
//		luaL_checktype(L, index, LUA_TTABLE);
//		fb::Vec2I ret;
//		lua_rawgeti(L, index, 1);
//		ret.x = luaL_checkint(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.y = luaL_checkint(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static fb::Vec2I luaU_to(lua_State* L, int index)
//	{
//		fb::Vec2I ret;
//		lua_rawgeti(L, index, 1);
//		ret.x = lua_tointeger(L, -1);
//		lua_pop(L, 1);
//		lua_rawgeti(L, index, 2);
//		ret.y = lua_tointeger(L, -1);
//		lua_pop(L, 1);
//		return ret;
//	}
//
//	static void luaU_push(lua_State* L, const fb::Vec2I& val)
//	{
//		lua_createtable(L, 2, 0);
//		lua_pushinteger(L, val.x);
//		lua_rawseti(L, -2, 1);
//		lua_pushinteger(L, val.y);
//		lua_rawseti(L, -2, 2);
//	}
//};