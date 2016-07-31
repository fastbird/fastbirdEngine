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
#include "SmoothGain.h"
#include "AudioEx.h"
#include "AudioSourceStatus.h"
#include "AudioBuffer.h"
#include "AudioSource.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBCommonHeaders/ProfilerSimple.h"
using namespace fb;

AudioId NextAudioId = 1;
AudioManager* sAudioManagerRaw = 0;
std::atomic<bool> sDeinitialized = false;
namespace fb{
	static void eos_callback(void *userData, ALuint source);
	void CheckALError(){
		auto error = alGetError();
		if (error){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("OpenAL error code : 0x%x", error).c_str());
		}
	}

	class AudioThread : public Thread{
		// Interface
		bool Init() {
			return true;
		}

		// Returns 'repeat?' flag.
		bool Run(){
			return sAudioManagerRaw->AudioThreadFunc();
		}

		void Exit() {

		}
	};
}

FunctionId NextCallbackId = 1;
//---------------------------------------------------------------------------
class AudioManager::Impl{
public:
	static const int MaximumAudioSources = 24;
	static const int NumReservedAudioSources = 8;
	static const unsigned NumSortPerFrame = 10;
	ALCdevice* mDevice;
	ALCcontext* mContext;
	static LPALGETSOURCEDVSOFT alGetSourcedvSOFT;
	VectorMap<std::string, AudioBufferPtr> mAudioBuffers;	
	// source plyaing audio id
	std::stack<ALuint> mALSources;	
	// audioId - source index
	AudioSources mAudioSources;
	std::multimap<float, AudioId> mAudioSourcesSorted;
	// Source, Buffer	
	typedef VectorMap<AudioId, std::vector<IAudioManipulatorPtr> > AudioManipulators;
	AudioManipulators mAudioManipulators;
	AudioManipulators mAudioManipulatorsQueue;
	std::vector<AudioExPtr> mAudioExs;	
	std::vector<AudioExPtr> mAudioExsQueue;

	typedef VectorMap<FunctionId, CallbackFunction> CallbackMap;
	typedef VectorMap<AudioId, std::vector<FunctionId> > CallbackIdMap;
	CallbackMap mEndCallbacks;
	CallbackIdMap mEndCallbackIds;
	Vec3Tuple mListenerPos;	
	Vec3Tuple mListenerPosMainThread;
	int mNumPlaying;
	int mNumGeneratedSources;
	float mMasterGain;
	bool mSorted;
	bool mEnabled;

	struct PlayData{
		AudioSourcePtr mSource;
		std::string mFilePath;		
	};
	typedef VectorMap<AudioId, PlayData> PlayDataVector;
	PlayDataVector mAudioPlayQueue;
	mutable FB_CRITICAL_SECTION mAudioMutex;
	AudioThread mAudioThread;
	struct SettingData{
		AudioId mAudioId;
		int mPropertyType;
		union{
			struct{
				float x, y, z;
			} mVec3;
			int mInt;
		};
	};
	typedef std::vector<SettingData> SettingDataVector;
	SettingDataVector mSettingDataQueue;
	std::unordered_set<AudioId> mInvalidatedAudioIds;

	struct StopData{
		AudioId mAudioId;
		TIME_PRECISION mSec;
	};
	typedef std::vector<StopData> StopQueue;
	StopQueue mStopQueue;

	//---------------------------------------------------------------------------
	Impl()
		: mDevice(0)
		, mContext(0)
		, mNumPlaying(0)
		, mNumGeneratedSources(0)
		, mSorted(false)
		, mMasterGain(1.0f)
		, mEnabled(true)
	{
		mListenerPos = std::make_tuple(0.f, 0.f, 0.f);
		mListenerPosMainThread = mListenerPos;
	}

