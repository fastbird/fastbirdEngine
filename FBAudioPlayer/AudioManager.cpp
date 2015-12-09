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
#include "AudioManager.h"
#include "AudioHelper.h"
#include "AudioFadeOut.h"
#include "AudioFadeIn.h"
#include "AudioEx.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/Helpers.h"
using namespace fb;

AudioId NextAudioId = 1;
namespace fb{
	static void eos_callback(void *userData, ALuint source);
}
FB_DECLARE_SMART_PTR_STRUCT(AudioBuffer);
struct AudioBuffer{
	std::string mFilepath;
	INT64 mLastAccessed;
	ALuint mBuffer;
	unsigned mReferences;
	TIME_PRECISION mLength;

	AudioBuffer()
		: mLastAccessed(0)
		, mBuffer(0)
		, mReferences(0)
		, mLength(0)
	{

	}
	~AudioBuffer(){
		alDeleteBuffers(1, &mBuffer);
	}
};

//---------------------------------------------------------------------------
class AudioManager::Impl{
public:
	ALCdevice* mDevice;
	ALCcontext* mContext;
	static LPALGETSOURCEDVSOFT alGetSourcedvSOFT;
	VectorMap<std::string, AudioBufferPtr> mAudioBuffers;
	VectorMap<AudioId, ALuint> mAudioSources;
	// Source, Buffer
	VectorMap<ALuint, std::string> mBufferBySource;
	std::vector<IAudioManipulatorPtr> mAudioManipulators;
	std::vector<AudioExPtr> mAudioExs;

	//---------------------------------------------------------------------------
	Impl()
		: mDevice(0)
		, mContext(0)
	{
	}

	~Impl(){
		// use deinit()
	}

