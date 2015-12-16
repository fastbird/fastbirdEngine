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

FB_DECLARE_SMART_PTR_STRUCT(AudioSource);
struct AudioSource{
private:
	AudioId mAudioId;
	AudioBufferPtr mAudioBuffer;
	ALuint mALSource;
	AudioProperty mProperty;
	float mPlayingTime;
	bool mPlaying;

public:

	AudioSource()
		: mALSource(-1)
		, mAudioId(INVALID_AUDIO_ID)
		, mPlayingTime(0)
		, mPlaying(false)
	{
	}
	~AudioSource(){
		if (mAudioBuffer)
			mAudioBuffer->mReferences--;
	}

	void SetAudioId(AudioId id){
		mAudioId = id;
	}
	const AudioId& GetAudioId() const{
		return mAudioId;
	}

	void SetAudioBuffer(AudioBufferPtr buffer){
		if (mAudioBuffer)
			mAudioBuffer->mReferences--;
		mAudioBuffer = buffer;
		buffer->mReferences++;
	}

	AudioBufferPtr GetAudioBuffer() const{
		return mAudioBuffer;
	}

	void SetALAudioSource(ALuint src){
		mALSource = src;
	}
	ALuint GetALAudioSource() const {
		return mALSource;
	}
	
	void SetPlaying(bool playing){
		mPlaying = playing;
	}	

	void SetProperty(const AudioProperty& prop){
		mProperty = prop;
	}

	void ApplyProp(){
		alSource3f(mALSource, AL_POSITION, std::get<0>(mProperty.mPosition), 
			std::get<1>(mProperty.mPosition), std::get<2>(mProperty.mPosition));
		alSourcei(mALSource, AL_SOURCE_RELATIVE, mProperty.mRelative ? AL_TRUE : AL_FALSE);
		alSourcef(mALSource, AL_REFERENCE_DISTANCE, mProperty.mReferenceDistance);
		alSourcef(mALSource, AL_ROLLOFF_FACTOR, mProperty.mRolloffFactor);
		alSourcef(mALSource, AL_GAIN, mProperty.mGain);
	}

	void ApplyRemainedTime(){
		alSourcef(mALSource, AL_SEC_OFFSET, mPlayingTime);
	}

	bool GetPlaying() const {
		return mPlaying;
	}

	bool Update(float dt){
		mPlayingTime += dt;
		if (!mPlaying && mPlayingTime >= mAudioBuffer->mLength)
			return true; // delete me

		return false; // dont delete me.
	}

	void SetPosition(float x, float y, float z){
		mProperty.mPosition = std::make_tuple(x, y, z);
		if (mPlaying){
			alSource3f(mALSource, AL_POSITION, x, y, z);
		}
	}
	void SetRelative(bool relative){
		mProperty.mRelative = relative;
		if (mPlaying){
			alSourcei(mALSource, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
		}
	}

	void SetReferenceDistance(float distance){
		mProperty.mReferenceDistance = distance;
		if (mPlaying){
			alSourcef(mALSource, AL_REFERENCE_DISTANCE, distance);
		}
	}

	void SetRollOffFactor(float factor){
		mProperty.mRolloffFactor = factor;
		if (mPlaying){
			alSourcef(mALSource, AL_ROLLOFF_FACTOR, factor);
		}
	}

	void SetOffsetInSec(float sec){
		mPlayingTime = sec;
		if (mPlaying){
			alSourcef(mALSource, AL_SEC_OFFSET, mPlayingTime);
		}
	}

	void SetGain(float gain){
		mProperty.mGain = gain;
		if (mPlaying){
			alSourcef(mALSource, AL_GAIN, gain);
		}
	}

	float GetGain() const{
		return mProperty.mGain;
	}

	float GetLeftTime() const{
		return mAudioBuffer->mLength - mPlayingTime;
	}

	float GetLength() const {
		return mAudioBuffer->mLength;
	}
};

FunctionId NextCallbackId = 1;
//---------------------------------------------------------------------------
class AudioManager::Impl{
public:
	static const int MaximumAudioSources = 32;
	ALCdevice* mDevice;
	ALCcontext* mContext;
	static LPALGETSOURCEDVSOFT alGetSourcedvSOFT;
	VectorMap<std::string, AudioBufferPtr> mAudioBuffers;	
	// source plyaing audio id
	std::stack<ALuint> mALSources;
	// audioId - source index
	VectorMap<AudioId, AudioSourcePtr> mAudioSources;
	VectorMap<AudioId, AudioSourcePtr> mReservedAudioSources;
	// Source, Buffer	
	std::vector<IAudioManipulatorPtr> mAudioManipulators;
	std::vector<AudioExPtr> mAudioExs;
	std::vector<AudioSourcePtr> mPlayList;
	std::vector<AudioSourcePtr> mPlayListSpatial;

