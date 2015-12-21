#include "stdafx.h"
#include "AudioBuffer.h"
using namespace fb;
AudioBuffer::AudioBuffer()
	: mLastAccessed(0)
	, mBuffer(0)
	, mReferences(0)
	, mLength(0){

}

AudioBuffer::~AudioBuffer(){
	alDeleteBuffers(1, &mBuffer);
}