#include "stdafx.h"
#include "ParticleTest.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBEngineFacade/ParticleFacade.h"
using namespace fb;
class ParticleTest::Impl{
public:
	Impl(){
	}

	void Update(TIME_PRECISION dt){
		
	}
};

FB_IMPLEMENT_STATIC_CREATE(ParticleTest);

ParticleTest::ParticleTest()
	:mImpl(new Impl)
{

}

ParticleTest::~ParticleTest(){

}

void ParticleTest::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}