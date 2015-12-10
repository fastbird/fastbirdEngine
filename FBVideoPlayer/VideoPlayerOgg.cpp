/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include <theora/theoradec.h>
#include <ogg/ogg.h>
#include "VideoPlayerOgg.h"
#include "FBAudioPlayer/Audio.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/ResourceProvider.h"
#include "FBRenderer/Texture.h"
#include "FBTimer/Timer.h"

using namespace fb;

namespace fb{
	Timer* gpTimer = 0;
	void stripe_decoded(VideoPlayerOgg::Impl* player, th_ycbcr_buffer _src,
		int _fragy0, int _fragy_end);
}

class VideoPlayerOgg::Impl{
public:
	VideoPlayerOggWeakPtr mSelf;
	ogg_sync_state mSyncState;
	ogg_stream_state mStreamStateTheora;
	std::vector<ogg_packet> mPacketsTheora;

	th_info mTheoraInfo;
	th_comment mTheoraComment;
	th_setup_info* mTheoraSetup;
	th_dec_ctx* mTheoraDecoderCtx;
	th_ycbcr_buffer mYCbCr;

	FILE* mFile;

	std::string mFilepath;

	double mVideoBufTime;
	ogg_int64_t mGranulePos;

	TexturePtr mTextures[3];
	ShaderPtr mPS;

	VideoNotifierFunc mNotifierFunc;

	TIME_PRECISION mDurationAfterFinish;
	bool mFinish;

	bool mVideoBufReady;
	bool mIsFirstFrame;
	TIME_PRECISION mStartedTime;

	AudioPtr mAudio;

	//---------------------------------------------------------------------------
	Impl()
		:mTheoraSetup(0)
		, mTheoraDecoderCtx(0)
		, mFile(0)
		, mVideoBufTime(0)
		, mGranulePos(-1)
		, mDurationAfterFinish(0)
		, mFinish(false)
		, mVideoBufReady(false)
		, mIsFirstFrame(true)
	{
		mPS = Renderer::GetInstance().CreateShader("EssentialEnginedata/shaders/YUVMovie.hlsl", BINDING_SHADER_PS);
		if (!gpTimer){
			gpTimer = Timer::GetMainTimer().get();
		}
	}

	~Impl(){
		Clear();
		if (mFile) {
			fclose(mFile);
			mFile = 0;
		}
	}

	void Clear()
	{
		if (mTheoraDecoderCtx){
			th_decode_free(mTheoraDecoderCtx);
		}

		th_comment_clear(&mTheoraComment);
		th_info_clear(&mTheoraInfo);


		if (mStreamStateTheora.body_data != 0){
			ogg_stream_clear(&mStreamStateTheora);
		}

		ogg_sync_clear(&mSyncState);

		mGranulePos = -1;
		mVideoBufTime = 0;		

		for (int i = 0; i < 3; i++)
		{
			free(mYCbCr[i].data);
		}

		mFilepath.clear();

		mAudio = 0;
	}

	bool PlayVideo(const char* path){
		if (!path || strlen(path) == 0) {
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the file %s", path).c_str());
			return false;
		}

		auto error = fopen_s(&mFile, path, "rb");
		if (error){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot open the file %s", path).c_str());
			return false;
		}
		if (!mFilepath.empty())
			Clear();

		mAudio = Audio::Create();
		mAudio->InitOgg();

		for (int i = 0; i < 3; i++)
		{
			mYCbCr[i].data = 0;
		}
		ogg_sync_init(&mSyncState);
		mStreamStateTheora.body_data = 0;

		th_info_init(&mTheoraInfo);
		th_comment_init(&mTheoraComment);

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

				// check vorbis
				if (!consumed && mAudio){
					consumed = mAudio->ProcessOggHeaderPacket(&unknownStream, &packet);
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
			mIsFirstFrame = true;
			open_video();

			ogg_packet packet;
			while (ogg_stream_packetout(&mStreamStateTheora, &packet)){
				mPacketsTheora.push_back(packet);
			}
		}

		if (mAudio)
			mAudio->ProcessOggHeader();
		return true;
	}

	int buffer_data(){
		assert(mFile);
		if (!mFile)
			return 0;

		char *buffer = ogg_sync_buffer(&mSyncState, 4096);
		int bytes = fread_s(buffer, 4096, 1, 4096, mFile);
		ogg_sync_wrote(&mSyncState, bytes);
		return(bytes);

	}