	bool Init(){
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

	void Deinit(){
		mAudioExs.clear();
		for (auto it : mAudioSources){
			alDeleteSources(1, &it.second);
		}
		mAudioBuffers.clear();	

		alcMakeContextCurrent(NULL);
		alcDestroyContext(mContext);
		alcCloseDevice(mDevice);
		mDevice = 0;
	}

	void Update(TIME_PRECISION dt){
		static TIME_PRECISION accumulator = 0;
		for (auto it = mAudioManipulators.begin(); it != mAudioManipulators.end(); /**/){
			bool finished = (*it)->Update(dt);
			if (finished){
				it = mAudioManipulators.erase(it);
			}
			else{
				++it;
			}
		}
		for (auto it = mAudioExs.begin(); it != mAudioExs.end(); /**/){
			bool finished = (*it)->Update();
			if (finished){
				it = mAudioExs.erase(it);
			}
			else{
				++it;
			}
		}


		if (mAudioSources.empty())
			return;
		
		accumulator += dt;
		if (accumulator >= 0.125f){
			accumulator -= 0.125f;
			alureUpdate();			
		}

		static TIME_PRECISION accumulatorForBuffer = 0;
		accumulatorForBuffer += dt;
		if (accumulatorForBuffer >= 60.f){
			accumulatorForBuffer -= 60.f;
			using namespace std::chrono;
			auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			for (auto it = mAudioBuffers.begin(); it != mAudioBuffers.end(); /**/){
				if (it->second->mReferences==0 && curTick - it->second->mLastAccessed > 60000){
					it = mAudioBuffers.erase(it);
				}
				else{
					++it;
				}
			}
		}

	}
	
	// returning audioid + sourceId;
	std::pair<AudioId, ALuint> PlayAudioBuffer(AudioBufferPtr buffer, std::string pathKey){
		ALuint src;
		alGenSources(1, &src);
		if (alGetError() != AL_NO_ERROR){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot create audio source for (%s).", buffer->mFilepath.c_str());
			return std::make_pair(INVALID_AUDIO_ID, 0);
		}
		alSourcei(src, AL_BUFFER, buffer->mBuffer);
		AudioId audioId = NextAudioId++;		
		if (alurePlaySource(src, eos_callback, (void*)audioId) == AL_FALSE){
			alDeleteSources(1, &src);
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot play AudioSource(%d, %s)", src, buffer->mFilepath.c_str()).c_str());
			return std::make_pair(INVALID_AUDIO_ID, 0);
		}
		mAudioSources[audioId] = src;
		mBufferBySource[src] = pathKey;
		++buffer->mReferences;
		return std::make_pair(audioId, src);
	}

	AudioBufferPtr GetAudioBuffer(std::string path, std::string loweredPath){
		if (path.size() != loweredPath.size()){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		using namespace std::chrono;
		auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();		
		std::pair<AudioId, ALuint> audioIdSrcId;
		auto it = mAudioBuffers.Find(loweredPath);
		if (it != mAudioBuffers.end()){
			it->second->mLastAccessed = curTick;
			return it->second;
		}
		else{
			auto buffer = alureCreateBufferFromFile(path.c_str());
			if (!buffer){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot create audio buffer for(%s)", path.c_str()).c_str());
				return 0;
			}
			AudioBufferPtr audioBuffer(new AudioBuffer);
			audioBuffer->mBuffer = buffer;
			audioBuffer->mFilepath = path;
			audioBuffer->mLastAccessed = curTick;
			ALint bufferSize, frequency, channels, bit;
			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_FREQUENCY, &frequency);
			alGetBufferi(buffer, AL_CHANNELS, &channels);
			alGetBufferi(buffer, AL_BITS, &bit);
			audioBuffer->mLength = GetDuration(bufferSize, frequency, channels, bit);
			mAudioBuffers[loweredPath] = audioBuffer;
			return audioBuffer;
		}
	}

	AudioId PlayAudio(const char* path, ALuint* srcId = 0){
		if (!ValidCStringLength(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return INVALID_AUDIO_ID;
		}
		

		std::string pathKey(path);
		ToLowerCase(pathKey);
		auto audioBuffer = GetAudioBuffer(path, pathKey);
		auto audioIdSrcId = PlayAudioBuffer(audioBuffer, pathKey);
		if (audioIdSrcId.first != INVALID_AUDIO_ID){
			if (srcId){
				*srcId = audioIdSrcId.second;
			}
		}

		return audioIdSrcId.first;
	}

	AudioId PlayAudio(const char* path, float x, float y, float z){
		ALuint src;
		auto id = PlayAudio(path, &src);
		if (id != INVALID_AUDIO_ID){
			alSource3f(src, AL_POSITION, x, y, z);
		}
		return id;
	}

	AudioId PlayAudio(const char* path, const AudioProperty& property){
		ALuint src;
		auto id = PlayAudio(path, &src);
		if (id != INVALID_AUDIO_ID){
			alSource3f(src, AL_POSITION, std::get<0>(property.mPosition), std::get<1>(property.mPosition), std::get<2>(property.mPosition));
			alSourcei(src, AL_SOURCE_RELATIVE, property.mRelative? AL_TRUE : AL_FALSE);			
			alSourcef(src, AL_REFERENCE_DISTANCE, property.mReferenceDistance);
			alSourcef(src, AL_ROLLOFF_FACTOR, property.mRolloffFactor);
			alSourcef(src, AL_GAIN, property.mGain);
		}
		return id;
	}

	AudioId PlayAudioWithFadeIn(const char* path, const AudioProperty& prop, TIME_PRECISION sec){
		auto id = PlayAudio(path, prop);
		if (id != INVALID_AUDIO_ID){
			SetGain(id, 0.f);
			auto length = GetAudioLength(id);
			auto fadeIn = AudioFadeIn::Create(id, std::min(length, sec), prop.mGain);
			mAudioManipulators.push_back(fadeIn);
		}
		return id;
	}

	bool StopAudio(AudioId id){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alureStopSource(it->second, AL_TRUE);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Stop invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool StopWithFadeOut(AudioId id, TIME_PRECISION sec){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){			
			auto leftTime = GetAudioLeftTime(id);
			auto fadeOut = AudioFadeOut::Create(id, std::min(leftTime, sec));
			mAudioManipulators.push_back(fadeOut);			
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Stop invalid audio(%u)", id).c_str());
			return false;
		}
	}

	TIME_PRECISION GetAudioLeftTime(AudioId id){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){			
			ALfloat offset;
			alGetSourcef(it->second, AL_SEC_OFFSET, &offset);
			ALint bufferSize, frequency, channels, bit;
			ALint buffer;
			alGetSourcei(it->second, AL_BUFFER, &buffer);
			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_FREQUENCY, &frequency);
			alGetBufferi(buffer, AL_CHANNELS, &channels);
			alGetBufferi(buffer, AL_BITS, &bit);
			float duration = GetDuration(bufferSize, frequency, channels, bit);
			return duration - offset;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Canno get a length for invalid audio(%u)", id).c_str());
			return 0;
		}
	}

	TIME_PRECISION GetAudioLength(const char* path){
		std::string lowered(path);
		ToLowerCase(lowered);
		auto buffer = GetAudioBuffer(path, lowered);
		if (!buffer){
			return 0.f;
		}
		return buffer->mLength;
	}

