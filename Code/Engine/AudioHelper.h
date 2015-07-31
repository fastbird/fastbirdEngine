#pragma once

// codes are from alhelpers.h in OpenALSoft Examples.

namespace fastbird{
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