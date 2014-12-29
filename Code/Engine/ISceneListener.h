#pragma once

namespace fastbird
{
	class ISceneListener
	{
	public:
		virtual void OnAfterMakeVisibleSet(){}
		virtual void OnBeforeRenderingOpaques(){}
		virtual void OnBeforeRenderingTransparents(){}
	};
}