#include "stdafx.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "AudioProperty.h"
using namespace fb;
namespace fb{
	void CheckALError();
}

class AudioSource::Impl{
public:
	AudioId mAudioId;
	AudioBufferPtr mAudioBuffer;
	ALuint mALSource;
	AudioProperty mProperty;
	float mPlayingTime;
	AudioSourceStatus::Enum mStatus;
	float mDistPerRef;

	//---------------------------------------------------------------------------
	Impl(AudioId audioId)
		: mAudioId(audioId)
		, mALSource(-1)		
		, mPlayingTime(0)
		, mStatus(AudioSourceStatus::Waiting)
		, mDistPerRef(FLT_MAX)
	{		

	}

	~Impl(){
		if (mAudioBuffer)
			mAudioBuffer->mReferences--;
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

	void SetStatus(AudioSourceStatus::Enum status){
		mStatus = status;
	}

	AudioSourceStatus::Enum GetStatus(){
		return mStatus;
	}

	void SetProperty(const AudioProperty& prop){
		mProperty = prop;
	}

	void ApplyProp(){
		if (mALSource != -1){
			alSource3f(mALSource, AL_POSITION, std::get<0>(mProperty.mPosition),
				std::get<1>(mProperty.mPosition), std::get<2>(mProperty.mPosition));
			alSourcei(mALSource, AL_SOURCE_RELATIVE, mProperty.mRelative ? AL_TRUE : AL_FALSE);
			alSourcef(mALSource, AL_REFERENCE_DISTANCE, mProperty.mReferenceDistance);
			alSourcef(mALSource, AL_ROLLOFF_FACTOR, mProperty.mRolloffFactor);
			alSourcef(mALSource, AL_GAIN, mProperty.mGain);
			alSourcei(mALSource, AL_LOOPING, mProperty.mLoop ? AL_TRUE : AL_FALSE);
			CheckALError();
		}
	}

	void ApplyRemainedTime(){
		if (mALSource != -1){
			alSourcef(mALSource, AL_SEC_OFFSET, mPlayingTime);
			CheckALError();
		}
	}

	bool IsPlaying() const {
		return mStatus == AudioSourceStatus::Playing;
	}

	bool Update(float dt){
		mPlayingTime += dt;
		if (mStatus == AudioSourceStatus::Waiting && mPlayingTime >= mAudioBuffer->mLength)
			return true; // delete me

		return false; // dont delete me.
	}

	void SetPosition(float x, float y, float z){
		mProperty.mPosition = std::make_tuple(x, y, z);
		if (mALSource != -1){
			alSource3f(mALSource, AL_POSITION, x, y, z);
			CheckALError();
		}
	}

	Vec3Tuple GetPosition() const{
		return mProperty.mPosition;
	}
	void SetRelative(bool relative){
		mProperty.mRelative = relative;
		if (mALSource != -1){
			alSourcei(mALSource, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
			CheckALError();
		}
	}

	bool GetRelative() const {
		return mProperty.mRelative;
	}

	void SetReferenceDistance(float distance){
		mProperty.mReferenceDistance = distance;
		if (mALSource != -1){
			alSourcef(mALSource, AL_REFERENCE_DISTANCE, distance);
			CheckALError();
		}
	}

	float GetReferenceDistance(){
		return mProperty.mReferenceDistance;
	}

	void SetRollOffFactor(float factor){
		mProperty.mRolloffFactor = factor;
		if (mALSource != -1){
			alSourcef(mALSource, AL_ROLLOFF_FACTOR, factor);
			CheckALError();
		}
	}

	void SetOffsetInSec(float sec){
		mPlayingTime = sec;
		if (mALSource != -1){
			alSourcef(mALSource, AL_SEC_OFFSET, mPlayingTime);
			CheckALError();
		}
	}

	void SetGain(float gain){
		mProperty.mGain = gain;
		if (mALSource != -1){
			alSourcef(mALSource, AL_GAIN, gain);
			CheckALError();
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

	void SetDistPerRef(float distPerRef){
		mDistPerRef = distPerRef;
	}

	void SetLoop(bool loop){
		mProperty.mLoop = loop;
		if (mALSource != -1){
			alSourcei(mALSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
			CheckALError();
		}
	}
};
//---------------------------------------------------------------------------
AudioSourcePtr AudioSource::Create(AudioId audioId){
	return AudioSourcePtr(new AudioSource(audioId), [](AudioSource* obj){delete obj; });
}
AudioSource::AudioSource(AudioId audioId)
	: mImpl(new Impl(audioId))
{

}

AudioSource::~AudioSource(){

}

const AudioId& AudioSource::GetAudioId() const {
	return mImpl->GetAudioId();
}

void AudioSource::SetAudioBuffer(AudioBufferPtr buffer) {
	mImpl->SetAudioBuffer(buffer);
}

AudioBufferPtr AudioSource::GetAudioBuffer() const {
	return mImpl->GetAudioBuffer();
}

void AudioSource::SetALAudioSource(unsigned src) {
	mImpl->SetALAudioSource(src);
}

unsigned AudioSource::GetALAudioSource() const {
	return mImpl->GetALAudioSource();
}

void AudioSource::SetStatus(AudioSourceStatus::Enum status) {
	mImpl->SetStatus(status);
}

AudioSourceStatus::Enum AudioSource::GetStatus() {
	return mImpl->GetStatus();
}

void AudioSource::SetProperty(const AudioProperty& prop) {
	mImpl->SetProperty(prop);
}

void AudioSource::ApplyProp() {
	mImpl->ApplyProp();
}

void AudioSource::ApplyRemainedTime() {
	mImpl->ApplyRemainedTime();
}

bool AudioSource::IsPlaying() const {
	return mImpl->IsPlaying();
}

bool AudioSource::Update(float dt) {
	return mImpl->Update(dt);
}

void AudioSource::SetPosition(float x, float y, float z) {
	mImpl->SetPosition(x, y, z);
}

Vec3Tuple AudioSource::GetPosition() const {
	return mImpl->GetPosition();
}

void AudioSource::SetRelative(bool relative) {
	mImpl->SetRelative(relative);
}

bool AudioSource::GetRelative() const {
	return mImpl->GetRelative();
}

void AudioSource::SetReferenceDistance(float distance) {
	mImpl->SetReferenceDistance(distance);
}

float AudioSource::GetReferenceDistance() {
	return mImpl->GetReferenceDistance();
}

void AudioSource::SetRollOffFactor(float factor) {
	mImpl->SetRollOffFactor(factor);
}

void AudioSource::SetOffsetInSec(float sec) {
	mImpl->SetOffsetInSec(sec);
}

void AudioSource::SetGain(float gain) {
	mImpl->SetGain(gain);
}

float AudioSource::GetGain() const {
	return mImpl->GetGain();
}

float AudioSource::GetLeftTime() const {
	return mImpl->GetLeftTime();
}

float AudioSource::GetLength() const {
	return mImpl->GetLength();
}

void AudioSource::SetDistPerRef(float distPerRef){
	mImpl->SetDistPerRef(distPerRef);
}

float AudioSource::GetDistPerRef() const{
	return mImpl->mDistPerRef;
}

void AudioSource::SetLoop(bool loop){
	mImpl->SetLoop(loop);
}

bool AudioSource::GetLoop() const{
	return mImpl->mProperty.mLoop;
}