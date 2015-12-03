#include "stdafx.h"
#include "SkyBoxTest.h"
#include "FBEngineFacade\SkyFacade.h"
using namespace fb;
class SkyBoxTest::Impl{
public:
	SkyFacadePtr mSky;

	Impl(){
		mSky = SkyFacade::Create()->CreateSkyBox("data/skybox.material");
		mSky->AttachToScene();
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(SkyBoxTest);
SkyBoxTest::SkyBoxTest()
	:mImpl(new Impl)
{

}
SkyBoxTest::~SkyBoxTest(){

}