#include "stdafx.h"
#include "PrimitiveTopology.h"
namespace fb {
	PRIMITIVE_TOPOLOGY PrimitiveTopologyFromString(const char* sz) {
		for (int i = 0; i <= PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; ++i) {
			if (_stricmp(STR_PRIMITIVE_TOPOLOGY[i], sz) == 0) {
				return PRIMITIVE_TOPOLOGY(i);
			}
		}
		return PRIMITIVE_TOPOLOGY_UNKNOWN;
	}
}