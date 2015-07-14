#pragma once
#include <Engine/IVideoPlayer.h>
#include <theora/theoradec.h>
namespace fastbird{
	class VideoPlayerOgg : public IVideoPlayer{
		ogg_stream_state* mStreamState;
		ogg_sync_state* mSyncState;		
		ogg_page* mPage;
		ogg_packet* mPacket;
		th_info* mInfo;
		th_comment* mComment;
		th_setup_info* mSetup;
		th_dec_ctx* mDecoderCtx;
		th_ycbcr_buffer mYCbCr;
		FILE* mFile;
		
		std::string mFilepath;
		int mStateFlag;
		int mIsTheora;
		int mIsProcessingTheoraHeader;

		double mPlayTime;
		double mVideoBufTime;
		ogg_int64_t mGranulePos;
		bool mVideoBufReady;
		bool mFinish;

		SmartPtr<ITexture> mTextures[3];
		SmartPtr<IShader> mPS;
		

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
		virtual void Update(float dt);

		virtual bool IsFinish() const{ return mFinish; }

	
	};
}