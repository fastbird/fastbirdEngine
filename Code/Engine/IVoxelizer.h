#pragma once
#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Vec3I.h>

namespace fastbird
{
	class IMeshObject;
	class CLASS_DECLSPEC_ENGINE IVoxelizer: public ReferenceCounter
	{
	public:
		static IVoxelizer* CreateVoxelizer();
		static void DeleteVoxelizer(IVoxelizer* p);
		virtual bool RunVoxelizer(const char* filename, UINT numVoxels, bool swapYZ, bool oppositCull) = 0;
		virtual IMeshObject* GetMeshObject() const = 0;

		typedef std::vector<fastbird::Vec3I> HULLS;
		virtual const HULLS& GetHulls() const = 0;
	};
}