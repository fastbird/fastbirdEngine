#include <Engine/StdAfx.h>
#include <Engine/Audio.h>
#include <Engine/AudioHelper.h>

namespace fastbird{
	bool Audio::CheckAlFunc = true;
	LPALBUFFERSAMPLESSOFT alBufferSamplesSOFT = wrap_BufferSamples;
	LPALISBUFFERFORMATSUPPORTEDSOFT alIsBufferFormatSupportedSOFT;

}
using namespace fastbird;

Audio::Audio()
	: mError(false)
	, mConvSize(4096)
{
	mStreamStateVorbis.body_data = 0;
	mVorbisDspState.vi = 0;
	if (CheckAlFunc){
		CheckAlFunc = false;
		if (alIsExtensionPresent("AL_SOFT_buffer_samples"))
		{
			Log("AL_SOFT_buffer_samples supported!");
			alBufferSamplesSOFT = (LPALBUFFERSAMPLESSOFT)alGetProcAddress("alBufferSamplesSOFT");
			alIsBufferFormatSupportedSOFT = (LPALISBUFFERFORMATSUPPORTEDSOFT)alGetProcAddress("alIsBufferFormatSupportedSOFT");
		}
		else
			printf("AL_SOFT_buffer_samples not supported\n");

	}
}

Audio::~Audio(){

}

void Audio::Clear(){
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

void Audio::InitOgg(){
	vorbis_info_init(&mVorbisInfo);
	vorbis_comment_init(&mVorbisComment);
	mStreamStateVorbis.body_data = 0;
}

// returning processed
bool Audio::ProcessOggHeaderPacket(ogg_stream_state* unknownStream, ogg_packet* packet){
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

bool Audio::ProcessOggHeader(){
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
					auto samples = DecodeAudio();
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

unsigned Audio::DecodeAudio(){
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

bool Audio::PlayAudio(const char* path){
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

	return true;
}

void Audio::Update(float dt){
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
		unsigned samples = DecodeAudio();
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