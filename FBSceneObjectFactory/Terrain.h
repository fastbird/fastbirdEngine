#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(Terrain);
	class Terrain {
		FB_DECLARE_PIMPL_NON_COPYABLE(Terrain);
		Terrain();
		~Terrain();

	public:
		static TerrainPtr Create();
	};
}