	int queue_page(ogg_page *page){
		int ret = -1;
		auto no = ogg_page_serialno(page);

		if (mStreamStateTheora.serialno == no){
			ret = ogg_stream_pagein(&mStreamStateTheora, page);
		}
		else if (mAudio && mAudio->GetVorbisSerialno() == no){
			ret = ogg_stream_pagein(mAudio->GetStreamStatePtr(), page);
		}

		return ret;
	}

	void StopVideo(){

	}

	void RegisterVideoNotifier(VideoNotifierFunc func){
		mNotifierFunc = func;
	}

	void SetDurationAfterFinish(TIME_PRECISION time){
		mDurationAfterFinish = time;
	}

	void StripeDecoded(th_ycbcr_buffer _src, int _fragy0, int _fragy_end){
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

	void open_video(){
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


			auto& renderer = Renderer::GetInstance();
			mTextures[pli] = renderer.CreateTexture(0, mYCbCr[pli].width, mYCbCr[pli].height,
				PIXEL_FORMAT_A8_UNORM, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE, TEXTURE_TYPE_DEFAULT);			
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
	void Update(TIME_PRECISION dt){
		if (mFinish){
			if (mDurationAfterFinish > 0){				
				mDurationAfterFinish -= dt;
			}
			return;
		}

		if (mIsFirstFrame){
			mIsFirstFrame = false;
			mStartedTime = gpTimer->GetTime();
		}

		while (!mVideoBufReady && !mPacketsTheora.empty()){
			auto packet = mPacketsTheora.front();
			mPacketsTheora.erase(mPacketsTheora.begin());
			int ret = 0;
			if ((ret = th_decode_packetin(mTheoraDecoderCtx, &packet, &mGranulePos)) >= 0){
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
			if (mNotifierFunc){
				mNotifierFunc(mSelf.lock().get());
			}
			return;
		}
		TIME_PRECISION curTime = gpTimer->GetTime() - mStartedTime;
		if (mVideoBufTime < curTime){
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

		if (mAudio){
			mAudio->Update(dt);
		}
	}

	void Render(){
		RenderEventMarker mark("Video Rendering");
		for (int pli = 0; pli<3; pli++){
			mTextures[pli]->Bind(BINDING_SHADER_PS, pli);
		}
		auto& renderer = Renderer::GetInstance();
		renderer.GetResourceProvider()->BindDepthStencilState(ResourceTypes::DepthStencilStates::NoDepthStencil);
		renderer.DrawFullscreenQuad(mPS, false);
	}

	bool IsFinish(){
		return mFinish && mDurationAfterFinish <= 0.f;
	}

};

namespace fb{
	void stripe_decoded(VideoPlayerOgg::Impl* player, th_ycbcr_buffer _src,
		int _fragy0, int _fragy_end){
		player->StripeDecoded(_src, _fragy0, _fragy_end);
	}
}

//---------------------------------------------------------------------------
VideoPlayerOggPtr VideoPlayerOgg::Create(){
	VideoPlayerOggPtr p(new VideoPlayerOgg, [](VideoPlayerOgg* obj){ delete obj; });
	p->mImpl->mSelf = p;
	return p;
}
VideoPlayerOgg::VideoPlayerOgg()
	: mImpl(new Impl)
{
	
}

VideoPlayerOgg::~VideoPlayerOgg(){

}

bool VideoPlayerOgg::PlayVideo(const char* path) {
	return mImpl->PlayVideo(path);
}

void VideoPlayerOgg::StopVideo() {
	mImpl->StopVideo();
}

void VideoPlayerOgg::RegisterVideoNotifier(VideoNotifierFunc func) {
	mImpl->RegisterVideoNotifier(func);
}

void VideoPlayerOgg::SetDurationAfterFinish(TIME_PRECISION time) {
	mImpl->SetDurationAfterFinish(time);
}

void VideoPlayerOgg::Update(TIME_PRECISION dt) {
	mImpl->Update(dt);
}

void VideoPlayerOgg::Render(){
	mImpl->Render();
}

bool VideoPlayerOgg::IsFinish() const {
	return mImpl->IsFinish();
}

