#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/LaserRenderer.h>
namespace fastbird
{	
	
	LaserRenderer::LaserRenderer()
	{
		mBoundingVolumeWorld = BoundingVolume::Create(BoundingVolume::BV_AABB);
	}
	LaserRenderer::~LaserRenderer()
	{
	}

	void LaserRenderer::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
	{
		assert(name);
		mMaterial = IMaterial::CreateMaterial(name);
		assert(mMaterial);
	}

	void LaserRenderer::AddLaser(const fastbird::Vec3& from, const fastbird::Vec3& to, float thickness, const fastbird::Color& color)
	{
		mLasers.push_back(Laser{ from, to, color, thickness });
		mBoundingVolumeWorld->Merge(from);
		mBoundingVolumeWorld->Merge(to);
	}

	void LaserRenderer::Render()
	{
		if (mLasers.empty())
			return;

		mLasers.clear();
		mBoundingVolumeWorld->Invalidate();
	}
	void LaserRenderer::PostRender()
	{

	}
}