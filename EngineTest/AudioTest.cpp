#include "stdafx.h"
#include "AudioTest.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBAudioPlayer/AudioEx.h"
using namespace fb;
class AudioTest::Impl{
public:
	Impl(){
		//EngineFacade::GetInstance().PlayAudio("data/audio/button_mouse_click.wav");
		EngineFacade::GetInstance().SetListenerPosition(Vec3(0, 0, 0));
		AudioProperty prop;
		auto audioex = AudioEx::Create(prop);
		audioex->SetStartLoopEnd("", "data/audio/big_laser_fire_loop.wav", "");
		audioex->Play(6.f);
		
	}
};

FB_IMPLEMENT_STATIC_CREATE(AudioTest);
AudioTest::AudioTest()
	: mImpl(new Impl)
{

}

AudioTest::~AudioTest(){

}