#include <Engine/StdAfx.h>
#include <Engine/VideoPlayerOgg.h>
#include <Engine/Renderer.h>

using namespace fastbird;

VideoPlayerOgg::VideoPlayerOgg()
	: mTheoraSetup(0)
	, mTheoraDecoderCtx(0)
	, mFile(0)
	, mPlayTime(0)
	, mVideoBufTime(0)
	, mAudioBufTime(0)
	, mGranulePos(-1)
	, mDurationAfterFinish(0)
	, mFinish(false)
	, mVideoBufReady(false)
{
	mPS = gFBEnv->pRenderer->CreateShader("es/shaders/YUVMovie.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
	
}

VideoPlayerOgg::~VideoPlayerOgg(){
	Clear();
	if (mFile) {
		fclose(mFile);
		mFile = 0;
	}
}

void VideoPlayerOgg::Clear()
{
	if (mTheoraDecoderCtx){
		th_decode_free(mTheoraDecoderCtx);
	}
	vorbis_comment_clear(&mVorbisComment);
	vorbis_info_clear(&mVorbisInfo);

	th_comment_clear(&mTheoraComment);
	th_info_clear(&mTheoraInfo);

	if (mStreamStateVorbis.body_data != 0){
		ogg_stream_clear(&mStreamStateVorbis);
	}
	if (mStreamStateTheora.body_data != 0){
		ogg_stream_clear(&mStreamStateTheora);
	}

	ogg_sync_clear(&mSyncState);
	
	mGranulePos = -1;
	mVideoBufTime = 0;
	mPlayTime = 0;	

	gFBEnv->pRenderer->UnregisterVideoPlayer(this);

	for (int i = 0; i < 3; i++)
	{
		free(mYCbCr[i].data);
	}

	mFilepath.clear();
}

bool VideoPlayerOgg::PlayVideo(const char* path){
	if (!path || strlen(path) == 0) {
		Error("Cannot find the file %s", path);
		return false;
	}

	auto error = fopen_s(&mFile, path, "rb");
	if (error){
		Error("Cannot open the file %s", path);
		return false;
	}
	if (!mFilepath.empty())
		Clear();



	for (int i = 0; i < 3; i++)
	{
		mYCbCr[i].data = 0;
	}
	ogg_sync_init(&mSyncState);
	mStreamStateTheora.body_data = 0;
	mStreamStateVorbis.body_data = 0;
	th_info_init(&mTheoraInfo);
	th_comment_init(&mTheoraComment);

	vorbis_info_init(&mVorbisInfo);
	vorbis_comment_init(&mVorbisComment);

	mFilepath = path;

	// read whole data
	int readed = 0;
	while (readed = buffer_data()){
		/* nothing to do*/
	}
	if (mFile) {
		fclose(mFile);
		mFile = 0;
	}

	ogg_page page;
	int parsingTheora = 0;
	while (ogg_sync_pageout(&mSyncState, &page) > 0){
		if (!ogg_page_bos(&page)){
			queue_page(&page);
		}
		else{
			ogg_stream_state unknownStream;
			ogg_packet packet;
			ogg_stream_init(&unknownStream, ogg_page_serialno(&page));
			ogg_stream_pagein(&unknownStream, &page);
			int got_packet = ogg_stream_packetpeek(&unknownStream, &packet);
			if (!got_packet){
				ogg_stream_clear(&unknownStream);
				return false;
			}
			// check theora
			bool consumed = false;
			if (mStreamStateTheora.body_data == 0){
				parsingTheora = th_decode_headerin(&mTheoraInfo, &mTheoraComment, &mTheoraSetup, &packet);
				if (parsingTheora > 0){
					memcpy(&mStreamStateTheora, &unknownStream, sizeof(unknownStream));
					ogg_stream_packetout(&mStreamStateTheora, NULL);
					consumed = true;
				}
			}
			if (!consumed){
				if (mStreamStateVorbis.body_data == 0){
					auto found = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &packet)==0;
					if (found){
						memcpy(&mStreamStateVorbis, &unknownStream, sizeof(unknownStream));
						ogg_stream_packetout(&mStreamStateVorbis, NULL);
						consumed = true;
					}
				}
			}

			if (!consumed){
				// ignore this page
				ogg_stream_clear(&unknownStream);
			}
		}
	}

	if (mStreamStateTheora.body_data != 0){
		while (parsingTheora > 0){
			ogg_packet packet;
			int got_packet = ogg_stream_packetpeek(&mStreamStateTheora, &packet);
			if (got_packet){
				parsingTheora = th_decode_headerin(&mTheoraInfo, &mTheoraComment, &mTheoraSetup, &packet);
				if (parsingTheora){
					ogg_stream_packetout(&mStreamStateTheora, NULL);
				}
			}
		}
		mTheoraDecoderCtx = th_decode_alloc(&mTheoraInfo, mTheoraSetup);
		Log("Ogg logical stream %lx is Theora %dx%d %.02f fps video\n"
			"Encoded frame content is %dx%d with %dx%d offset\n",
			mStreamStateTheora.serialno, mTheoraInfo.frame_width, mTheoraInfo.frame_height,
			(double)mTheoraInfo.fps_numerator / mTheoraInfo.fps_denominator,
			mTheoraInfo.pic_width, mTheoraInfo.pic_height, mTheoraInfo.pic_x, mTheoraInfo.pic_y);
		/*Either way, we're done with the codec setup data.*/
		th_setup_free(mTheoraSetup);
		mTheoraSetup = 0;

		open_video();

		ogg_packet packet;
		while (ogg_stream_packetout(&mStreamStateTheora, &packet)){
			mPacketsTheora.push_back(packet);
		}


	}

	if (mStreamStateVorbis.body_data != 0){
		int count = 2;  // need to parse two more packets
		while (count > 0){
			ogg_packet packet;
			int got_packet = ogg_stream_packetpeek(&mStreamStateVorbis, &packet);
			if (got_packet){
				bool succ = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, &packet) == 0;
				if (succ){
					ogg_stream_packetout(&mStreamStateVorbis, NULL);					
				}
				else{
					Error("Cannot parse vorbis header!");
					break;
				}
				--count;
			}
			else{
				Error("Vorbis header data is corrupted.");
				break;
			}
		}
	}
	gFBEnv->pRenderer->RegisterVideoPlayer(this);

	return true;
}


