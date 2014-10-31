--Startup.lua

function Startup()
	FBPrint('Startup function is called.');
	-- load uis
	
	LoadLuaUI("data/ui/FleetUI.ui");
	SetVisibleLuaUI("FleetUI", true);
end