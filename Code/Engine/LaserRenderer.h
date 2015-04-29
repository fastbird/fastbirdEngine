#pragma once
#include <Engine/ILaserRenderer.h>
namespace fastbird
{
	class LaserRenderer : public ILaserRenderer
	{
	public:
		LaserRenderer();
		~LaserRenderer();
		virtual void AddLaser(const fastbird::Vec3& from, const fastbird::Vec3& to, float thickness, const fastbird::Color& color);

		virtual void SetMaterial(const char* name, int pass = 0);
		virtual void PreRender(){}
		virtual void Render();
		virtual void PostRender();
		

	private:
		struct Laser
		{
			Vec3 mFrom;
			Vec3 mTo;
			Color mColor;
			float thickness;
		};

		std::vector<Laser> mLasers;
		SmartPtr<IMaterial> mMaterial;


	};
}