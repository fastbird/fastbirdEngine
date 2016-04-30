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
#include "Audio.h"
#include "AudioHelper.h"
#include "FBCommonHeaders/Helpers.h"
namespace fb{
	LPALBUFFERSAMPLESSOFT alBufferSamplesSOFT = wrap_BufferSamples;
	LPALISBUFFERFORMATSUPPORTEDSOFT alIsBufferFormatSupportedSOFT;
}
using namespace fb;

class Audio::Impl
{
public:
	std::string mFilepath;	
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
	ALenum mAudioFormat;
	ALenum mAudioChannel;

	//---------------------------------------------------------------------------
	Impl()
		: mError(false)
		, mConvSize(4096)
	{
		for (int i = 0; i < NUM_BUFFERS; ++i){
			mAudioBuffers[i] = 0;
		}
		static bool CheckAlFunc = true;
		mStreamStateVorbis.body_data = 0;
		mVorbisDspState.vi = 0;
		if (CheckAlFunc){
			CheckAlFunc = false;
			if (alIsExtensionPresent("AL_SOFT_buffer_samples"))
			{
				Logger::Log(FB_DEFAULT_LOG_ARG, "AL_SOFT_buffer_samples supported!");
				alBufferSamplesSOFT = (LPALBUFFERSAMPLESSOFT)alGetProcAddress("alBufferSamplesSOFT");
				alIsBufferFormatSupportedSOFT = (LPALISBUFFERFORMATSUPPORTEDSOFT)alGetProcAddress("alIsBufferFormatSupportedSOFT");
			}
			else
			{
				Logger::Log(FB_DEFAULT_LOG_ARG, "AL_SOFT_buffer_samples not supported");
			}
		}
	}

	void Clear(){
		if (mStreamStateVorbis.body_data != 0){
			ogg_stream_clear(&mStreamStateVorbis);
		}

		vorbis_comment_clear(&mVorbisComment);
		vorbis_info_clear(&mVorbisInfo);

		if (mVorbisDspState.vi){
			alDeleteBuffers(NUM_BUFFERS, mAudioBuffers);
			alDeleteSources(1, &mAudioSource);
			vorbis_dsp_clear(&mVorbisDspState);
			mVorbisDspState.vi = 0;
		}

		mFilepath.clear();
	}

	void InitOgg(){
		vorbis_info_init(&mVorbisInfo);
		vorbis_comment_init(&mVorbisComment);
		mStreamStateVorbis.body_data = 0;
	}

	// returning processed
	bool ProcessOggHeaderPacket(ogg_stream_state* unknownStream, ogg_packet* packet){
		if (mStreamStateVorbis.body_data == 0){
			auto found = vorbis_synthesis_headerin(&mVorbisInfo, &mVorbisComment, packet) == 0;
			if (found){
				memcpy(&mStreamStateVorbis, unknownStream, sizeof(*unknownStream));
				ogg_stream_packetout(&mStreamStateVorbis, NULL);
				return true;
			}
		}
		return false;
	}

