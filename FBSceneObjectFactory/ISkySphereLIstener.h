#pragma once
#include "FBCommonHeaders/GenericNotifier.h"
namespace fb {
	class SkySphere;
	class ISkySphereListener : public GenericListenerSub(ISkySphereListener) {
	public:
		//	need to update environment map.
		virtual void OnInterpolationFinished(SkySphere* sky) {}
	};
}