int VideoPlayerOgg::buffer_data(){
	assert(mFile);
	if (!mFile)
		return 0;

	char *buffer = ogg_sync_buffer(&mSyncState, 4096);
	int bytes = fread_s(buffer, 4096, 1, 4096, mFile);
	ogg_sync_wrote(&mSyncState, bytes);
	return(bytes);

}

int VideoPlayerOgg::queue_page(ogg_page *page){
	int ret = -1;
	auto no = ogg_page_serialno(page);

	if (mStreamStateTheora.serialno == no){
		ret = ogg_stream_pagein(&mStreamStateTheora, page);
	}
	else if (mStreamStateVorbis.serialno == no){
		ret = ogg_stream_pagein(&mStreamStateVorbis, page);
	}

	return ret;
}


void VideoPlayerOgg::StopVideo(){

}

void VideoPlayerOgg::RegisterVideoNotifier(VideoNotifierFunc func){
	mNotifierFunc = func;
}

void VideoPlayerOgg::SetDurationAfterFinish(float time){
	mDurationAfterFinish = time;
}

namespace fastbird{
	void stripe_decoded(VideoPlayerOgg* player, th_ycbcr_buffer _src,
		int _fragy0, int _fragy_end){
		player->StripeDecoded(_src, _fragy0, _fragy_end);
	}
}

void VideoPlayerOgg::StripeDecoded(th_ycbcr_buffer _src, int _fragy0, int _fragy_end){
	// skipping the data from the last packet.
	// The decode result of the last packet usually consists of black pixels.
	// This disturb the fade out effect.
	if (mPacketsTheora.empty())
		return;
	for (int pli = 0; pli<3; pli++){
		int yshift;
		int y_end;
		int y;
		yshift = pli != 0 && !(mTheoraInfo.pixel_fmt & 2);
		y_end = _fragy_end << (3 - yshift);
		/*An implemention intending to display this data would need to check the
		crop rectangle before proceeding.*/
		for (y = _fragy0 << (3 - yshift); y<y_end; y++){
			memcpy(mYCbCr[pli].data + y*mYCbCr[pli].stride,
				_src[pli].data + y*_src[pli].stride, 
				_src[pli].width);
		}
	}
}

