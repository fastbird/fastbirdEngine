#pragma once
#include "FBCommonHeaders/Types.h"
#include "AudioSourceStatus.h"
namespace fb{
	struct AudioProperty;
	FB_DECLARE_SMART_PTR_STRUCT(AudioBuffer);
	FB_DECLARE_SMART_PTR_STRUCT(AudioSource);
	struct FB_DLL_AUDIOPLAYER AudioSource{
		FB_DECLARE_PIMPL_NON_COPYABLE(AudioSource);
		AudioSource();
		~AudioSource();

	public:
		static AudioSourcePtr Create();

		void SetAudioId(AudioId id);
		const AudioId& GetAudioId() const;
		void SetAudioBuffer(AudioBufferPtr buffer);
		AudioBufferPtr GetAudioBuffer() const;
		void SetALAudioSource(unsigned src);
		unsigned GetALAudioSource() const;
		void SetStatus(AudioSourceStatus::Enum status);
		AudioSourceStatus::Enum GetStatus();
		void SetProperty(const AudioProperty& prop);
		void ApplyProp();
		void ApplyRemainedTime();
		bool IsPlaying() const;
		bool Update(float dt);
		void SetPosition(float x, float y, float z);
		Vec3Tuple GetPosition() const;
		void SetRelative(bool relative);
		bool GetRelative() const;
		void SetReferenceDistance(float distance);
		float GetReferenceDistance();
		void SetRollOffFactor(float factor);
		void SetOffsetInSec(float sec);
		void SetGain(float gain);
		float GetGain() const;
		float GetLeftTime() const;
		float GetLength() const;
		void SetDistPerRef(float distPerRef);
		float GetDistPerRef() const;
		void SetLoop(bool loop);
		bool GetLoop() const;
	};
}