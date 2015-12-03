#include "stdafx.h"
#include "MeshGroupTest.h"
#include "FBSceneManager/SpatialObject.h"
#include "FBEngineFacade/MeshFacade.h"
#include "FBEngineFacade/EngineFacade.h"
using namespace fb;
class MeshGroupTest::Impl{
public:
	MeshFacadePtr mMesh;

	Impl(){
		mMesh = MeshFacade::Create()->LoadMeshGroup("data/aagun2.dae");
		mMesh->AttachToScene();
	}

	~Impl(){

	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(MeshGroupTest);
MeshGroupTest::MeshGroupTest()
	: mImpl(new Impl)
{

}

MeshGroupTest::~MeshGroupTest(){

}

void MeshGroupTest::SetCameraTarget(){	
	EngineFacade::GetInstance().SetMainCameraTarget(mImpl->mMesh->GetSpatialObject());
}