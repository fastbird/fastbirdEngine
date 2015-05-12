#pragma once

namespace fastbird
{
	class ISceneListener
	{
	public:
		virtual void OnAfterMakeVisibleSet(IScene* scene){}
		virtual void OnBeforeRenderingOpaques(IScene* scene){}
		virtual void OnBeforeRenderingTransparents(IScene* scene){}
	};
}