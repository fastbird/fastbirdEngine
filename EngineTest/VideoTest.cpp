#include "stdafx.h"
#include "VideoTest.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBVideoPlayer/IVideoPlayer.h"
using namespace fb;
class VideoTest::Impl{
public:
	IVideoPlayerPtr mVideoPlayer;
	Impl(){
		mVideoPlayer = EngineFacade::GetInstance().CreateVideoPlayer(VideoPlayerType::OGG);
		mVideoPlayer->PlayVideo("EssentialEngineData/video/fastbirdengine2.ogv");
	}
};

FB_IMPLEMENT_STATIC_CREATE(VideoTest);
VideoTest::VideoTest()
	: mImpl(new Impl)
{

}

VideoTest::~VideoTest(){

}