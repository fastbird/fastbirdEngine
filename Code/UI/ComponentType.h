#pragma once

namespace fastbird
{
	namespace ComponentType
	{
		enum Enum
		{
			Window, 
			Title, 
			Button,
			CheckBox,
			RadioBox,
			ListBox,
			ListItem,
			TextField,
			Grid,
			StaticText,
			ImageBox,
			FileSelector,
			Scroller,
		};
		static const char* sz[] = 
		{
			"Window", 
			"Title", 
			"Button",
			"CheckBox",
			"RadioBox",
			"ListBox",
			"TextField",
			"Grid",
			"StaticText",
			"ImageBox",
			"FileSelector",
		};
	}

	inline const char* ToString(ComponentType::Enum e)
	{
		return ComponentType::sz[e];
	}
}