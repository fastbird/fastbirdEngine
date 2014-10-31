#pragma once

namespace fastbird
{
	class ISceneListener
	{
	public:
		virtual void OnBeforeRenderingOpaques(){}
		virtual void OnBeforeRenderingTransparents(){}
	};
}