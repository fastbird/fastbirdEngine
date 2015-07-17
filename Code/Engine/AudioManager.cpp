#include <Engine/StdAfx.h>
#include <Engine/AudioManager.h>

using namespace fastbird;

LPALGETSOURCEDVSOFT AudioManager::alGetSourcedvSOFT = 0;

AudioManager::AudioManager()
	: mDevice(0)
	, mContext(0)
{

}

AudioManager::~AudioManager(){
	if (mDevice)
		Deinit();
}

bool AudioManager::Init(){
	mDevice = alcOpenDevice(NULL);
	if (!mDevice){
		Error("No sound device found!");
		return false;
	}

	mContext = alcCreateContext(mDevice, NULL);
	if (!mContext){
		Error("Cannot create the audio context!");
		return false;
	}

	if (alcMakeContextCurrent(mContext) == ALC_FALSE)
	{
		Error("Could not make context current!\n");
		return false;
	}

	if (!alIsExtensionPresent("AL_SOFT_source_length"))
	{
		Error("Required AL_SOFT_source_length not supported!\n");
		return false;
	}

	if (!alIsExtensionPresent("AL_SOFT_source_latency"))
		Error("AL_SOFT_source_latency not supported, audio may be a bit laggy.");
	else
	{
		alGetSourcedvSOFT = (LPALGETSOURCEDVSOFT)alGetProcAddress("alGetSourcedvSOFT");
	}
	Log("OpenAL initialized!");
	return true;
}
void AudioManager::Deinit(){
	alcMakeContextCurrent(NULL);
	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);
	mDevice = 0;
}