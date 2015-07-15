#pragma once
#include <Engine/IVideoPlayer.h>
#include <theora/theoradec.h>
#include <vorbis/codec.h>
namespace fastbird{
	class VideoPlayerOgg : public IVideoPlayer{
		ogg_sync_state mSyncState;

		ogg_stream_state mStreamStateTheora;
		ogg_stream_state mStreamStateVorbis;

		std::vector<ogg_packet> mPacketsTheora;
		std::vector<ogg_packet> mPacketsVorbis;
		
		th_info mTheoraInfo;
		th_comment mTheoraComment;
		th_setup_info* mTheoraSetup;
		th_dec_ctx* mTheoraDecoderCtx;
		th_ycbcr_buffer mYCbCr;

		vorbis_info mVorbisInfo;
		vorbis_comment mVorbisComment;
		vorbis_dsp_state mVorbisDspState;
		vorbis_block mVorbisBlock;

		FILE* mFile;
		
		std::string mFilepath;
		
		double mPlayTime;
		double mVideoBufTime;
		double mAudioBufTime;		
		ogg_int64_t mGranulePos;	

		SmartPtr<ITexture> mTextures[3];
		SmartPtr<IShader> mPS;

		VideoNotifierFunc mNotifierFunc;

		float mDurationAfterFinish;
		bool mFinish;

		bool mVideoBufReady;
		

	private:
		friend void stripe_decoded(VideoPlayerOgg* player, th_ycbcr_buffer _src, int _fragy0, int _fragy_end);
		int buffer_data();
		int queue_page(ogg_page *page);
		void open_video(void);
		void StripeDecoded(th_ycbcr_buffer _src, int _fragy0, int _fragy_end);
		void Display();

	public:
		VideoPlayerOgg();
		~VideoPlayerOgg();

		void Clear();

		virtual bool PlayVideo(const char* path);
		virtual void StopVideo();

		virtual void RegisterVideoNotifier(VideoNotifierFunc func);
		virtual void SetDurationAfterFinish(float time);
		virtual void Update(float dt);

		virtual bool IsFinish() const{ return mFinish; }

	
	};
}