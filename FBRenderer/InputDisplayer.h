#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBInputManager/IInputConsumer.h"
namespace fb {
	FB_DECLARE_SMART_PTR(InputDisplayer);
	class InputDisplayer : public IInputConsumer {
		FB_DECLARE_PIMPL_NON_COPYABLE(InputDisplayer);
		InputDisplayer();
		~InputDisplayer();

	public:
		static InputDisplayerPtr Create();
		void ConsumeInput(IInputInjectorPtr injector) OVERRIDE;
	};
}
