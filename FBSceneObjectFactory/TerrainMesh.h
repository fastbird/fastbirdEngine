#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(TerrainMesh);
	class TerrainMesh {
		FB_DECLARE_PIMPL_NON_COPYABLE(TerrainMesh);
		TerrainMesh(int width, int height);
		~TerrainMesh();

	public:
		static TerrainMeshPtr Create(int width, int height);
		int GetPointCount();
		void Bind();
	};
}