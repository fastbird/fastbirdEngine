#pragma once
namespace fb {
	namespace CursorType {
		enum Enum {
			Gold,
			Num,
		};

		static const char* strs[] = {
			"Gold",
			"Num",
		};

		inline Enum ConvertToEnum(const char* str) {
			for (int i = 0; i < Num; ++i) {
				if (_stricmp(strs[i], str) == 0) {
					return Enum(i);
				}
			}
			return Num;
		}
	}
}
