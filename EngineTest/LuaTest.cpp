#include "stdafx.h"
#include "LuaTest.h"
#include "FBLua/LuaObject.h"
using namespace fb;
int LuaTestFunc(lua_State* L);
class LuaTest::Impl {
public:

	Impl() {
		auto L = LuaUtils::GetLuaState();
		LUA_SETCFUNCTION(L, LuaTestFunc);

		LuaUtils::DoFile("Data/LuaTest.lua");
	}

};

FB_IMPLEMENT_STATIC_CREATE(LuaTest);
LuaTest::LuaTest()
	:mImpl(new Impl)
{
}
LuaTest::~LuaTest() {

}


int LuaTestFunc(lua_State* L) {
	LuaObject table;
	table.NewTable(L);
	table.SetSeq(1, 1);
	table.SetSeq(2, 1);
	table.SetSeq(3, 1);
	table.SetSeq(4, 1);
	table.PushToStack();
	return 1;	
}