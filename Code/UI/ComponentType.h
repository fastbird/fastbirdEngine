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
			HorizontalGauge,
			NumericUpDown,
			DropDown,
			TextBox,
			ColorRamp,
			NamedPortrait,
			PropertyList,
			TabWindow,

			NUM
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
			"HorizontalGauge",
			"NumericUpDown",
			"DropDown",
			"TextBox",
			"ColorRamp",
			"NamedPortrait",
			"PropertyList",
			"TabWindow",

			"NUM",
		};
		
		static_assert(ARRAYCOUNT(strings) == NUM + 1, "Count mismatch");

		inline const char* ConvertToString(ComponentType::Enum e)
		{
			assert(e >= Window && e < NUM);
			return strings[e];
		}

		inline const char* ConvertToString(unsigned e)
		{
			return ConvertToString(ComponentType::Enum(e));
		}

		inline Enum ConverToEnum(const char* sz)
		{
			int i = 0;
			for (auto& strEnum : strings)
			{
				if (_stricmp(strEnum, sz) == 0)
					return Enum(i);

				++i;
			}

			assert(0);
			return NUM;
		}
	}
}