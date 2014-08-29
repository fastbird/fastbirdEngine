#pragma once
#include <Engine/IBillboardQuad.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class BillboardQuad : public IBillboardQuad
	{
	public:
		BillboardQuad();
		virtual ~BillboardQuad();

		// IObject
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();
		virtual void SetMaterial(IMaterial* mat, int pass = 0);
		virtual IMaterial* GetMaterial(int pass = 0) const;

		// IBillboardQuad
		virtual void SetBillobardData(const Vec3& pos, const Vec2& size, const Vec2& offset, const Color& color);
		virtual void SetAlphaBlend(bool blend);

	private:		
		Vec3 mWorldPos;
		Vec2 mSize;
		Vec2 mOffset;
		DWORD mColor;
		SmartPtr<IMaterial> mMaterial;

	};
}