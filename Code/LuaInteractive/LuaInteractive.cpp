// LuaInteractive.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << argv[i] << std::endl;
	}
	if (argc != 2)
		return 0;

	auto L = luaL_newstate();
	luaL_openlibs(L);

	int error = luaL_dofile(L, argv[1]);
	if (error)
	{
		std::cout << lua_tostring(L, -1);
	}
	else
	{
		std::cout << "No error.";
	}

	lua_close(L);
	getchar();
	return 0;
}

