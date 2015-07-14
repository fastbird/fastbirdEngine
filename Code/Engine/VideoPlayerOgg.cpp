#include <Engine/StdAfx.h>
#include <Engine/VideoPlayerOgg.h>
#include <Engine/Renderer.h>

using namespace fastbird;

VideoPlayerOgg::VideoPlayerOgg()
	: mSyncState(0)
	, mInfo(0)
	, mComment(0)
	, mSetup(0)
	, mPacket(0)
	, mFile(0)
	, mStateFlag(0)
	, mPage(0)
	, mStreamState(0)
	, mIsTheora(0)
	, mIsProcessingTheoraHeader(0)
	, mDecoderCtx(0)
	, mPlayTime(0)
	, mVideoBufTime(0)
	, mGranulePos(-1)
	, mVideoBufReady(false)
	, mFinish(false)
{
	mPS = gFBEnv->pRenderer->CreateShader("es/shaders/YUVMovie.hlsl", BINDING_SHADER_PS, IMaterial::SHADER_DEFINES());
}

VideoPlayerOgg::~VideoPlayerOgg(){
	
}

void VideoPlayerOgg::Clear()
{
	mIsTheora = 0;
	mIsProcessingTheoraHeader = 0;
	if (mSyncState){
		ogg_sync_clear(mSyncState);
		FB_DELETE(mSyncState);
		mSyncState = 0;
	}
	if (mComment){
		th_comment_clear(mComment);
		FB_DELETE(mComment);
		mComment = 0;
	}
	if (mInfo){
		th_info_clear(mInfo);
		FB_DELETE(mInfo);
		mInfo = 0;
	}

	if (mPage){
		FB_DELETE(mPage);
		mPage = 0;
	}

	if (mStreamState){
		FB_DELETE(mStreamState);
		mStreamState = 0;
	}

	if (mPacket){
		FB_DELETE(mPacket);
		mPacket = 0;
	}
	if (mDecoderCtx){
		th_decode_free(mDecoderCtx);
		mDecoderCtx = 0;
	}
	if (mSetup){
		th_setup_free(mSetup);
		mSetup = 0;
	}
	gFBEnv->pRenderer->UnregisterVideoPlayer(this);
}

int VideoPlayerOgg::buffer_data(){
	assert(mFile && mSyncState);
	if (!mFile)
		return;
	if (!mSyncState)
		return;

	char *buffer = ogg_sync_buffer(mSyncState, 4096);
	int bytes = fread_s(buffer, 4096, 1, 4096, mFile);
	ogg_sync_wrote(mSyncState, bytes);
	return(bytes);

}

int VideoPlayerOgg::queue_page(ogg_page *page){
	if (mIsTheora)
		ogg_stream_pagein(mStreamState, page);
	return 0;
}

bool VideoPlayerOgg::PlayVideo(const char* path){
	if (!path || strlen(path) == 0) {
		Error("Cannot find the file %s", path);
		return false;
	}

	mFilepath = path;
	Clear();
	
	mSyncState = FB_NEW(ogg_sync_state);
	ogg_sync_init(mSyncState);
	mComment = FB_NEW(th_comment);
	th_comment_init(mComment);
	mInfo = FB_NEW(th_info);
	th_info_init(mInfo);
	mPage = FB_NEW(ogg_page);
	mStreamState = FB_NEW(ogg_stream_state);
	mPacket = FB_NEW(ogg_packet);


	/* Only interested in Theora streams */
	while (!mStateFlag){
		int ret = buffer_data();
		if (ret == 0)
			break;
		while (ogg_sync_pageout(mSyncState, mPage)>0){
			int got_packet;
			ogg_stream_state test;

			/* is this a mandated initial header? If not, stop parsing */
			if (!ogg_page_bos(mPage)){
				/* don't leak the page; get it into the appropriate stream */
				queue_page(mPage);
				mStateFlag = 1;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(mPage));
			ogg_stream_pagein(&test, mPage);
			got_packet = ogg_stream_packetpeek(&test, mPacket);

			/* identify the codec: try theora */
			if ((got_packet == 1) && !mIsTheora && (mIsProcessingTheoraHeader =
				th_decode_headerin(mInfo, mComment, &mSetup, mPacket)) >= 0){
				/* it is theora -- save this stream state */
				memcpy(mStreamState, &test, sizeof(test));
				mIsTheora = 1;
				/*Advance past the successfully processed header.*/
				if (mIsProcessingTheoraHeader)
					ogg_stream_packetout(mStreamState, NULL);
			}
			else{
				/* whatever it is, we don't care about it */
				ogg_stream_clear(&test);
			}
		}
		/* fall through to non-bos page parsing */
	}

	/* we're expecting more header packets. */
	while (mIsTheora && mIsProcessingTheoraHeader){
		int ret;

		/* look for further theora headers */
		while (mIsProcessingTheoraHeader && (ret = ogg_stream_packetpeek(mStreamState, mPacket))){
			if (ret<0)
				continue;
			mIsProcessingTheoraHeader = th_decode_headerin(mInfo, mComment, &mSetup, mPacket);
			if (mIsProcessingTheoraHeader<0){
				Error("Error parsing Theora stream headers");
				return false;
				fprintf(stderr, "Error parsing Theora stream headers; "
					"corrupt stream?\n");
				exit(1);
			}
			else if (mIsProcessingTheoraHeader>0){
				/*Advance past the successfully processed header.*/
				ogg_stream_packetout(mStreamState, NULL);
			}
			mIsTheora++;
		}

		/*Stop now so we don't fail if there aren't enough pages in a short
		stream.*/
		if (!(mIsTheora && mIsProcessingTheoraHeader))
			break;

		/* The header pages/packets will arrive before anything else we
		care about, or the stream is not obeying spec */

		if (ogg_sync_pageout(mSyncState, mPage)>0){
			queue_page(mPage); /* demux into the appropriate stream */
		}
		else{
			int ret = buffer_data(); /* someone needs more data */
			if (ret == 0){
				Error("End of file while searching for codec headers.");
				return false;
			}
		}
	}

	/* and now we have it all.  initialize decoders */
	if (mIsTheora){
		mDecoderCtx = th_decode_alloc(mInfo, mSetup);
		Log("Ogg logical stream %lx is Theora %dx%d %.02f fps video\n"
			"Encoded frame content is %dx%d with %dx%d offset\n",
			mStreamState->serialno, mInfo->frame_width, mInfo->frame_height,
			(double)mInfo->fps_numerator / mInfo->fps_denominator,
			mInfo->pic_width, mInfo->pic_height, mInfo->pic_x, mInfo->pic_y);
	}
	else{
		/* tear down the partial theora setup */
		th_info_clear(mInfo);
		FB_SAFE_DEL(mInfo);
		th_comment_clear(mComment);
		FB_SAFE_DEL(mComment);
	}
	/*Either way, we're done with the codec setup data.*/
	th_setup_free(mSetup);
	mSetup = 0;

	/* open video */
	if (mIsTheora){
		open_video();
		gFBEnv->pRenderer->RegisterVideoPlayer(this);
	}
	else{
		Clear();
	}
}

