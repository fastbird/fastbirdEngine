#pragma once
#include <Engine/IVoxelizer.h>
#include <Engine/MeshObject.h>

namespace fastbird
{
	class IColladaImporter;
	class Voxelizer : public IVoxelizer
	{
	public:
		Voxelizer();
		virtual ~Voxelizer();

	protected:
		virtual void FinishSmartPtr();

	public:
		// numVoxels : in one axis, total = numVexels ^ 3 
		virtual bool RunVoxelizer(const char* filename, UINT numVoxels, bool swapYZ, bool oppositCull);
		virtual IMeshObject* GetMeshObject() const;
		virtual const HULLS& GetHulls() const {return mHulls;}

	protected:
		void CalcDistanceMap();
		void CreateHull();

	private:
		std::string mFilepath;
		float mVoxelSize;

		float* mDistanceMap;
		std::vector<Vec3I> mHulls;
		UINT mNumVoxels; // in one axis.
		SmartPtr<IColladaImporter> mColladaImporter;


	};
}