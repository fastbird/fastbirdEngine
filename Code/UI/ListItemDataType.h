#pragma once
namespace fastbird{
	namespace ListItemDataType{
		enum Enum{
			String,
			CheckBox,
			TexturePath,
			TextureRegion,
			Texture,
			NumberKey,
			TextField,
			NumericUpDown,
			HorizontalGauge,

			Unknown,
		};

		static const char* szStrings[] = {
			"String",
			"CheckBox",
			"TexturePath",
			"TextureRegion",
			"Texture",
			"NumberKey",
			"TextField",
			"NumericUpDown",
			"HorizontalGauge",

			"Unknown",
		};

		inline const char* ConvertToString(Enum e)
		{
			return szStrings[e];
		}

		inline Enum ConvertToEnum(const char* str)
		{
			for (unsigned i = 0; i <= Unknown; ++i)
			{
				if (_stricmp(str, szStrings[i]) == 0)
					return Enum(i);
			}
		}
		REGISTER_ENUM_TO_LUA(Unknown, ListItemDataType);
	}
}