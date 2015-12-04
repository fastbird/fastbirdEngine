#include "stdafx.h"
#include "MeshTest.h"
#include "FBSceneManager/SpatialObject.h"
#include "FBEngineFacade/MeshFacade.h"
#include "FBEngineFacade/EngineFacade.h"
using namespace fb;
class MeshTest::Impl{
public:
	MeshFacadePtr mMesh;

	Impl(){
		mMesh = MeshFacade::Create()->LoadMeshObject("data/cruiser_prototype2.dae");
		mMesh->AttachToScene();
	}

	~Impl(){

	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(MeshTest);
MeshTest::MeshTest()
	: mImpl(new Impl)
{

}

MeshTest::~MeshTest(){

}

void MeshTest::SetCameraTarget(){	
	EngineFacade::GetInstance().SetMainCameraTarget(mImpl->mMesh->GetSpatialObject());
}