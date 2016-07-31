#pragma once
#include "FBCommonHeaders/GenericNotifier.h"
namespace fb {	
	FB_DECLARE_SMART_PTR(SkyFacade);
	class ISkyFacadeListener : public GenericListenerSub(ISkyFacadeListener) {
	public:
		//	need to update environment map.
		virtual void OnInterpolationFinished(const SkyFacadePtr& sky) {}
	};
}
#pragma once
