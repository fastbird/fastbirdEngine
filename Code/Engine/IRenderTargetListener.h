#pragma once

namespace fastbird{
	class IRenderTarget;
	class IRenderTargetListener{
	public:
		virtual void OnRenderTargetDeleted(IRenderTarget* rt) {}
	};
}