	bool ProcessOggHeader(){
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
						mError = true;
						break;
					}
					--count;
				}
				else{
					Error("Vorbis header data is corrupted.");
					mError = true;
					break;
				}
			}
			mAudioChannel = ChannelsToAL(mVorbisInfo.channels);
			if (vorbis_synthesis_init(&mVorbisDspState, &mVorbisInfo) == 0){
				alListener3f(AL_POSITION, 0, 0, 0);
				// AL Buffers
				alGenBuffers(NUM_BUFFERS, mAudioBuffers);
				assert(alGetError() == AL_NO_ERROR);
				alGenSources(1, &mAudioSource);
				assert(alGetError() == AL_NO_ERROR);
				alSource3f(mAudioSource, AL_POSITION, 0, 0, 0);
				//alSourcei(mAudioSource, AL_SOURCE_RELATIVE, AL_TRUE);
				//alSourcei(mAudioSource, AL_ROLLOFF_FACTOR, 0);
				assert(alGetError() == AL_NO_ERROR && "Could not set source parameters");


				mAudioFormat = GetAudioFormat(mAudioChannel, AL_SHORT_SOFT, alIsBufferFormatSupportedSOFT);
				if (mAudioFormat == 0){
					Error("Cannot recognize the audio format!");
					mError = true;
				}
				else{
					alSourceRewind(mAudioSource);
					alSourcei(mAudioSource, AL_BUFFER, 0);

					mConvSize = mConvSize / mVorbisInfo.channels;
					vorbis_block_init(&mVorbisDspState, &mVorbisBlock);
					int i;
					for (i = 0; i < NUM_BUFFERS; ++i){
						auto samples = DecodeOggAudio();
						if (samples == 0)
							break;

						auto frames = BytesToFrames(samples * 2 * mVorbisInfo.channels, mAudioChannel, AL_SHORT_SOFT);
						alBufferSamplesSOFT(mAudioBuffers[i], mVorbisInfo.rate, mAudioFormat,
							frames, mAudioChannel, AL_SHORT_SOFT, mConvBuffer);

					}
					auto err = alGetError();
					if (err){
						Error("alBufferAmplesSOFT error!");
						mError = true;
					}

					alSourceQueueBuffers(mAudioSource, i, mAudioBuffers);
					err = alGetError();
					if (err){
						Error("AL Queue buffers error.");
						mError = true;
					}
					alSourcePlay(mAudioSource);
					err = alGetError();
					if (err)
					{
						Error("Error starting playback!");
						mError = true;
					}
				}
			}
		}
		return !mError;
	}

	int GetVorbisSerialno() const{
		return mStreamStateVorbis.serialno;
	}

	ogg_stream_state* GetStreamStatePtr(){
		return &mStreamStateVorbis;
	}

	unsigned DecodeOggAudio(){
		if (mError)
			return 0;

		ogg_packet packet;
		float **pcm = 0;
		int samples = 0;
		while (samples == 0 && ogg_stream_packetpeek(&mStreamStateVorbis, &packet) > 0){
			if (vorbis_synthesis(&mVorbisBlock, &packet) == 0) /* test for success! */{
				int err = vorbis_synthesis_blockin(&mVorbisDspState, &mVorbisBlock);
				assert(err == 0);
				ogg_stream_packetout(&mStreamStateVorbis, 0);
				samples = vorbis_synthesis_pcmout(&mVorbisDspState, &pcm);
			}
		}


		if (samples > 0){
			int j;
			int clipflag = 0;
			int bout = (samples < mConvSize ? samples : mConvSize);

			/* convert floats to 16 bit signed ints (host order) and
			interleave */
			for (int i = 0; i < mVorbisInfo.channels; i++){
				ogg_int16_t *ptr = mConvBuffer + i;
				float  *mono = pcm[i];
				for (j = 0; j < bout; j++){
#if 1
					int val = (int)floor(mono[j] * 32767.f + .5f);
#else /* optional dither */
					int val = mono[j] * 32767.f + drand48() - 0.5f;
#endif
					/* might as well guard against clipping */
					if (val > 32767){
						val = 32767;
						clipflag = 1;
					}
					if (val < -32768){
						val = -32768;
						clipflag = 1;
					}
					*ptr = val;
					ptr += mVorbisInfo.channels;
				}
			}

			vorbis_synthesis_read(&mVorbisDspState, bout); /* tell libvorbis how
														   many samples we
														   actually consumed */
			return bout;
		}
		return 0;
	}

	bool PlayAudio(const char* path){
		if (!ValidCStringLength(path)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");			
			return false;
		}

		{
			FileSystem::Open file(path, "rb");
			auto err = file.Error();
			if (err) {
				Logger::Log(FB_ERROR_LOG_ARG, "Cannot open the file.");
				return false;
			}
		}

		if (!mFilepath.empty())
			Clear();
		mAudioBuffers[0] = alureCreateBufferFromFile(path);
		if (!mAudioBuffers[0]){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot load audio(%s)", path).c_str());
		}


		return true;
	}

	void Update(float dt){
		/*
		**pcm is a multichannel float vector.  In stereo, for
		example, pcm[0] is left, and pcm[1] is right.  samples is
		the size of each channel.  Convert the float values
		(-1.<=range<=1.) to whatever PCM format and write it out */
		ALint processed, state;
		alGetSourcei(mAudioSource, AL_SOURCE_STATE, &state);
		alGetSourcei(mAudioSource, AL_BUFFERS_PROCESSED, &processed);
		if (alGetError() != AL_NO_ERROR)
		{
			Error("Error checking source state!");
			return;
		}

		while (processed > 0 && !mError){
			processed--;
			unsigned samples = DecodeOggAudio();
			if (samples > 0){
				// to AL

				ALuint bufid;

				alSourceUnqueueBuffers(mAudioSource, 1, &bufid);
				auto frames = BytesToFrames(samples * 2 * mVorbisInfo.channels, mAudioChannel, AL_SHORT_SOFT);
				alBufferSamplesSOFT(bufid, mVorbisInfo.rate, mAudioFormat, frames,
					mAudioChannel, AL_SHORT_SOFT, mConvBuffer);
				alSourceQueueBuffers(mAudioSource, 1, &bufid);
				if (alGetError() != AL_NO_ERROR)
				{
					Error("Error buffering audio data!");
					mError = true;
				}
			}
		}

		if (!mError && state != AL_PLAYING && state != AL_PAUSED)
		{
			ALint queued;

			/* If no buffers are queued, playback is finished */
			alGetSourcei(mAudioSource, AL_BUFFERS_QUEUED, &queued);
			if (queued == 0)
				return;

			alSourcePlay(mAudioSource);
			if (alGetError() != AL_NO_ERROR)
			{
				Error("Error restarting playback\n");
				mError = true;
			}
		}
	}
};

FB_IMPLEMENT_STATIC_CREATE(Audio);

Audio::Audio()
	: mImpl(new Impl)
{	
}


void Audio::Clear(){
	mImpl->Clear();
}

void Audio::InitOgg(){
	mImpl->InitOgg();
}

bool Audio::ProcessOggHeaderPacket(ogg_stream_state* unknownStream, ogg_packet* packet){
	return mImpl->ProcessOggHeaderPacket(unknownStream, packet);
}

bool Audio::PlayAudio(const char* path){
	return mImpl->PlayAudio(path);
}

void Audio::Update(float dt){
	mImpl->Update(dt);
}

bool Audio::ProcessOggHeader(){
	return mImpl->ProcessOggHeader();
}

int Audio::GetVorbisSerialno() const{
	return mImpl->GetVorbisSerialno();
}

ogg_stream_state* Audio::GetStreamStatePtr(){
	return mImpl->GetStreamStatePtr();
}