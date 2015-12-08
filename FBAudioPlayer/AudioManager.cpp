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
	AudioBuffer()
		: mLastAccessed(0)
		, mBuffer(0)
		, mReferences(0)
	{

	}
	~AudioBuffer(){
		alDeleteBuffers(1, &mBuffer);
	}
};

class AudioManager::Impl{
public:
	ALCdevice* mDevice;
	ALCcontext* mContext;
	static LPALGETSOURCEDVSOFT alGetSourcedvSOFT;
	VectorMap<std::string, AudioBufferPtr> mAudioBuffers;
	VectorMap<AudioId, ALuint> mAudioSources;
	// Source, Buffer
	VectorMap<ALuint, std::string> mBufferBySource;

	//---------------------------------------------------------------------------
	Impl()
		: mDevice(0)
		, mContext(0)
	{
	}

	~Impl(){
		
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

	AudioId PlayAudio(const char* path, ALuint* srcId = 0){
		if (!ValidCStringLength(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return INVALID_AUDIO_ID;
		}
		using namespace std::chrono;
		auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

		std::string pathKey(path);
		ToLowerCase(pathKey);
		std::pair<AudioId, ALuint> audioIdSrcId;
		auto it = mAudioBuffers.Find(pathKey);
		if (it != mAudioBuffers.end()){
			it->second->mLastAccessed = curTick;
			audioIdSrcId = PlayAudioBuffer(it->second, pathKey);
		}
		else{
			auto buffer = alureCreateBufferFromFile(path);
			if (!buffer){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot create audio buffer for(%s)", path).c_str());
				return INVALID_AUDIO_ID;
			}
			AudioBufferPtr audioBuffer(new AudioBuffer);
			audioBuffer->mBuffer = buffer;
			audioBuffer->mFilepath = path;
			audioBuffer->mLastAccessed = curTick;
			mAudioBuffers[pathKey] = audioBuffer;
			audioIdSrcId = PlayAudioBuffer(audioBuffer, pathKey);
		}
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
		}
		return id;
	}

	bool SetPosition(AudioId id, float x, float y, float z){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSource3f(it->second, AL_POSITION, x, y, z);			
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "Setting position for invalid audio(%u)", id);
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
			Logger::Log(FB_ERROR_LOG_ARG, "Setting the relative flag for invalid audio(%u)", id);
			return false;
		}
	}

	bool SetMaxDistance(AudioId id, float distance){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			alSourcef(it->second, AL_MAX_DISTANCE, distance);
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "Setting max distance for invalid audio(%u)", id);
			return false;
		}
	}

	void OnPlayFinished(void* userdata, ALuint source){
		AudioId id = (AudioId)userdata;
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

bool AudioManager::SetPosition(AudioId id, float x, float y, float z){
	return mImpl->SetPosition(id, x, y, z);
}

bool AudioManager::SetRelative(AudioId id, bool relative){
	return mImpl->SetRelative(id, relative);
}

void AudioManager::SetListenerPosition(float x, float y, float z){
	alListener3f(AL_POSITION, x, y, z);
}

bool AudioManager::SetMaxDistance(AudioId id, float distance){
	return mImpl->SetMaxDistance(id, distance);
}