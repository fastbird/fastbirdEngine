#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBSceneManager/SceneObject.h"
namespace fb{
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(SkyBox);
	class FB_DLL_SCENEOBJECTFACTORY SkyBox : public SceneObject{
		FB_DECLARE_PIMPL_NON_COPYABLE(SkyBox);
		SkyBox(const char* materialPath);
		~SkyBox();

	public:
		static SkyBoxPtr Create(const char* materialPath);

		//-------------------------------------------------------------------
		// Rendering
		//-------------------------------------------------------------------
		void PreRender(const RenderParam& param, RenderParamOut* paramOut);
		void Render(const RenderParam& param, RenderParamOut* paramOut);
		void PostRender(const RenderParam& param, RenderParamOut* paramOut);

		void SetMaterial(const char* path);
		MaterialPtr GetMaterial() const;
	};
}