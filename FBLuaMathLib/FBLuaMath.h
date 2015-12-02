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
#include "FBLua/LuaUtils.h"

namespace fb{
	class Vec2;
	template<>
	Vec2 luaU_check<Vec2>(lua_State* L, int      index);
	template<>
	Vec2 luaU_to<Vec2>(lua_State* L, int      index);
	template<>
	void luaU_push<Vec2>(lua_State* L, const Vec2& value);
	template<>
	void luaU_push<Vec2>(lua_State* L, Vec2& value);

	class Vec2I;
	template<>
	Vec2I luaU_check<Vec2I>(lua_State* L, int      index);
	template<>
	Vec2I luaU_to<Vec2I>(lua_State* L, int      index);
	template<>
	void luaU_push<Vec2I>(lua_State* L, const Vec2I& value);
	template<>
	void luaU_push<Vec2I>(lua_State* L, Vec2I& value);

	class Vec3;
	template<> 
	Vec3 luaU_check<Vec3>(lua_State* L, int      index);	
	template<>
	Vec3 luaU_to<Vec3>(lua_State* L, int      index);
	template<>
	void luaU_push<Vec3>(lua_State* L, const Vec3& value);
	template<> 
	void luaU_push<Vec3>(lua_State* L, Vec3& value);

	class Vec3I;
	template<>
	Vec3I luaU_check<Vec3I>(lua_State* L, int      index);
	template<>
	Vec3I luaU_to<Vec3I>(lua_State* L, int      index);
	template<>
	void luaU_push<Vec3I>(lua_State* L, const Vec3I& value);
	template<>
	void luaU_push<Vec3I>(lua_State* L, Vec3I& value);

	class Vec4;
	template<>
	Vec4 luaU_check<Vec4>(lua_State* L, int      index);
	template<>
	Vec4 luaU_to<Vec4>(lua_State* L, int      index);
	template<>
	void luaU_push<Vec4>(lua_State* L, const Vec4& value);
	template<>
	void luaU_push<Vec4>(lua_State* L, Vec4& value);

	class Quat;
	template<>
	Quat luaU_check<Quat>(lua_State* L, int      index);
	template<>
	Quat luaU_to<Quat>(lua_State* L, int      index);
	template<>
	void luaU_push<Quat>(lua_State* L, const Quat& value);
	template<>
	void luaU_push<Quat>(lua_State* L, Quat& value);

	class Transformation;
	template<>
	Transformation luaU_check<Transformation>(lua_State* L, int      index);
	template<>
	Transformation luaU_to<Transformation>(lua_State* L, int      index);
	template<>
	void luaU_push<Transformation>(lua_State* L, const Transformation& value);
	template<>
	void luaU_push<Transformation>(lua_State* L, Transformation& value);
}