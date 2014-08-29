#pragma once
#include <Engine/IDustRenderer.h>
namespace fastbird
{
	class DustRenderer : public IDustRenderer
	{
	public:
		DustRenderer();
		virtual ~DustRenderer();

		// IDustRenderer
		virtual void InitDustRenderer(const Vec3& min, const Vec3& max, size_t count, 
			const Color& cmin, const Color& cmax,
			float normalizeDist);
		virtual const Vec3& GetMin() const { return mMin; }
		virtual IMaterial* GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
		{
			return mMaterial;
		}


		// IObject
		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();
		virtual void SetMaterial(const char* name, int pass = 0);

	private:
		Vec3 mMin;
		SmartPtr<IVertexBuffer> mVBColor;
		SmartPtr<IVertexBuffer> mVBPos;
		SmartPtr<IMaterial> mMaterial;



	};
}