	TIME_PRECISION GetAudioLength(AudioId id) {
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			ALint bufferSize, frequency, channels, bit;
			ALint buffer;
			alGetSourcei(it->second, AL_BUFFER, &buffer);
			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_FREQUENCY, &frequency);
			alGetBufferi(buffer, AL_CHANNELS, &channels);
			alGetBufferi(buffer, AL_BITS, &bit);
			return GetDuration(bufferSize, frequency, channels, bit);			
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Canno get a length for invalid audio(%u)", id).c_str());
			return 0;
		}
	}

	bool SetPosition(AudioId id, float x, float y, float z){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSource3f(it->second, AL_POSITION, x, y, z);			
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting position for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetRelative(AudioId id, bool relative){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcei(it->second, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting the relative flag for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetReferenceDistance(AudioId id, float distance){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcef(it->second, AL_REFERENCE_DISTANCE, distance);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting max distance for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetRolloffFactor(AudioId id, float factor){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcef(it->second, AL_ROLLOFF_FACTOR, factor);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting max distance for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetOffsetInSec(AudioId id, float sec){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcef(it->second, AL_SEC_OFFSET, sec);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting offset for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetGain(AudioId id, float gain){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcef(it->second, AL_GAIN, gain);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting gain for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	float GetGain(AudioId id) const{
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			ALfloat gain;
			alGetSourcef(it->second, AL_GAIN, &gain);
			return gain;			
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting gain for invalid audio(%u)", id).c_str());
			return 0.f;
		}
	}

	void RegisterAudioEx(AudioExPtr audioex){
		if (!ValueExistsInVector(mAudioExs, audioex)){
			mAudioExs.push_back(audioex);
		}
	}

	void OnPlayFinished(void* userdata, ALuint source){
		AudioId id = (AudioId)userdata;
		for (auto& it : mAudioExs){
			it->OnFinish(id);
		}

		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Audio(%u) is finished.", id).c_str());
			mAudioSources.erase(it);
		}
		else{
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Audio(%u) is not found.", id).c_str());
		}

		auto bufferIt = mBufferBySource.Find(source);
		if (bufferIt != mBufferBySource.end()){
			auto buffer = bufferIt->second;
			auto bIt = mAudioBuffers.Find(buffer);
			if (bIt != mAudioBuffers.end()){
				assert(bIt->second->mReferences > 0);
				--bIt->second->mReferences;
			}
			
		}
	}
};
LPALGETSOURCEDVSOFT AudioManager::Impl::alGetSourcedvSOFT = 0;

//---------------------------------------------------------------------------
AudioManager* sAudioManagerRaw = 0;
AudioManagerWeakPtr sAudioManager;

namespace fb{
	static void eos_callback(void *userData, ALuint source)
	{
		if (sAudioManagerRaw){
			sAudioManagerRaw->mImpl->OnPlayFinished(userData, source);
		}

	}
}
AudioManagerPtr AudioManager::Create(){
	if (sAudioManager.expired()){
		AudioManagerPtr p(new AudioManager, [](AudioManager* obj){ delete obj; });
		sAudioManagerRaw = p.get();
		sAudioManager = p;
		return p;
	}
	return sAudioManager.lock();
}

AudioManager& AudioManager::GetInstance(){
	if (sAudioManager.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "AudioManager is deleted. Program will crash...");
	}
	return *sAudioManagerRaw;
}

AudioManager::AudioManager()
	: mImpl(new Impl)
{
}
AudioManager::~AudioManager()
{

}

bool AudioManager::Init(){
	return mImpl->Init();
}

void AudioManager::Deinit(){
	mImpl->Deinit();
}

void AudioManager::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}

AudioId AudioManager::PlayAudio(const char* path){
	return mImpl->PlayAudio(path);
}

AudioId AudioManager::PlayAudio(const char* path, float x, float y, float z){
	return mImpl->PlayAudio(path, x, y, z);
}

AudioId AudioManager::PlayAudio(const char* path, const AudioProperty& property){
	return mImpl->PlayAudio(path, property);
}

AudioId AudioManager::PlayAudioWithFadeIn(const char* path, const AudioProperty& prop, TIME_PRECISION sec){
	return mImpl->PlayAudioWithFadeIn(path, prop, sec);
}

bool AudioManager::StopAudio(AudioId id){
	return mImpl->StopAudio(id);
}

TIME_PRECISION AudioManager::GetAudioLength(const char* path) {
	return mImpl->GetAudioLength(path);
}

TIME_PRECISION AudioManager::GetAudioLength(AudioId id) {
	return mImpl->GetAudioLength(id);
}

bool AudioManager::StopWithFadeOut(AudioId id, TIME_PRECISION sec){
	return mImpl->StopWithFadeOut(id, sec);
}

TIME_PRECISION AudioManager::GetAudioLeftTime(AudioId id){
	return mImpl->GetAudioLeftTime(id);
}

bool AudioManager::SetPosition(AudioId id, float x, float y, float z){
	return mImpl->SetPosition(id, x, y, z);
}

bool AudioManager::SetRelative(AudioId id, bool relative){
	return mImpl->SetRelative(id, relative);
}

void AudioManager::SetListenerPosition(float x, float y, float z){
	alListener3f(AL_POSITION, x, y, z);
}

bool AudioManager::SetReferenceDistance(AudioId id, float distance){
	return mImpl->SetReferenceDistance(id, distance);
}

bool AudioManager::SetRolloffFactor(AudioId id, float factor){
	return mImpl->SetRolloffFactor(id, factor);
}

bool AudioManager::SetOffsetInSec(AudioId id, float sec){
	return mImpl->SetOffsetInSec(id, sec);
}

bool AudioManager::SetGain(AudioId id, float gain){
	return mImpl->SetGain(id, gain);
}

float AudioManager::GetGain(AudioId id) const{
	return mImpl->GetGain(id);
}

void AudioManager::RegisterAudioEx(AudioExPtr audioex){
	return mImpl->RegisterAudioEx(audioex);
}