void VideoPlayerOgg::open_video(){
	th_stripe_callback cb;
	/*Here we allocate a buffer so we can use the striped decode feature.
	There's no real reason to do this in this application, because we want to
	write to the file top-down, but the frame gets decoded bottom up, so we
	have to buffer it all anyway.
	But this illustrates how the API works.*/
	for (int pli = 0; pli<3; pli++){
		int xshift;
		int yshift;
		xshift = pli != 0 && !(mTheoraInfo.pixel_fmt & 1);
		yshift = pli != 0 && !(mTheoraInfo.pixel_fmt & 2);
		mYCbCr[pli].data = (unsigned char *)malloc(
			(mTheoraInfo.frame_width >> xshift)*(mTheoraInfo.frame_height >> yshift)*sizeof(char));
		mYCbCr[pli].stride = mTheoraInfo.frame_width >> xshift;
		mYCbCr[pli].width = mTheoraInfo.frame_width >> xshift;
		mYCbCr[pli].height = mTheoraInfo.frame_height >> yshift;



		mTextures[pli] = gFBEnv->pRenderer->CreateTexture(0, 	mYCbCr[pli].width, mYCbCr[pli].height,
			PIXEL_FORMAT_A8_UNORM, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_DEFAULT);
		mTextures[pli]->SetShaderStage(BINDING_SHADER_PS);
		mTextures[pli]->SetSlot(pli);
		auto mapData = mTextures[pli]->Map(0, MAP_TYPE::MAP_TYPE_WRITE_DISCARD, MAP_FLAG::MAP_FLAG_NONE);
		if (mapData.pData){
			memset(mapData.pData, 0, mYCbCr[pli].width * mYCbCr[pli].height);
			mTextures[pli]->Unmap(0);
		}
	}
	cb.ctx = this;
	cb.stripe_decoded = (th_stripe_decoded_func)stripe_decoded;
	th_decode_ctl(mTheoraDecoderCtx, TH_DECCTL_SET_STRIPE_CB, &cb, sizeof(cb));
}

//---------------------------------------------------------------------------
void VideoPlayerOgg::Update(float dt){
	if (mFinish){
		if (mDurationAfterFinish > 0){
			Display();
			mDurationAfterFinish -= dt;
		}
		return;
	}

	mPlayTime += dt;
	while (!mVideoBufReady && !mPacketsTheora.empty()){
		auto packet = mPacketsTheora.front();
		mPacketsTheora.erase(mPacketsTheora.begin());
		if (th_decode_packetin(mTheoraDecoderCtx, &packet, &mGranulePos) >= 0){
			mVideoBufTime = th_granule_time(mTheoraDecoderCtx, mGranulePos);
			mVideoBufReady = true;
		}
		else{
			Error("Cannot decode the file %s", mFilepath.c_str());
			break;
		}
	}

	if (!mVideoBufReady){
		mFinish = true;
		Display();
		if (mNotifierFunc){
			mNotifierFunc(this);
		}
		return;
	}

	if (mVideoBufTime <= mPlayTime){			
		mVideoBufReady = false;
		for (int pli = 0; pli<3; pli++){
			auto mapdata = mTextures[pli]->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
			if (mapdata.pData){
				for (int row = 0; row < mYCbCr[pli].height; ++row){
					unsigned char* dest = (unsigned char*)mapdata.pData;
					memcpy(dest + row*mapdata.RowPitch, mYCbCr[pli].data + row*mYCbCr[pli].stride,
						mYCbCr[pli].width);
				}
				mTextures[pli]->Unmap(0);
			}
		}
	}
	Display();
}

void VideoPlayerOgg::Display(){
	D3DEventMarker mark("Video Rendering");
	for (int pli = 0; pli<3; pli++){		
		mTextures[pli]->Bind();
	}
	auto renderer = (Renderer*)gFBEnv->pRenderer;
	renderer->SetNoDepthStencil();
	renderer->DrawFullscreenQuad(mPS, false);
}