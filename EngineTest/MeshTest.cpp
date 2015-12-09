#include "stdafx.h"
#include "MeshTest.h"
#include "FBSceneManager/SpatialObject.h"
#include "FBEngineFacade/MeshFacade.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBInputManager/IInputInjector.h"
using namespace fb;
class MeshTest::Impl{
public:
	MeshFacadePtr mMesh;

	Impl(){
		mMesh = MeshFacade::Create()->LoadMeshGroup("data/aagun2.dae");
		mMesh->AttachToScene();
		mMesh->SetPosition(Vec3(100, 100, 100));
		Quat q;
		q.FromAngleAxis(Radian(45), Vec3::UNIT_Z);
		mMesh->SetMeshRotation(1, q);
		q.FromAngleAxis(Radian(45), Vec3::UNIT_X);
		mMesh->SetMeshRotation(2, q);
	}

	~Impl(){

	}

	void Update(TIME_PRECISION dt){
		auto injector = EngineFacade::GetInstance().GetInputInjector();
		if (injector->IsKeyPressed('K')){
			mMesh->PlayAction("Fire", true, false);
		}
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

void MeshTest::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}