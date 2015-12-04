#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(MeshTest);
	class MeshTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(MeshTest);
		MeshTest();
		~MeshTest();

	public:
		static MeshTestPtr Create();

		void SetCameraTarget();
	};
}