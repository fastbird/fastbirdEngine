#pragma once

namespace fb {
	struct GraphicDeviceInfo {
		wchar_t Description[128] = {0};
		unsigned VendorId = 0;
		unsigned DeviceId = 0;
		unsigned SubSysId = 0;
		unsigned Revision = 0;
		size_t DedicatedVideoMemory = 0;
		size_t DedicatedSystemMemory = 0;
		size_t SharedSystemMemory = 0;
	};
}
