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
#include "AudioHelper.h"

namespace fb{

	/* GetFormat retrieves a compatible buffer format given the channel config and
	* sample type. If an alIsBufferFormatSupportedSOFT-compatible function is
	* provided, it will be called to find the closest-matching format from
	* AL_SOFT_buffer_samples. Returns AL_NONE (0) if no supported format can be
	* found. */
	ALenum GetAudioFormat(ALenum channels, ALenum type, LPALISBUFFERFORMATSUPPORTEDSOFT palIsBufferFormatSupportedSOFT)
	{
		ALenum format = AL_NONE;

		/* If using AL_SOFT_buffer_samples, try looking through its formats */
		if (palIsBufferFormatSupportedSOFT)
		{
			/* AL_SOFT_buffer_samples is more lenient with matching formats. The
			* specified sample type does not need to match the returned format,
			* but it is nice to try to get something close. */
			if (type == AL_UNSIGNED_BYTE_SOFT || type == AL_BYTE_SOFT)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO8_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO8_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD8_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_8_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_8_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_8_SOFT;
			}
			else if (type == AL_UNSIGNED_SHORT_SOFT || type == AL_SHORT_SOFT)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO16_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO16_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD16_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_16_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_16_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_16_SOFT;
			}
			else if (type == AL_UNSIGNED_BYTE3_SOFT || type == AL_BYTE3_SOFT ||
				type == AL_UNSIGNED_INT_SOFT || type == AL_INT_SOFT ||
				type == AL_FLOAT_SOFT || type == AL_DOUBLE_SOFT)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO32F_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO32F_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD32F_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_32F_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_32F_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_32F_SOFT;
			}

			if (format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
				format = AL_NONE;

			/* A matching format was not found or supported. Try 32-bit float. */
			if (format == AL_NONE)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO32F_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO32F_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD32F_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_32F_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_32F_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_32F_SOFT;

				if (format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
					format = AL_NONE;
			}
			/* 32-bit float not supported. Try 16-bit int. */
			if (format == AL_NONE)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO16_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO16_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD16_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_16_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_16_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_16_SOFT;

				if (format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
					format = AL_NONE;
			}
			/* 16-bit int not supported. Try 8-bit int. */
			if (format == AL_NONE)
			{
				if (channels == AL_MONO_SOFT) format = AL_MONO8_SOFT;
				else if (channels == AL_STEREO_SOFT) format = AL_STEREO8_SOFT;
				else if (channels == AL_QUAD_SOFT) format = AL_QUAD8_SOFT;
				else if (channels == AL_5POINT1_SOFT) format = AL_5POINT1_8_SOFT;
				else if (channels == AL_6POINT1_SOFT) format = AL_6POINT1_8_SOFT;
				else if (channels == AL_7POINT1_SOFT) format = AL_7POINT1_8_SOFT;

				if (format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
					format = AL_NONE;
			}

			return format;
		}

		/* We use the AL_EXT_MCFORMATS extension to provide output of Quad, 5.1,
		* and 7.1 channel configs, AL_EXT_FLOAT32 for 32-bit float samples, and
		* AL_EXT_DOUBLE for 64-bit float samples. */
		if (type == AL_UNSIGNED_BYTE_SOFT)
		{
			if (channels == AL_MONO_SOFT)
				format = AL_FORMAT_MONO8;
			else if (channels == AL_STEREO_SOFT)
				format = AL_FORMAT_STEREO8;
			else if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
			{
				if (channels == AL_QUAD_SOFT)
					format = alGetEnumValue("AL_FORMAT_QUAD8");
				else if (channels == AL_5POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_51CHN8");
				else if (channels == AL_6POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_61CHN8");
				else if (channels == AL_7POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_71CHN8");
			}
		}
		else if (type == AL_SHORT_SOFT)
		{
			if (channels == AL_MONO_SOFT)
				format = AL_FORMAT_MONO16;
			else if (channels == AL_STEREO_SOFT)
				format = AL_FORMAT_STEREO16;
			else if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
			{
				if (channels == AL_QUAD_SOFT)
					format = alGetEnumValue("AL_FORMAT_QUAD16");
				else if (channels == AL_5POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_51CHN16");
				else if (channels == AL_6POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_61CHN16");
				else if (channels == AL_7POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_71CHN16");
			}
		}
		else if (type == AL_FLOAT_SOFT && alIsExtensionPresent("AL_EXT_FLOAT32"))
		{
			if (channels == AL_MONO_SOFT)
				format = alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
			else if (channels == AL_STEREO_SOFT)
				format = alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
			else if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
			{
				if (channels == AL_QUAD_SOFT)
					format = alGetEnumValue("AL_FORMAT_QUAD32");
				else if (channels == AL_5POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_51CHN32");
				else if (channels == AL_6POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_61CHN32");
				else if (channels == AL_7POINT1_SOFT)
					format = alGetEnumValue("AL_FORMAT_71CHN32");
			}
		}
		else if (type == AL_DOUBLE_SOFT && alIsExtensionPresent("AL_EXT_DOUBLE"))
		{
			if (channels == AL_MONO_SOFT)
				format = alGetEnumValue("AL_FORMAT_MONO_DOUBLE");
			else if (channels == AL_STEREO_SOFT)
				format = alGetEnumValue("AL_FORMAT_STEREO_DOUBLE");
		}

		/* NOTE: It seems OSX returns -1 from alGetEnumValue for unknown enums, as
		* opposed to 0. Correct it. */
		if (format == -1)
			format = 0;

		return format;
	}

	void AL_APIENTRY wrap_BufferSamples(ALuint buffer, ALuint samplerate,
		ALenum internalformat, ALsizei samples,
		ALenum channels, ALenum type,
		const ALvoid *data)
	{
		alBufferData(buffer, internalformat, data,
			FramesToBytes(samples, channels, type),
			samplerate);
	}

	const char *ChannelsName(ALenum chans)
	{
		switch (chans)
		{
		case AL_MONO_SOFT: return "Mono";
		case AL_STEREO_SOFT: return "Stereo";
		case AL_REAR_SOFT: return "Rear";
		case AL_QUAD_SOFT: return "Quadraphonic";
		case AL_5POINT1_SOFT: return "5.1 Surround";
		case AL_6POINT1_SOFT: return "6.1 Surround";
		case AL_7POINT1_SOFT: return "7.1 Surround";
		}
		return "Unknown Channels";
	}

	const char *TypeName(ALenum type)
	{
		switch (type)
		{
		case AL_BYTE_SOFT: return "S8";
		case AL_UNSIGNED_BYTE_SOFT: return "U8";
		case AL_SHORT_SOFT: return "S16";
		case AL_UNSIGNED_SHORT_SOFT: return "U16";
		case AL_INT_SOFT: return "S32";
		case AL_UNSIGNED_INT_SOFT: return "U32";
		case AL_FLOAT_SOFT: return "Float32";
		case AL_DOUBLE_SOFT: return "Float64";
		}
		return "Unknown Type";
	}


	ALsizei FramesToBytes(ALsizei size, ALenum channels, ALenum type)
	{
		switch (channels)
		{
		case AL_MONO_SOFT:    size *= 1; break;
		case AL_STEREO_SOFT:  size *= 2; break;
		case AL_REAR_SOFT:    size *= 2; break;
		case AL_QUAD_SOFT:    size *= 4; break;
		case AL_5POINT1_SOFT: size *= 6; break;
		case AL_6POINT1_SOFT: size *= 7; break;
		case AL_7POINT1_SOFT: size *= 8; break;
		}

		switch (type)
		{
		case AL_BYTE_SOFT:           size *= sizeof(ALbyte); break;
		case AL_UNSIGNED_BYTE_SOFT:  size *= sizeof(ALubyte); break;
		case AL_SHORT_SOFT:          size *= sizeof(ALshort); break;
		case AL_UNSIGNED_SHORT_SOFT: size *= sizeof(ALushort); break;
		case AL_INT_SOFT:            size *= sizeof(ALint); break;
		case AL_UNSIGNED_INT_SOFT:   size *= sizeof(ALuint); break;
		case AL_FLOAT_SOFT:          size *= sizeof(ALfloat); break;
		case AL_DOUBLE_SOFT:         size *= sizeof(ALdouble); break;
		}

		return size;
	}

	ALsizei BytesToFrames(ALsizei size, ALenum channels, ALenum type)
	{
		return size / FramesToBytes(1, channels, type);
	}

	ALenum ChannelsToAL(int channels){
		switch (channels){
		case 1:
			return AL_MONO_SOFT;
		case 2:
			return AL_STEREO_SOFT;
		}

		Error("Can not find channels enum for %d", channels);
		return AL_STEREO_SOFT;
	}

	TIME_PRECISION GetDuration(unsigned bufferSize, unsigned frequency, unsigned channels, unsigned bitsPerSample){
		return (TIME_PRECISION)bufferSize / (TIME_PRECISION)(frequency * channels * (bitsPerSample / 8));
	}
}