void VideoPlayerOgg::StopVideo(){

}

void VideoPlayerOgg::RegisterVideoNotifier(VideoNotifierFunc func){

}

namespace fastbird{
	void stripe_decoded(VideoPlayerOgg* player, th_ycbcr_buffer _src,
		int _fragy0, int _fragy_end){
		player->StripeDecoded(_src, _fragy0, _fragy_end);
	}
}

void VideoPlayerOgg::StripeDecoded(th_ycbcr_buffer _src, int _fragy0, int _fragy_end){
	for (int pli = 0; pli<3; pli++){
		int yshift;
		int y_end;
		int y;
		yshift = pli != 0 && !(mInfo->pixel_fmt & 2);
		y_end = _fragy_end << 3 - yshift;
		/*An implemention intending to display this data would need to check the
		crop rectangle before proceeding.*/
		for (y = _fragy0 << 3 - yshift; y<y_end; y++){
			memcpy(mYCbCr[pli].data + y*mYCbCr[pli].stride,
				_src[pli].data + y*_src[pli].stride, _src[pli].width);
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
		xshift = pli != 0 && !(mInfo->pixel_fmt & 1);
		yshift = pli != 0 && !(mInfo->pixel_fmt & 2);
		mYCbCr[pli].data = (unsigned char *)malloc(
			(mInfo->frame_width >> xshift)*(mInfo->frame_height >> yshift)*sizeof(char));
		mYCbCr[pli].stride = mInfo->frame_width >> xshift;
		mYCbCr[pli].width = mInfo->frame_width >> xshift;
		mYCbCr[pli].height = mInfo->frame_height >> yshift;

		mTextures[pli] = gFBEnv->pRenderer->CreateTexture(0, 	mYCbCr[pli].width, mYCbCr[pli].height,
			PIXEL_FORMAT_A8_UNORM, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_DEFAULT);
		mTextures[pli]->SetShaderStage(BINDING_SHADER_PS);
		mTextures[pli]->SetSlot(pli);
	}
	cb.ctx = this;
	cb.stripe_decoded = (th_stripe_decoded_func)stripe_decoded;
	th_decode_ctl(mDecoderCtx, TH_DECCTL_SET_STRIPE_CB, &cb, sizeof(cb));
}

//---------------------------------------------------------------------------
void VideoPlayerOgg::Update(float dt){
	if (!mIsTheora || mFinish)
		return;
	mPlayTime += dt;
	while (!mVideoBufReady){
		/* theora is one in, one out... */
		if (ogg_stream_packetout(mStreamState, mPacket)>0){
			if (th_decode_packetin(mDecoderCtx, mPacket, &mGranulePos) >= 0){
				mVideoBufTime = th_granule_time(mDecoderCtx, mGranulePos);
				mVideoBufReady = true;
			}
		}
		else{
			break;
		}
	}

	if (!mVideoBufReady && feof(mFile)){
		mFinish = true;
		return;
	}

	if (!mVideoBufReady){
		/* no data yet for somebody.  Grab another page */
		buffer_data();
		while (ogg_sync_pageout(mSyncState, mPage)>0){
			queue_page(mPage);
		}
	}
	/* dumpvideo frame, and get new one */
	else {
		if (mVideoBufTime >= mPlayTime)
			Display();
	}
}

void VideoPlayerOgg::Display(){
	for (int pli = 0; pli<3; pli++){
		auto mapdata = mTextures[pli]->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (mapdata.pData){
			memcpy(mapdata.pData, mYCbCr[pli].data, mYCbCr[pli].width*mYCbCr[pli].height);
			mTextures[pli]->Unmap(0);
		}
		mTextures[pli]->Bind();
	}
	auto renderer = (Renderer*)gFBEnv->pRenderer;
	renderer->DrawFullscreenQuad(mPS, false);
}