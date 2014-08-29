#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/Renderer/RendererEnums.h>

namespace fastbird
{
	class IMaterial;
	class CLASS_DECLSPEC_ENGINE IBillboardQuad : public SpatialObject
	{
	public:
		static IBillboardQuad* CreateBillboardQuad();

		virtual ~IBillboardQuad(){}

		virtual void SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color) = 0;

		virtual void SetAlphaBlend(bool blend) = 0;
		
	};
}