	~Impl(){
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("(info) num alsources = %d, num generatedSources = %d", 
			mALSources.size(), mNumGeneratedSources).c_str());
		assert(mALSources.size() == mNumGeneratedSources);
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
	void JoinThread(){
		mAudioThread.Join();
	}
	bool IsAudioThread(){
		return std::this_thread::get_id() == mAudioThread.mThreadDesc->mThreadID;
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
				++mNumGeneratedSources;
				mALSources.push(src);
			}
		}
		Logger::Log(FB_DEFAULT_LOG_ARG, "OpenAL initialized!");
		mAudioThread.CreateThread(1024, "AudioThread");
		return true;
	}

	void Deinit(){
		mAudioExs.clear();
		std::vector<AudioId> audioIds;
		{
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			audioIds.reserve(mAudioSources.size());
			for (auto& it : mAudioSources) {
				audioIds.push_back(it.first);
			}
		}
		for (auto audioId : audioIds){
			if (audioId != -1){
				StopAudio(audioId);
			}
		}
	}
	
	Real GetDistanceSQ(const Vec3Tuple& a, const Vec3Tuple& b){
		float xgap = std::get<0>(a) - std::get<0>(b);
		float ygap = std::get<1>(a) - std::get<1>(b);
		float zgap = std::get<2>(a) - std::get<2>(b);
		return xgap * xgap + ygap * ygap + zgap * zgap;
	}

	float GetLengthSQ(const Vec3Tuple& a){
		float x = std::get<0>(a);
		float y = std::get<0>(a);
		float z = std::get<0>(a);
		return x * x + y * y + z * z;
	}

	// AudioThreadFunc
	void DropAudio(){		
		int desiredDrop = mNumPlaying - MaximumAudioSources;
		if (desiredDrop <= 0)
			return;
		auto it = mAudioSourcesSorted.begin();
		std::advance(it, MaximumAudioSources-1);
		for (; it != mAudioSourcesSorted.end();++it){
			auto audioIt = mAudioSources.Find(it->second);
			if (audioIt == mAudioSources.end()){
				++it;
				continue;
			}

			auto& audio = audioIt->second;
			if (audio->GetStatus() == AudioSourceStatus::Playing){
				audio->SetStatus(AudioSourceStatus::Dropping);
				StopWithFadeOut(it->second, 0.1f);											
			}			
		}		
	}

	// AudioThread
	bool PlayWaitingAudio(AudioSourcePtr audioSource, ALuint src, TIME_PRECISION fadeInTime){
		assert(audioSource->GetLeftTime() > 0.f);
		assert(audioSource->GetStatus() == AudioSourceStatus::Waiting);
		assert(src != -1);
		alSourcei(src, AL_BUFFER, audioSource->GetAudioBuffer()->mBuffer);
		CheckALError();
		
		bool error = false;
		if (mEnabled){
			error = alurePlaySource(src, eos_callback, (void*)audioSource->GetAudioId()) == AL_FALSE;
		}
		if (error){
			CheckALError();
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot play AudioSource(%u, %s)",
				audioSource->GetAudioId(), audioSource->GetAudioBuffer()->mFilepath.c_str()).c_str());
			audioSource->SetStatus(AudioSourceStatus::Waiting);
			return false;
		}
		else{
			CheckALError();
			audioSource->SetALAudioSource(src);
			audioSource->SetStatus(AudioSourceStatus::Playing);
			++mNumPlaying;
			audioSource->ApplyProp();
			audioSource->ApplyRemainedTime();
			auto audioId = audioSource->GetAudioId();
			auto fadeIn = SmoothGain::Create(audioId,
				1.0f / std::min(fadeInTime, audioSource->GetLeftTime()*.2f), 0.f,
				audioSource->GetGain());
			mAudioManipulators[audioId].push_back(fadeIn);
			return true;
		}
		
	}

	// AudioThread
	void CheckReserved(){		
		if (mALSources.empty() || mNumPlaying >= MaximumAudioSources)
			return;
		if (mAudioSources.size() < MaximumAudioSources)
			return;
		VectorMap<float, AudioId> sorted;
		for (auto& it : mAudioSources){
			if (it.second->GetStatus() == AudioSourceStatus::Waiting){
				auto dist = it.second->GetRelative() ?
					GetLengthSQ(it.second->GetPosition()) :
					GetDistanceSQ(mListenerPos, it.second->GetPosition());
				sorted[dist / it.second->GetReferenceDistance()] = it.second->GetAudioId();
			}
		}
		for (auto& itSorted : sorted){
			auto itSource = mAudioSources.Find(itSorted.second);
			if (itSource != mAudioSources.end()){
				auto alsrc = mALSources.top();
				if (PlayWaitingAudio(itSource->second, alsrc, 0.5f)){
					mALSources.pop();
				}
			}
			if (mALSources.empty())
				break;
		}
	}

	void Update(TIME_PRECISION dt){		
	}
	
	// AudioThreadFunc
	bool PlayAudioBuffer(AudioBufferPtr buffer, AudioSourcePtr audioSource){		
		if (!buffer || buffer->mBuffer == -1){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
			return false;
		}		
		if (mNumPlaying >= 256){
			return false;
		}
		auto audioId = audioSource->GetAudioId();
		assert(audioId != INVALID_AUDIO_ID);
		audioSource->SetAudioBuffer(buffer);
		{
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			mAudioSources[audioId] = audioSource;
		}
		
		ALuint alsource = -1;
		if (!mALSources.empty()){
			alsource = mALSources.top();
			mALSources.pop();
		}
		else{			
			float audioDistance = audioSource->GetRelative() ?
				GetLengthSQ(audioSource->GetPosition()) : GetDistanceSQ(mListenerPos, audioSource->GetPosition());
			audioDistance /= audioSource->GetReferenceDistance();				
			mAudioSourcesSorted.insert(std::make_pair(audioDistance, audioId));
			auto it = mAudioSourcesSorted.begin();
			std::advance(it, MaximumAudioSources-1);			
			if (audioDistance >= it->first) {
				audioSource->SetStatus(AudioSourceStatus::Waiting);
				return false;
			}			
			DropAudio();
			
			alGenSources(1, &alsource);
			if (alGetError() != AL_NO_ERROR){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot create audio source. Current playing: %d", mNumPlaying).c_str());
				alsource = -1;
			}
			else{
				++mNumGeneratedSources;
			}
		}
		if (alsource != -1){
			alSourcei(alsource, AL_BUFFER, buffer->mBuffer);
			CheckALError();
			bool error = false;
			if (mEnabled){
				error = alurePlaySource(alsource, eos_callback, (void*)audioId) == AL_FALSE;
			}
			if (error){
				CheckALError();
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot play AudioSource(%u, %s)", 
					audioId, buffer->mFilepath.c_str()).c_str());
				audioSource->SetStatus(AudioSourceStatus::Waiting);
				audioSource->SetALAudioSource(-1);
				mALSources.push(alsource);
				return false;
			}
			else{
				CheckALError();
				audioSource->SetStatus(AudioSourceStatus::Playing);
				++mNumPlaying;
				audioSource->SetALAudioSource(alsource);				
				audioSource->ApplyProp();
				return true;
			}
		}
		return false;
	}
	
	AudioBufferPtr GetAudioBuffer(std::string path, std::string loweredPath){
		if (path.size() != loweredPath.size()){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		using namespace std::chrono;
		auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();		
		std::pair<AudioId, ALuint> audioIdSrcId;
		mAudioMutex.Lock();
		auto it = mAudioBuffers.Find(loweredPath);
		if (it != mAudioBuffers.end()){
			it->second->mLastAccessed = curTick;
			mAudioMutex.Unlock();
			return it->second;
		}
		else{
			mAudioMutex.Unlock();
			ALuint buffer = 0;
			try {
				buffer = alureCreateBufferFromFile(path.c_str());
			}
			catch (...) {
				Logger::Log(FB_ERROR_LOG_ARG, "Exception occurred.");
				return 0;
			}
			CheckALError();
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
			mAudioMutex.Lock();
			mAudioBuffers[loweredPath] = audioBuffer;
			mAudioMutex.Unlock();
			return audioBuffer;
		}
	}

	struct FBAudio{
		std::string mFilepath;
		AudioProperty mProperty;
	};

	std::string ParseFBAudioForAudio(const char* path){
		tinyxml2::XMLDocument doc;
		auto err = doc.LoadFile(path);
		if (err){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot parse fbaudio(%s)", path).c_str());
			return std::string();
		}

		auto root = doc.RootElement();
		if (!root){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid format(%s)", path);
			return std::string();
		}

		auto filepath = root->Attribute("file");
		if (!filepath){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("'file' attribute is not found in (%s)", path).c_str());
			return std::string();
		}
		else{
			return std::string(filepath);
		}
	}

	FBAudio ParseFBAudio(const char* path){
		FBAudio ret;
		tinyxml2::XMLDocument doc;
		auto err = doc.LoadFile(path);
		if (err){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot parse fbaudio(%s)", path).c_str());
			return ret;
		}

		auto root = doc.RootElement();
		if (!root){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid format(%s)", path);
			return ret;
		}

		auto filepath = root->Attribute("file");
		if (!filepath){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("'file' attribute is not found in (%s)", path).c_str());			
		}
		else{
			ret.mFilepath = filepath;
		}

		if (!FileSystem::Exists(ret.mFilepath.c_str())){
			std::string xmlpath = path;
			auto onlyFilename = FileSystem::GetFileName(ret.mFilepath.c_str());
			ret.mFilepath = FileSystem::ReplaceFilename(xmlpath.c_str(), onlyFilename.c_str());
			if (!FileSystem::Exists(ret.mFilepath.c_str())){
				ret.mFilepath.clear();
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Cannot find a audio file(%s or %s)", filepath, ret.mFilepath.c_str()).c_str());
			}			
		}

		auto sz = root->Attribute("gain");
		if (sz){
			ret.mProperty.mGain = StringConverter::ParseReal(sz, ret.mProperty.mGain);
		}

		sz = root->Attribute("relative");
		if (sz){
			ret.mProperty.mRelative = StringConverter::ParseBool(sz, ret.mProperty.mRelative);
		}

		sz = root->Attribute("rollOffFactor");
		if (sz){
			ret.mProperty.mRolloffFactor = StringConverter::ParseReal(sz, ret.mProperty.mRolloffFactor);
		}

		sz = root->Attribute("referenceDistance");
		if (sz){
			ret.mProperty.mReferenceDistance = StringConverter::ParseReal(sz, ret.mProperty.mReferenceDistance);
		}

		sz = root->Attribute("numberOfSimultaneous");
		if (sz) {
			ret.mProperty.mNumSimultaneous = StringConverter::ParseUnsignedInt(sz,
				ret.mProperty.mNumSimultaneous);
		}

		sz = root->Attribute("simultaneousCheckRange");
		if (sz) {
			ret.mProperty.mSimultaneousCheckRange = StringConverter::ParseReal(sz,
				ret.mProperty.mSimultaneousCheckRange);
		}

		sz = root->Attribute("loop");
		if (sz){
			ret.mProperty.mLoop = StringConverter::ParseBool(sz, ret.mProperty.mLoop);
		}

		return ret;
	}

	// fb audio
	AudioId PlayFBAudio(const char* path){
		auto parsedAudio = ParseFBAudio(path);
		if (parsedAudio.mFilepath.empty())
			return INVALID_AUDIO_ID;
		return PlayAudio(parsedAudio.mFilepath.c_str(), parsedAudio.mProperty);
	}
	AudioId PlayFBAudio(const char* path, const Vec3Tuple& pos){
		auto parsedAudio = ParseFBAudio(path);
		if (parsedAudio.mFilepath.empty())
			return INVALID_AUDIO_ID;
		parsedAudio.mProperty.mRelative = false;
		parsedAudio.mProperty.mPosition = pos;
		return PlayAudio(parsedAudio.mFilepath.c_str(), parsedAudio.mProperty);
	}

	// normal audio
	AudioId PlayAudio(const char* path){
		if (!ValidCString(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return INVALID_AUDIO_ID;
		}

		if (_stricmp(FileSystem::GetExtension(path), ".fbaudio") == 0)
			return PlayFBAudio(path);
		AudioProperty prop;
		prop.mRelative = true;
		return PlayAudio(path, prop);
	}

	AudioId PlayAudio(const char* path, const AudioProperty& property){
		if (!ValidCString(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return INVALID_AUDIO_ID;
		}
		
		if (_stricmp(FileSystem::GetExtension(path), ".fbaudio") == 0){
			Logger::Log(FB_ERROR_LOG_ARG, "Rejected. Use PlayFBAudio() for .fbaudio");
			return INVALID_AUDIO_ID;
		}
		
		
		ENTER_CRITICAL_SECTION l(mAudioMutex);

		AudioId audioId = NextAudioId++;
		auto audioSource = AudioSource::Create(audioId);
		audioSource->SetProperty(property);
		mAudioPlayQueue.Insert(std::make_pair(audioId, PlayData{ audioSource, path }));
		return audioId;
	}

	AudioId PlayAudio(const char* path, const Vec3Tuple& pos){
		if (_stricmp(FileSystem::GetExtension(path), ".fbaudio") == 0){
			return PlayFBAudio(path, pos);
		}
		AudioProperty prop;
		prop.mRelative = false;
		prop.mPosition = pos;
		return PlayAudio(path, prop);
	}	

	AudioId PlayAudioWithFadeIn(const char* path, const AudioProperty& prop, TIME_PRECISION inSec){
		auto id = PlayAudio(path, prop);
		if (id != INVALID_AUDIO_ID){			
			auto fadeIn = SmoothGain::Create(id, 1.0f / inSec, 0.f, prop.mGain);
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			mAudioManipulatorsQueue[id].push_back(fadeIn);			
		}
		return id;
	}

	bool StopAudioInAudioThread(AudioId id){
		assert(IsAudioThread());
		auto itSource = mAudioSources.Find(id);
		alureStopSource(itSource->second->GetALAudioSource(), AL_TRUE);
		CheckALError();
		return true;
	}

	bool StopAudio(AudioId id){
		if (mInvalidatedAudioIds.find(id) != mInvalidatedAudioIds.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Stop invalid audio(%u)", id).c_str());
			return false;
		}

		ENTER_CRITICAL_SECTION l(mAudioMutex);		
		mStopQueue.push_back(StopData{ id, -1.0f });
		return true;
	}

	bool StopWithFadeOut(AudioId id, TIME_PRECISION sec){
		if (mInvalidatedAudioIds.find(id) != mInvalidatedAudioIds.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Stop invalid audio(%u)", id).c_str());
			return false;
		}

		ENTER_CRITICAL_SECTION l(mAudioMutex);		
		mStopQueue.push_back(StopData{ id, sec });
		return true;		
	}

	TIME_PRECISION GetAudioLeftTime(AudioId id){
		if (mInvalidatedAudioIds.find(id) != mInvalidatedAudioIds.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot get left time for invalid audio(%u)", id).c_str());
			return 0.f;
		}

		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto itQueue = mAudioPlayQueue.Find(id);
		if (itQueue != mAudioPlayQueue.end()){
			return GetAudioLength(itQueue->second.mFilePath.c_str());
		}
		else{			
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				return it->second->GetLeftTime();
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot get left time for invalid audio(%u)", id).c_str());
				return 0;
			}
		}
	}

	TIME_PRECISION GetAudioLength(const char* path){		
		std::string audioPath;
		if (_stricmp(FileSystem::GetExtension(path), ".fbaudio") == 0){
			audioPath = ParseFBAudioForAudio(path);
		}
		else{
			audioPath = path;
		}
		std::string lowered(audioPath);
		ToLowerCase(lowered);

		auto buffer = GetAudioBuffer(audioPath, lowered);
		if (!buffer){
			return 0.f;
		}
		return buffer->mLength;
	}

	TIME_PRECISION GetAudioLength(AudioId id) {		
		if (mInvalidatedAudioIds.find(id) != mInvalidatedAudioIds.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Canno get a length for invalid audio(%u)", id).c_str());
			return 0.f;
		}
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto itQueue = mAudioPlayQueue.Find(id);
		if (itQueue != mAudioPlayQueue.end()){
			return GetAudioLength(itQueue->second.mFilePath.c_str());
		}
		else{			
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				return it->second->GetLength();
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Canno get a length for invalid audio(%u)", id).c_str());
				return 0;
			}
		}
	}

	FunctionId RegisterEndCallback(AudioId id, std::function< void(AudioId) > callback){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		mEndCallbacks[NextCallbackId] = callback;
		if (!ValueExistsInVector(mEndCallbackIds[id], NextCallbackId))
			mEndCallbackIds[id].push_back(NextCallbackId);

		return NextCallbackId++;
	}

	void UnregisterEndCallbackForAudio(AudioId id){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
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
		ENTER_CRITICAL_SECTION l(mAudioMutex);
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

	// AudioThread
	IAudioManipulatorPtr GetManipulator(AudioId id, AudioManipulatorType::Enum type) {
		assert(IsAudioThread());
		auto it = mAudioManipulators.Find(id);
		if (it != mAudioManipulators.end()){
			auto& manipulators = it->second;
			for (auto itM = manipulators.begin(); itM != manipulators.end(); ++itM){
				if ((*itM)->GetManipulatorType() == type){
					return *itM;
				}
			}
		}
		return 0;
	}

	// audio thread
	void DeleteManipulator(AudioId id, AudioManipulatorType::Enum type){
		assert(IsAudioThread());
		auto sourceIt = mAudioSources.Find(id);
		if (sourceIt != mAudioSources.end() && sourceIt->second->GetStatus() == AudioSourceStatus::Dropping){
			return;
		}
		auto it = mAudioManipulators.Find(id);
		if (it != mAudioManipulators.end()){
			auto& manipulators = it->second;
			for (auto itM = manipulators.begin(); itM != manipulators.end(); /**/){
				if ((*itM)->GetManipulatorType() == type){
					itM = manipulators.erase(itM);
				}
				else{
					++itM;
				}
			}
			if (manipulators.empty()){
				mAudioManipulators.erase(it);
			}
		}
	}

	void GetAudioList(std::vector<AudioDebugData>& list) const{
		AudioSources sources;
		SettingDataVector positionData;
		{
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			sources = mAudioSources;
			for (auto& settingData : mSettingDataQueue){
				if (settingData.mPropertyType == AL_POSITION){
					auto itFound = std::find_if(positionData.begin(), positionData.end(), [&](const SettingData& a){
						return a.mAudioId == settingData.mAudioId && a.mPropertyType == settingData.mPropertyType;
					});
					if (itFound != positionData.end()){
						itFound->mVec3 = settingData.mVec3;
					}
					else{
						positionData.push_back(settingData);
					}
				}
			}
		}
		for (auto& it : sources){
			if (it.second){
				list.push_back(AudioDebugData());
				
				AudioDebugData& data = list.back();
				data.mDistPerRef = it.second->GetDistPerRef();
				data.mFilePath = it.second->GetAudioBuffer()->mFilepath;
				data.mGain = it.second->GetGain();
				data.mPosition = it.second->GetPosition();
				auto audioId = it.second->GetAudioId();
				for (auto& itPos : positionData){
					if (itPos.mAudioId == audioId){
						data.mPosition = std::make_tuple( itPos.mVec3.x, itPos.mVec3.y, itPos.mVec3.z );
						break;
					}
				}
				data.mStatus = it.second->GetStatus();
			}
		}
	}

	bool IsValidSource(AudioId id) const{
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end())
			return true;		
		auto sourceIt = mAudioSources.Find(id);
		return sourceIt != mAudioSources.end();
	}

	void CheckSimultaneous(const std::string& path, const AudioSourcePtr& audioSource) {
		if (audioSource->GetNumSimultaneous() == 0)
			return;

		auto numSimul = audioSource->GetNumSimultaneous();
		std::vector<AudioSourcePtr> audioSources;		
		if (audioSource->GetRelative()) {			
			for (auto& it : mAudioSources) {
				if (it.second->GetAudioBuffer()->mFilepath == path) {
					audioSources.push_back(it.second);
				}
			}
			if (audioSources.size() < numSimul)
				return;
			std::sort(audioSources.begin(), audioSources.end(), [](const AudioSourcePtr& a, const AudioSourcePtr& b) {
				return a->GetLeftTime() > b->GetLeftTime();
			});
			for (size_t i = numSimul - 1; i < audioSources.size(); ++i) {
				StopAudioInAudioThread(audioSources[i]->GetAudioId());
			}
		}
		else {
			auto rangeSQ = audioSource->GetSimultaneousCheckRange();
			rangeSQ *= rangeSQ;
			VectorMap<float, AudioSourcePtr> audioSources;
			float preventSame = 0.001f;
			for (auto& it : mAudioSources) {
				if (it.second->GetAudioBuffer()->mFilepath == path) {
					auto distSQ = GetDistanceSQBetween(it.second->GetPosition(), audioSource->GetPosition());
					if (distSQ <= rangeSQ) {
						audioSources.Insert(std::make_pair(distSQ + preventSame, it.second));
						preventSame += 0.001f;
					}
				}
			}
			if (audioSources.size() < numSimul)
				return;
			for (size_t i = numSimul - 1; i < audioSources.size(); ++i) {
				StopAudioInAudioThread((audioSources.begin() + i)->second->GetAudioId());
			}			
		}
	}

	bool AudioThreadFunc(float dt){
		PlayDataVector playQueue;		
		SettingDataVector setttingDataQueue;
		setttingDataQueue.reserve(100);
		AudioManipulators manipulators;		
		StopQueue stopQueue;		
		std::vector<AudioExPtr> audioExQueue;
		{
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			playQueue.swap(mAudioPlayQueue);			
			mListenerPos = mListenerPosMainThread;
			setttingDataQueue.swap(mSettingDataQueue);			
			manipulators.swap(mAudioManipulatorsQueue);
			stopQueue.swap(mStopQueue);
			audioExQueue.swap(mAudioExsQueue);
		}
		mAudioExs.insert(mAudioExs.end(), audioExQueue.begin(), audioExQueue.end());

		alListener3f(AL_POSITION, std::get<0>(mListenerPos), 
			std::get<1>(mListenerPos), std::get<2>(mListenerPos));

		// execute play
		{
			for (auto& it : playQueue) {
				std::string pathKey(it.second.mFilePath);
				ToLowerCase(pathKey);
				auto audioBuffer = GetAudioBuffer(it.second.mFilePath, pathKey);
				if (!audioBuffer) {
					continue;
				}
				CheckSimultaneous(it.second.mFilePath, it.second.mSource);
				PlayAudioBuffer(audioBuffer, it.second.mSource);
			}
		}

		
		// execute settings
		for (auto& it : setttingDataQueue){
			auto itSource = mAudioSources.Find(it.mAudioId);
			if (itSource != mAudioSources.end()){
				switch (it.mPropertyType){
				case AL_POSITION:
				{
					itSource->second->SetPosition(it.mVec3.x, it.mVec3.y, it.mVec3.z);
					break;
				}
				case AL_ROLLOFF_FACTOR:
				{
					itSource->second->SetRollOffFactor(it.mVec3.x);
					break;
				}
				case AL_REFERENCE_DISTANCE:
				{
					itSource->second->SetReferenceDistance(it.mVec3.x);
					break;
				}
				case AL_SOURCE_RELATIVE:
				{
					itSource->second->SetRelative(it.mInt!=0);
					break;
				}
				case AL_SEC_OFFSET:
				{
					itSource->second->SetOffsetInSec(it.mVec3.x);
					break;
				}
				case AL_GAIN:
				{
					float gain = it.mVec3.x;
					bool smooth = it.mVec3.z != 0.0f;					
					if (smooth){
						float inSec = it.mVec3.y;
						SetGainSmoothAudioThread(it.mAudioId, gain, inSec);
					}
					else{
						bool checkManipulator = it.mVec3.y != 0.f;						
						SetGainAudioThread(it.mAudioId, gain, checkManipulator);						
					}
					break;
				}

				case AL_LOOPING:
				{
					itSource->second->SetLoop(it.mInt!=0);
					break;
				}

				case AL_MAX_GAIN:
				{
					itSource->second->SetMaxGain(it.mVec3.x);
					break;
				}

				default:
				{
					Logger::Log(FB_ERROR_LOG_ARG, "Property is not processed.");
					assert(0);
				}
				}
			}
			else{
				//Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting property(%d) for invalid audio(%u)", it.mPropertyType, it.mAudioId).c_str());
			}
		}

		for (auto& it : manipulators){
			mAudioManipulators[it.first].insert(mAudioManipulators[it.first].end(),
				it.second.begin(), it.second.end());
		}

		for (auto& it : stopQueue){
			auto itSource = mAudioSources.Find(it.mAudioId);
			if (itSource != mAudioSources.end()){
				if (itSource->second->GetALAudioSource() != -1){
					if (it.mSec > 0.f){
						auto leftTime = itSource->second->GetLeftTime();
						leftTime = std::min(std::max(leftTime, 0.f), it.mSec);
						auto fadeOutE = GetManipulator(it.mAudioId, AudioManipulatorType::SmoothGain);
						if (fadeOutE){
							fadeOutE->OnGainModified(0.f);
							fadeOutE->SetDuration(leftTime);
						}
						else{							
							float curGain = itSource->second->GetGain();
							auto fadeOut = SmoothGain::Create(it.mAudioId, curGain / std::min(leftTime, it.mSec),
								curGain, 0.f);
							ENTER_CRITICAL_SECTION l(mAudioMutex);
							mAudioManipulators[it.mAudioId].push_back(fadeOut);
						}
					}
					else{
						alureStopSource(itSource->second->GetALAudioSource(), AL_TRUE);
						CheckALError();
					}
				}
				else{
					OnPlayFinishedInternal(it.mAudioId);
					ENTER_CRITICAL_SECTION l(mAudioMutex);
					mAudioSources.erase(itSource);					
				}
			}
			else{
				//Logger::Log(FB_ERROR_LOG_ARG, FormatString("Stop invalid audio(%u)", it.mAudioId).c_str());
			}
		}

		// Update
		for (auto itMan = mAudioManipulators.begin(); itMan != mAudioManipulators.end(); /**/){
			for (auto it = itMan->second.begin(); it != itMan->second.end(); /**/)
			{
				bool finished = (*it)->Update(dt);
				if (finished){
					auto sourceIt = mAudioSources.Find((*it)->GetAudioId());
					if (sourceIt != mAudioSources.end()){
						if (sourceIt->second->GetGain() == 0.f){
							StopAudioInAudioThread(sourceIt->second->GetAudioId());
						}
					}
					it = itMan->second.erase(it);
				}
				else{
					++it;
				}
			}
			if (itMan->second.empty()){
				itMan = mAudioManipulators.erase(itMan);
			}
			else{
				++itMan;
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
		for (auto it = mAudioSources.begin(); it != mAudioSources.end(); /**/){
			if (it->second->Update(dt)){
				OnPlayFinishedInternal(it->second->GetAudioId());
				ENTER_CRITICAL_SECTION l(mAudioMutex);
				it = mAudioSources.erase(it);
			}
			else{
				++it;
			}
		}
		mAudioSourcesSorted.clear();
		if (mAudioSources.empty())
			return true;

		static TIME_PRECISION accumulator = 0;
		accumulator += dt;
		if (accumulator >= 0.125f){
			accumulator -= 0.125f;
			alureUpdate();
			CheckALError();
			CheckReserved();
		}


		for (auto& it : mAudioSources){
			auto dist = it.second->GetRelative() ?
				GetLengthSQ(it.second->GetPosition()) :
				GetDistanceSQ(mListenerPos, it.second->GetPosition());
			dist /= it.second->GetReferenceDistance();
			mAudioSourcesSorted.insert(std::make_pair(dist, it.second->GetAudioId()));
			it.second->SetDistPerRef(dist);
		}

		static TIME_PRECISION accumulatorForBuffer = 0;
		accumulatorForBuffer += dt;
		if (accumulatorForBuffer >= 60.f){
			accumulatorForBuffer -= 60.f;
			using namespace std::chrono;
			auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			for (auto it = mAudioBuffers.begin(); it != mAudioBuffers.end(); /**/){
				if (it->second->mReferences == 0 && curTick - it->second->mLastAccessed > 60000){
					it = mAudioBuffers.erase(it);
				}
				else{
					++it;
				}
			}
		}

		if (sDeinitialized)
			return false;

		return true;
	}

	bool SetPosition(AudioId id, float x, float y, float z){
		if (IsAudioThread()){
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				it->second->SetPosition(x, y, z);
				return true;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "No source found.");
			return false;
		}

		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);			
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetPosition(x, y, z);			
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){
				for (auto& itHas : mSettingDataQueue){
					if (itHas.mAudioId == id && itHas.mPropertyType == AL_POSITION){
						itHas.mVec3 = { x, y, z };
						return true;
					}
				}

				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_POSITION;
				data.mVec3 = { x, y, z };
				return true;
			}
			else{
				//Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting position for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	bool SetRelative(AudioId id, bool relative){
		if (IsAudioThread()){
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				it->second->SetRelative(relative);
				return true;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "No source found.");
			return false;
		}
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetRelative(relative);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){				
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_SOURCE_RELATIVE;
				data.mInt = relative ? 1 : 0;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting the relative flag for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	void SetListenerPosition(const Vec3Tuple& pos){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		mListenerPosMainThread = pos;
	}

	bool SetReferenceDistance(AudioId id, float distance){
		if (IsAudioThread()){
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				it->second->SetReferenceDistance(distance);
				return true;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "No source found.");
			return false;
		}
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetReferenceDistance(distance);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){				
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_REFERENCE_DISTANCE;
				data.mVec3.x = distance;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting reference distance for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	bool SetRolloffFactor(AudioId id, float factor){
		if (IsAudioThread()){
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				it->second->SetRollOffFactor(factor);
				return true;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "No source found.");
			return false;
		}
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetRollOffFactor(factor);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){				
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_ROLLOFF_FACTOR;
				data.mVec3.x = factor;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting roll off factor for invalid audio(%u)", id).c_str());
				return false;
			}
		}		
	}

	bool SetOffsetInSec(AudioId id, float sec){
		if (IsAudioThread()){
			auto it = mAudioSources.Find(id);
			if (it != mAudioSources.end()){
				it->second->SetOffsetInSec(sec);
				return true;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "No source found.");
			return false;
		}
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetOffsetInSec(sec);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){				
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_SEC_OFFSET;
				data.mVec3.x = sec;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting offset for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	bool SetGainAudioThread(AudioId id, float gain, bool checkManipulator){
		assert(IsAudioThread());
		auto itSource = mAudioSources.Find(id);
		if (itSource != mAudioSources.end()){
			bool foundManipulator = false;
			if (checkManipulator){
				if (itSource->second->GetStatus() == AudioSourceStatus::Dropping){
					Logger::Log(FB_ERROR_LOG_ARG, "gain dropping audio");
				}
				auto itM = mAudioManipulators.Find(id);
				if (itM != mAudioManipulators.end()){
					auto& manipulators = itM->second;
					for (auto& it : manipulators){
						it->OnGainModified(gain);
						if (it->GetManipulatorType() == AudioManipulatorType::SmoothGain)
							foundManipulator = true;
					}
				}
			}
			if (!foundManipulator){
				itSource->second->SetGain(gain);
			}
			return true;
		}
		else{			
			return false;
		}
	}

	bool SetGainSmoothAudioThread(AudioId id, float gain, float inSec){
		assert(IsAudioThread());
		auto itSource = mAudioSources.Find(id);
		if (itSource != mAudioSources.end()){
			if (itSource->second->GetStatus() != AudioSourceStatus::Dropping){
				bool foundManipulator = false;
				auto itM = mAudioManipulators.Find(id);
				if (itM != mAudioManipulators.end()){
					auto& manipulators = itM->second;
					for (auto& it : manipulators){
						it->OnGainModified(gain);
						if (it->GetManipulatorType() == AudioManipulatorType::SmoothGain){
							it->SetDuration(inSec);
							foundManipulator = true;
						}
					}
				}
				if (!foundManipulator){
					auto curGain = itSource->second->GetGain();					
					auto smoothGain = SmoothGain::Create(id,
						std::abs(curGain - gain) / inSec, curGain, gain);
					mAudioManipulators[id].push_back(smoothGain);
				}
				return true;
			}
		}
		return false;
	}

	bool SetGain(AudioId id, float gain, bool checkManipulator){
		if (IsAudioThread()){
			return SetGainAudioThread(id, gain, checkManipulator);
		}

		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetGain(gain);
			return true;
		}

		if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){			
			mSettingDataQueue.push_back(SettingData());
			auto& data = mSettingDataQueue.back();
			data.mAudioId = id;
			data.mPropertyType = AL_GAIN;
			data.mVec3.x = gain;
			data.mVec3.y = checkManipulator ? 1.0f : 0.0f;
			data.mVec3.z = 0.f; // no smooth: 0.f
			return true;
		}
		else{
			if (checkManipulator)
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting gain for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetGainSmooth(AudioId id, float gain, float inSec){
		if (IsAudioThread()){
			return SetGainSmoothAudioThread(id, gain, inSec);
		}
		if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			mSettingDataQueue.push_back(SettingData());
			auto& data = mSettingDataQueue.back();
			data.mAudioId = id;
			data.mPropertyType = AL_GAIN;
			data.mVec3.x = gain;
			data.mVec3.y = inSec;
			data.mVec3.z = 1.0f; // smooth: 1.f
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting gain smooth for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	float GetGain(AudioId id) const{
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			return it->second->GetGain();
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Getting gain for invalid audio(%u)", id).c_str());
			return 0.f;
		}
	}

	float GetGainFromFBAudio(const char* fbaudioPath){
		return ParseFBAudio(fbaudioPath).mProperty.mGain;
	}

	bool SetLoop(AudioId id, bool loop){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetLoop(loop);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){				
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_LOOPING;
				data.mInt = loop ? 1 : 0;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting loop for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	bool GetLoop(AudioId id) const{
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			return it->second->GetLoop();
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Getting loop for invalid audio(%u)", id).c_str());
			return false;
		}
	}

	bool SetMaxGain(AudioId id, float maxGain){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioPlayQueue.Find(id);
		if (it != mAudioPlayQueue.end()){
			it->second.mSource->SetMaxGain(maxGain);
			return true;
		}
		else{
			if (mInvalidatedAudioIds.find(id) == mInvalidatedAudioIds.end()){
				mSettingDataQueue.push_back(SettingData());
				auto& data = mSettingDataQueue.back();
				data.mAudioId = id;
				data.mPropertyType = AL_MAX_GAIN;
				data.mVec3.x = maxGain;
				return true;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Setting loop for invalid audio(%u)", id).c_str());
				return false;
			}
		}
	}

	float GetMaxGain(AudioId id){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			return it->second->GetMaxGain();
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Getting gain for invalid audio(%u)", id).c_str());
			return 0.f;
		}
	}

	void RegisterAudioEx(AudioExPtr audioex){
		ENTER_CRITICAL_SECTION l(mAudioMutex);
		if (!ValueExistsInVector(mAudioExsQueue, audioex)){
			mAudioExsQueue.push_back(audioex);
		}
	}

	bool IsRegisteredAudioEx(AudioExPtr audioex){
		bool exist = ValueExistsInVector(mAudioExsQueue, audioex);
		if (exist)
			return true;

		ENTER_CRITICAL_SECTION l(mAudioMutex);
		return ValueExistsInVector(mAudioExs, audioex);
	}

	void OnPlayFinishedInternal(AudioId id){
		assert(IsAudioThread());
		for (auto& it : mAudioExs){
			it->OnFinish(id);
		}

		// AudioId, FunctionIds
		ENTER_CRITICAL_SECTION l(mAudioMutex);
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
	}

	void OnPlayFinished(void* userdata, ALuint source){
		assert(IsAudioThread());		
		AudioId id = (AudioId)userdata;
		{
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			mInvalidatedAudioIds.insert(id);
		}
		OnPlayFinishedInternal(id);
		--mNumPlaying;
		auto it = mAudioSources.Find(id);
		if (it != mAudioSources.end()){
			assert(it->second->GetALAudioSource() != -1);
			mALSources.push(it->second->GetALAudioSource());			
			ENTER_CRITICAL_SECTION l(mAudioMutex);
			mAudioSources.erase(it);			
		}
		else{
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Audio(%u) is not found.", id).c_str());
		}		
	}

	void SetMasterGain(float gain){
		AudioSource::sMasterGain = gain;		
	}

	float GetMasterGain() const{
		return AudioSource::sMasterGain;
	}

	void SetEnabled(bool enabled){
		mEnabled = enabled;
		if (!mEnabled){
			std::vector<AudioId> audioIds;
			audioIds.reserve(mAudioSources.size());
			for (auto& it : mAudioSources){
				audioIds.push_back(it.first);
			}
			for (auto audioId : audioIds){
				if (audioId != -1){
					StopAudio(audioId);
				}
			}
		}
	}
};
LPALGETSOURCEDVSOFT AudioManager::Impl::alGetSourcedvSOFT = 0;

