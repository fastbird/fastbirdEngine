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
#include "AudioSource.h"
#include "AudioBuffer.h"
#include "AudioProperty.h"
using namespace fb;
namespace fb{
	void CheckALError();
	float AudioSource::sMasterGain = 1.f;	
	float AudioSource::sSoundGain = 1.f;
	float AudioSource::sMusicGain = 1.f;
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
	AudioSourceType::Enum mType;

	//---------------------------------------------------------------------------
	Impl(AudioId audioId, AudioSourceType::Enum type)
		: mAudioId(audioId)
		, mALSource(-1)		
		, mPlayingTime(0)
		, mStatus(AudioSourceStatus::Waiting)
		, mDistPerRef(FLT_MAX)
		, mType(type)
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
			alSourcef(mALSource, AL_GAIN, mProperty.mGain * sMasterGain * 
				(mType == AudioSourceType::Music ? sMusicGain : sSoundGain));
			alSourcei(mALSource, AL_LOOPING, mProperty.mLoop ? AL_TRUE : AL_FALSE);
			alSourcef(mALSource, AL_MAX_GAIN, mProperty.mMaxGain);
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
			alSourcef(mALSource, AL_GAIN, gain * sMasterGain);
			CheckALError();
		}
	}

	float GetGain() const{
		return mProperty.mGain;
	}

	void OnGainOptionChanged() {
		if (mALSource != -1) {
			alSourcef(mALSource, AL_GAIN, mProperty.mGain * sMasterGain *
				(mType == AudioSourceType::Music ? sMusicGain : sSoundGain));
			CheckALError();
		}
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

	void SetMaxGain(float maxGain){
		mProperty.mMaxGain = maxGain;
		if (mALSource != -1){
			alSourcef(mALSource, AL_MAX_GAIN, maxGain);
			CheckALError();
		}
	}
};
//---------------------------------------------------------------------------
AudioSourcePtr AudioSource::Create(AudioId audioId, AudioSourceType::Enum type){
	return AudioSourcePtr(new AudioSource(audioId, type), [](AudioSource* obj){delete obj; });
}
AudioSource::AudioSource(AudioId audioId, AudioSourceType::Enum type)
	: mImpl(new Impl(audioId, type))
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

void AudioSource::OnGainOptionChanged() {
	mImpl->OnGainOptionChanged();
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

void AudioSource::SetMaxGain(float maxGain){
	mImpl->SetMaxGain(maxGain);
}

float AudioSource::GetMaxGain() const{
	return mImpl->mProperty.mMaxGain;
}

size_t AudioSource::GetNumSimultaneous() const {
	return mImpl->mProperty.mNumSimultaneous;
}

float AudioSource::GetSimultaneousCheckRange() const {
	return mImpl->mProperty.mSimultaneousCheckRange;
}