#include "stdafx.h"
#include "AudioTest.h"
#include "FBEngineFacade/EngineFacade.h"
using namespace fb;
class AudioTest::Impl{
public:
	Impl(){
		//EngineFacade::GetInstance().PlayAudio("data/audio/button_mouse_click.wav");
		EngineFacade::GetInstance().SetListenerPosition(Vec3(0, 0, 0));
		AudioProperty prop;
		prop.mPosition = Vec3(100, 0, 0);
		auto id = EngineFacade::GetInstance().PlayAudio("data/audio/button_mouse_click.wav", prop);		
		
	}
};

FB_IMPLEMENT_STATIC_CREATE(AudioTest);
AudioTest::AudioTest()
	: mImpl(new Impl)
{

}

AudioTest::~AudioTest(){

}