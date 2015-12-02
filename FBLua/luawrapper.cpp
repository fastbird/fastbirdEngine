------------------------------------------------------------------------------
-- This source file is part of fastbird engine
-- For the latest info, see http://www.jungwan.net/
-- 
-- Copyright (c) 2013-2015 Jungwan Byun
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.
------------------------------------------------------------------------------

#include "stdafx.h"
#include "LuaUtils.h"
namespace fb{
#include "luawrapper.hpp"

	using namespace fb;

	void luaW_registerfuncs(lua_State* L, const luaL_Reg defaulttable[], const luaL_Reg table[])
	{
		// ... T
#if LUA_VERSION_NUM == 502
		if (defaulttable)
			luaL_setfuncs(L, defaulttable, 0); // ... T
		if (table)
			luaL_setfuncs(L, table, 0); // ... T
#else
		if (defaulttable)
			luaL_register(L, NULL, defaulttable); // ... T
		if (table)
			luaL_register(L, NULL, table); // ... T
#endif
	}

	void luaW_initialize(lua_State* L)
	{
		using namespace fb;
		// Ensure that the LuaWrapper table is set up
		LuaUtils::getfield(L, LUA_REGISTRYINDEX, LUAW_WRAPPER_KEY); // ... LuaWrapper
		if (LuaUtils::isnil(L, -1))
		{
			LuaUtils::newtable(L); // ... nil {}
			LuaUtils::pushvalue(L, -1); // ... nil {} {}
			LuaUtils::setfield(L, LUA_REGISTRYINDEX, LUAW_WRAPPER_KEY); // ... nil LuaWrapper

			// Create a storage table        
			LuaUtils::newtable(L); // ... LuaWrapper nil {}
			LuaUtils::setfield(L, -2, LUAW_STORAGE_KEY); // ... nil LuaWrapper

			// Create a holds table
			LuaUtils::newtable(L); // ... LuaWrapper {}
			LuaUtils::setfield(L, -2, LUAW_HOLDS_KEY); // ... nil LuaWrapper

			// Create a cache table, with weak values so that the userdata will not
			// be ref counted
			LuaUtils::newtable(L); // ... nil LuaWrapper {}
			LuaUtils::setfield(L, -2, LUAW_CACHE_KEY); // ... nil LuaWrapper

			LuaUtils::newtable(L); // ... nil LuaWrapper {}
			LuaUtils::pushstring(L, "v"); // ... nil LuaWrapper {} "v"
			LuaUtils::setfield(L, -2, "__mode"); // ... nil LuaWrapper {}
			LuaUtils::setfield(L, -2, LUAW_CACHE_METATABLE_KEY); // ... nil LuaWrapper

			LuaUtils::pop(L, 1); // ... nil
		}
		LuaUtils::pop(L, 1); // ...
	}
}