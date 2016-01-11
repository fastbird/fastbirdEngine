#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	struct RENDERTARGET_CONSTANTS;
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(CascadedShadowsManager);
	class CascadedShadowsManager{
		FB_DECLARE_PIMPL_NON_COPYABLE(CascadedShadowsManager);
		CascadedShadowsManager();
		~CascadedShadowsManager();

	public:
		static CascadedShadowsManagerPtr Create();

		void CreateViewports();
		void CreateShadowMap();		
		void RenderShadows(IScenePtr scene, bool mainRT);
		void UpdateFrame(CameraPtr viewerCamera, const Vec3& lightDir,
			const AABB& sceneAABB);
		TexturePtr GetShadowMap() const;
		void DeleteShadowMap();
		CameraPtr GetLightCamera() const;				
	};
}