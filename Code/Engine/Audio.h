#pragma once
#include <vorbis/codec.h>

namespace fastbird{
	class Audio{
		friend class VideoPlayerOgg;
		std::string mFilepath;
		FILE* mFile;
		bool mError;
		ogg_stream_state mStreamStateVorbis;
		std::vector<ogg_packet> mPacketsVorbis;

		vorbis_info mVorbisInfo;
		vorbis_comment mVorbisComment;
		vorbis_dsp_state mVorbisDspState;
		vorbis_block mVorbisBlock;
		int mConvSize;
		ogg_int16_t mConvBuffer[4096];				
		
		ALenum mChannel;
		static const int NUM_BUFFERS = 4;
		ALuint mAudioBuffers[NUM_BUFFERS];
		ALuint mAudioSource;

		static bool CheckAlFunc;
		ALenum mAudioFormat;
		ALenum mAudioChannel;

	public:
		Audio();
		~Audio();
		void Clear();
		void InitOgg();
		// returning processed
		bool ProcessOggHeaderPacket(ogg_stream_state* unknownStream, ogg_packet* packet);
		bool PlayAudio(const char* path);
		void Update(float dt);

	private:
		bool ProcessOggHeader();
		unsigned DecodeAudio();  // returning samples decoded
	};
}