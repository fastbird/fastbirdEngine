--LuaTest.lua
function FBPrint(str, ...)	
	_FBPrint(string.format(str, ...));
end

local t = LuaTestFunc();
FBPrint('num t = ' .. #t)