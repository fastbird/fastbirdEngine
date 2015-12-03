#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(MeshGroupTest);
	class MeshGroupTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(MeshGroupTest);
		MeshGroupTest();
		~MeshGroupTest();

	public:
		static MeshGroupTestPtr Create();

		void SetCameraTarget();
	};
}