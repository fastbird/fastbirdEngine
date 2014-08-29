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
			Hexagonal,
			CardScroller,
			CardItem,
			VerticalGauge,
		};
		static const char* strings[] = 
		{
			"Window", 
			"Title", 
			"Button",
			"CheckBox",
			"RadioBox",
			"ListBox",
			"ListItem",
			"TextField",
			"Grid",
			"StaticText",
			"ImageBox",
			"FileSelector",
			"Scroller",
			"Hexagonal",
			"CardScroller",
			"CardItem",
			"VerticalGauge",
		};
		
		inline const char* ConvertToString(ComponentType::Enum e)
		{
			assert(e >= Window && e <= VerticalGauge);
			return strings[e];
		}

		inline Enum ConverToEnum(const char* sz)
		{
			if (stricmp(sz, "Window") == 0)
				return Window;
			if (stricmp(sz, "Title") == 0)
				return Title;
			if (stricmp(sz, "Button") == 0)
				return Button;
			if (stricmp(sz, "CheckBox") == 0)
				return CheckBox;
			if (stricmp(sz, "RadioBox") == 0)
				return RadioBox;
			if (stricmp(sz, "ListBox") == 0)
				return ListBox;
			if (stricmp(sz, "ListItem") == 0)
				return ListItem;
			if (stricmp(sz, "TextField") == 0)
				return TextField;
			if (stricmp(sz, "Grid") == 0)
				return Grid;
			if (stricmp(sz, "StaticText") == 0)
				return StaticText;
			if (stricmp(sz, "ImageBox") == 0)
				return ImageBox;
			if (stricmp(sz, "FileSelector") == 0)
				return FileSelector;
			if (stricmp(sz, "Scroller") == 0)
				return Scroller;
			if (stricmp(sz, "Hexagonal") == 0)
				return Hexagonal;
			if (stricmp(sz, "CardScroller") == 0)
				return CardScroller;
			if (stricmp(sz, "CardItem") == 0)
				return CardItem;
			if (stricmp(sz, "VerticalGauge") == 0)
				return VerticalGauge;
			
			assert(0);
			return Window;
		}
	}
}