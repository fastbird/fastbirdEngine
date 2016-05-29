#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	FB_DECLARE_SMART_PTR(Invoker);
	class FB_DLL_THREAD Invoker {
		FB_DECLARE_PIMPL_NON_COPYABLE(Invoker);
		Invoker();
		~Invoker();

	public:
		static InvokerPtr Create();
		static Invoker& GetInstance();
		static bool HasInstance();

		/// invode 'func' at the beginning of update loop
		void InvokeAtStart(std::function<void()>&& func);
		/// invode 'func' at the end of update loop
		void InvokeAtEnd(std::function<void()>&& func);

		/// If you use EngineFacade don't need to call this function manually
		void Start();
		/// If you use EngineFacade don't need to call this function manually
		void End();

		void PrepareQuit();

	};
}