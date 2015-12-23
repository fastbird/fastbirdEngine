#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(MusicPlayer);
	class FB_DLL_AUDIOPLAYER MusicPlayer{
		FB_DECLARE_PIMPL_NON_COPYABLE(MusicPlayer);
		MusicPlayer();
		~MusicPlayer();

	public:
		static MusicPlayerPtr Create();
		MusicPlayer& GetInstance() const;
		/// Play looping
		void PlayMusic(const char* path, float fadeOutOld);
		void PlayMusic(const char* path, float fadeOutOld, bool loop);
		void ChangeMusic(const char* path, float fadeOutOld, float startNewAfter);
		void StopMusic(float fadeOut);
		void Update(float dt);
		bool IsPlaying();
	};
}