//---------------------------------------------------------------------------
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
	using namespace std::chrono_literals;
	Logger::Log(FB_ERROR_LOG_ARG, "Deleting Audio Manager.");	
	mImpl->Deinit();
	std::this_thread::sleep_for(200ms);
	sDeinitialized = true;
	sAudioManagerRaw = 0;
	mImpl->JoinThread();
	mImpl = 0;
	Logger::Log(FB_ERROR_LOG_ARG, "Audio Manager deleted.");
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

AudioId AudioManager::PlayFBAudio(const char* fbAudioPath){
	return mImpl->PlayFBAudio(fbAudioPath);
}

AudioId AudioManager::PlayFBAudio(const char* fbAudioPath, const Vec3Tuple& pos){
	return mImpl->PlayFBAudio(fbAudioPath, pos);
}

AudioId AudioManager::PlayAudio(const char* path){
	return mImpl->PlayAudio(path);	
}

AudioId AudioManager::PlayAudio(const char* path, const Vec3Tuple& pos){
	return mImpl->PlayAudio(path, pos);
}

AudioId AudioManager::PlayAudio(const char* path, const AudioProperty& property){
	return mImpl->PlayAudio(path, property);
}

AudioId AudioManager::PlayAudioWithFadeIn(const char* path, const AudioProperty& prop, TIME_PRECISION inSec){
	return mImpl->PlayAudioWithFadeIn(path, prop, inSec);
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

void AudioManager::SetListenerPosition(const Vec3Tuple& pos){
	mImpl->SetListenerPosition(pos);	
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

bool AudioManager::SetGain(AudioId id, float gain, bool checkManipulator){
	return mImpl->SetGain(id, gain, checkManipulator);
}

bool AudioManager::SetGainSmooth(AudioId id, float gain, float inSec){
	return mImpl->SetGainSmooth(id, gain, inSec);
}

float AudioManager::GetGain(AudioId id) const{
	return mImpl->GetGain(id);
}

float AudioManager::GetGainFromFBAudio(const char* fbaudioPath){
	return mImpl->GetGainFromFBAudio(fbaudioPath);
}

bool AudioManager::SetLoop(AudioId id, bool loop){
	return mImpl->SetLoop(id, loop);
}

bool AudioManager::GetLoop(AudioId id) const{
	return mImpl->GetLoop(id);
}

bool AudioManager::SetMaxGain(AudioId id, float maxGain){
	return mImpl->SetMaxGain(id, maxGain);
}

float AudioManager::GetMaxGain(AudioId id){
	return mImpl->GetMaxGain(id);
}

void AudioManager::RegisterAudioEx(AudioExPtr audioex){
	return mImpl->RegisterAudioEx(audioex);
}

bool AudioManager::IsRegisteredAudioEx(AudioExPtr audioex){
	return mImpl->IsRegisteredAudioEx(audioex);
}

void AudioManager::UnregisterEndCallbackForAudio(AudioId id){
	mImpl->UnregisterEndCallbackForAudio(id);
}

void AudioManager::UnregisterEndCallbackFunc(FunctionId funcId){
	mImpl->UnregisterEndCallbackFunc(funcId);
}

void AudioManager::DeleteManipulator(AudioId id, AudioManipulatorType::Enum type){
	mImpl->DeleteManipulator(id, type);
}

void AudioManager::GetAudioList(std::vector<AudioDebugData>& list) const{
	mImpl->GetAudioList(list);		
}

unsigned AudioManager::GetNumGenerated() const{
	return mImpl->mNumGeneratedSources;
}

bool AudioManager::IsValidSource(AudioId id) const{
	return mImpl->IsValidSource(id);
}

bool AudioManager::AudioThreadFunc(){
	using namespace std::chrono;
	static auto tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	auto curTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	auto dt = (curTick - tick) / (TIME_PRECISION)std::milli::den;
	if (dt > 0.03f && sAudioManagerRaw){
		tick = curTick;
		return mImpl->AudioThreadFunc(dt);
	}
	else{
		return sAudioManagerRaw != 0;
	}
}

void AudioManager::SetMasterGain(float gain){
	mImpl->SetMasterGain(gain);
}

float AudioManager::GetMasterGain() const{
	return mImpl->GetMasterGain();
}

void AudioManager::SetEnabled(bool enabled){
	mImpl->SetEnabled(enabled);
}