	typedef VectorMap<FunctionId, CallbackFunction> CallbackMap;
	typedef VectorMap<AudioId, std::vector<FunctionId> > CallbackIdMap;
	CallbackMap mEndCallbacks;
	CallbackIdMap mEndCallbackIds;

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
		}

		if (!alIsExtensionPresent("AL_SOFT_source_latency"))
			Error("AL_SOFT_source_latency not supported, audio may be a bit laggy.");
		else
		{
			alGetSourcedvSOFT = (LPALGETSOURCEDVSOFT)alGetProcAddress("alGetSourcedvSOFT");
		}
		for (int i = 0; i < MaximumAudioSources; ++i){
			ALuint src;
			alGenSources(1, &src);
			if (alGetError() != AL_NO_ERROR){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot create audio source idx(%d).", i).c_str());
			}
			else{
				mALSources.push(src);
			}
		}
		Log("OpenAL initialized!");
		return true;
	}

	void Deinit(){
		mAudioExs.clear();

		while (!mAudioSources.empty()){
			auto it = mAudioSources.end() - 1;
			StopAudio(it->second->GetAudioId());
		}
		
		mReservedAudioSources.clear();
		assert(mALSources.size() == MaximumAudioSources);
		while (!mALSources.empty()){
			auto src = mALSources.top();
			mALSources.pop();
			alDeleteSources(1, &src);
		}
		mAudioBuffers.clear();	

		alcMakeContextCurrent(NULL);
		alcDestroyContext(mContext);
		alcCloseDevice(mDevice);
		mDevice = 0;
	}

	bool PlayWithSourceAndFadeIn(AudioSourcePtr audioSource, ALuint src, TIME_PRECISION fadeInTime){
		alSourcei(src, AL_BUFFER, audioSource->GetAudioBuffer()->mBuffer);
		if (alurePlaySource(src, eos_callback, (void*)audioSource->GetAudioId()) == AL_FALSE){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot play AudioSource(%u, %s)",
				audioSource->GetAudioId(), audioSource->GetAudioBuffer()->mFilepath.c_str()).c_str());
			audioSource->SetPlaying(false);
			return false;
		}
		else{
			audioSource->SetALAudioSource(src);
			audioSource->SetPlaying(true);
			audioSource->ApplyProp();
			audioSource->ApplyRemainedTime();
			auto fadeIn = AudioFadeIn::Create(audioSource->GetAudioId(), fadeInTime, audioSource->GetGain());
			mAudioManipulators.push_back(fadeIn);
			mAudioSources[audioSource->GetAudioId()] = audioSource;
			return true;
		}
	}

	void CheckReserved(){
		unsigned remainedSource = MaximumAudioSources - mAudioSources.size();
		assert(remainedSource == mALSources.size());
		for (auto it = mReservedAudioSources.begin(); remainedSource > 0 && it != mReservedAudioSources.end(); /**/){
			auto src = mALSources.top();
			auto audioSource = it->second;
			if (PlayWithSourceAndFadeIn(audioSource, src, 0.5f)){
				it = mReservedAudioSources.erase(it);				
				mALSources.pop();
			}
			else{
				++it;
			}			
		}
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

		CheckReserved();
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
	AudioSourcePtr PlayAudioBuffer(AudioBufferPtr buffer, std::string pathKey){
		AudioId audioId = NextAudioId++;
		AudioSourcePtr audioSource(new AudioSource);
		audioSource->SetAudioId(audioId);
		audioSource->SetAudioBuffer(buffer);		
		if (!mALSources.empty()){
			auto src = mALSources.top();			
			alSourcei(src, AL_BUFFER, buffer->mBuffer);			
			if (alurePlaySource(src, eos_callback, (void*)audioId) == AL_FALSE){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot play AudioSource(%u, %s)", audioId, buffer->mFilepath.c_str()).c_str());
				audioSource->SetPlaying(false);
			}
			else{
				audioSource->SetALAudioSource(src);
				mALSources.pop();			
				audioSource->SetPlaying(true);				
			}
		}		
		if (audioSource->GetPlaying()){			
			mAudioSources[audioId] = audioSource;
		}
		else{			
			mReservedAudioSources[audioId] = audioSource;
		}
		return audioSource;
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

	AudioId PlayAudio(const char* path, AudioSourcePtr* source){
		if (!ValidCStringLength(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return INVALID_AUDIO_ID;
		}
		

		std::string pathKey(path);
		ToLowerCase(pathKey);
		auto audioBuffer = GetAudioBuffer(path, pathKey);
		auto audioSource = PlayAudioBuffer(audioBuffer, pathKey);
		if (source){
			*source = audioSource;
		}
		return audioSource->GetAudioId();
	}

	AudioId PlayAudio(const char* path, float x, float y, float z){
		AudioSourcePtr src;
		auto id = PlayAudio(path, &src);
		src->SetPosition(x, y, z);		
		return id;
	}

	AudioId PlayAudio(const char* path, const AudioProperty& property){
		AudioSourcePtr src;
		auto id = PlayAudio(path, &src);
		src->SetProperty(property);
		src->ApplyProp();			
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
			auto& source = it->second;
			alureStopSource(source->GetALAudioSource(), AL_TRUE);
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
			auto leftTime = it->second->GetLeftTime();
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
			return it->second->GetLeftTime();
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
			return it->second->GetLength();			
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Canno get a length for invalid audio(%u)", id).c_str());
			return 0;
		}
	}

	FunctionId RegisterEndCallback(AudioId id, std::function< void(AudioId) > callback){
		mEndCallbacks[NextCallbackId] = callback;
		if (!ValueExistsInVector(mEndCallbackIds[id], NextCallbackId))
			mEndCallbackIds[id].push_back(NextCallbackId);

		return NextCallbackId++;
	}

	void UnregisterEndCallbackForAudio(AudioId id){
		auto idIt = mEndCallbackIds.Find(id);
		if (idIt != mEndCallbackIds.end()){
			for (auto& funcId : idIt->second){
				auto funcIt = mEndCallbacks.Find(funcId);
				if (funcIt != mEndCallbacks.end()){
					mEndCallbacks.erase(funcIt);
				}
			}
			mEndCallbackIds.erase(idIt);
		}		
	}

	void UnregisterEndCallbackFunc(FunctionId functionId){
		// functionId, function
		auto funcIt = mEndCallbacks.Find(functionId);
		if (funcIt != mEndCallbacks.end()){
			mEndCallbacks.erase(funcIt);
		}
		// audioId, functionIds
		for (auto it = mEndCallbackIds.begin(); it != mEndCallbackIds.end(); /**/){
			for (auto funcIt = it->second.begin(); funcIt != it->second.end(); /**/){
				if (*funcIt == functionId){
					funcIt = it->second.erase(funcIt);
				}
				else{
					++funcIt;
				}
			}
			if (it->second.empty()){
				it = mEndCallbackIds.erase(it);
			}
			else{
				++it;
			}
		}
	}

	bool SetPosition(AudioId id, float x, float y, float z){
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			it->second->SetPosition(x, y, z);
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
			it->second->SetRelative(relative);			
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
			it->second->SetReferenceDistance(distance);
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
			it->second->SetRollOffFactor(factor);
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
			it->second->SetOffsetInSec(sec);			
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
			it->second->SetGain(gain);
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
			return it->second->GetGain();
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

		// AudioId, FunctionIds
		auto callbackIt = mEndCallbackIds.Find(id);
		if (callbackIt != mEndCallbackIds.end()){
			for (auto funcId : callbackIt->second){
				// functionId, Function
				auto funcIt = mEndCallbacks.Find(funcId);
				if (funcIt != mEndCallbacks.end()){
					funcIt->second(id);
					mEndCallbacks.erase(funcIt);
				}
			}
			mEndCallbackIds.erase(callbackIt);
		}

		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			mALSources.push(it->second->GetALAudioSource());			
			mAudioSources.erase(it);
		}
		else{
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Audio(%u) is not found.", id).c_str());
		}		

		for (auto it = mAudioManipulators.begin(); it != mAudioManipulators.end(); /**/){
			if ((*it)->GetAudioId() == id){
				it = mAudioManipulators.erase(it);
			}
			else{
				++it;
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
	AudioSourcePtr dummy;
	return mImpl->PlayAudio(path, &dummy);
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

FunctionId AudioManager::RegisterEndCallback(AudioId id, std::function< void(AudioId) > callback){
	return mImpl->RegisterEndCallback(id, callback);
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

void AudioManager::UnregisterEndCallbackForAudio(AudioId id){
	mImpl->UnregisterEndCallbackForAudio(id);
}

void AudioManager::UnregisterEndCallbackFunc(FunctionId funcId){
	mImpl->UnregisterEndCallbackFunc(funcId);
}