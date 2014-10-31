--FleetUI.lua

FleetUI = {ui="FleetUI"};

function FleetUI:OnOK(comp)
	FBPrint('FleetUI Ok clicked.');
end

function FleetUI:OnCancel(comp)
	FBPrint('FleetUI Cancel clicked.');
end

function FleetUI:OnVisible(comp)
	PreparePortrait(self.ui, "ShipImage") -- cpp function
end