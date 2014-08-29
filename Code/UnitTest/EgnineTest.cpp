#include <UnitTest/StdAfx.h>
#include <Engine/IVoxelizer.h>

TEST(Engine, Voxelizing)
{
	using namespace fastbird;
	IVoxelizer* pVox = IVoxelizer::CreateVoxelizer();
	bool successful = pVox->RunVoxelizer("D:/Blender/scifi/Battlestar_Pegasus/pegasus.dae", 64,
		false, false);
	IVoxelizer::DeleteVoxelizer(pVox);
	EXPECT_TRUE(successful);
}