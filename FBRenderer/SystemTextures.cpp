#include "stdafx.h"
#include "SystemTextures.h"
namespace fb {
	namespace SystemTextures {
		static const char* STRINGS[] = {
			"Environment",
			"Depth",
			"CloudVolume",
			"Noise",
			"ShadowMap",
			"GGXPrecalc",
			"Permutation",
			"Gradiants",
			"ValueNoise",
		};
		Enum ConvertToEnum(const char* str) {
			for (int i = 0; i <= LAST; ++i) {
				if (_stricmp(str, STRINGS[i]) == 0)
					return Enum(i);
			}
			return Permutation;
		}
	}
}