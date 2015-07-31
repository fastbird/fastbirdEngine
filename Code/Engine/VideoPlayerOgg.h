#pragma once
#include <Engine/IVideoPlayer.h>
#include <theora/theoradec.h>
namespace fastbird{
	class Audio;
	class VideoPlayerOgg : public IVideoPlayer{
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

		SmartPtr<ITexture> mTextures[3];
		SmartPtr<IShader> mPS;

		VideoNotifierFunc mNotifierFunc;

		float mDurationAfterFinish;
		bool mFinish;

		bool mVideoBufReady;
		bool mIsFirstFrame;
		float mStartedTime;

		Audio* mAudio;
		

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