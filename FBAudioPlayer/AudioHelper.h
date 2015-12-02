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

#pragma once

// codes are from alhelpers.h in OpenALSoft Examples.

namespace fb{
	/* Some helper functions to get the name from the channel and type enums. */
	const char *ChannelsName(ALenum chans);
	const char *TypeName(ALenum type);

	/* Helpers to convert frame counts and byte lengths. */
	ALsizei FramesToBytes(ALsizei size, ALenum channels, ALenum type);
	ALsizei BytesToFrames(ALsizei size, ALenum channels, ALenum type);

	/* Retrieves a compatible buffer format given the channel configuration and
	* sample type. If an alIsBufferFormatSupportedSOFT-compatible function is
	* provided, it will be called to find the closest-matching format from
	* AL_SOFT_buffer_samples. Returns AL_NONE (0) if no supported format can be
	* found. */
	ALenum GetAudioFormat(ALenum channels, ALenum type, LPALISBUFFERFORMATSUPPORTEDSOFT palIsBufferFormatSupportedSOFT);

	/* Loads samples into a buffer using the standard alBufferData call, but with a
	* LPALBUFFERSAMPLESSOFT-compatible prototype. Assumes internalformat is valid
	* for alBufferData, and that channels and type match it. */
	void AL_APIENTRY wrap_BufferSamples(ALuint buffer, ALuint samplerate,
		ALenum internalformat, ALsizei samples,
		ALenum channels, ALenum type,
		const ALvoid *data);

	ALenum ChannelsToAL(int channels);
}