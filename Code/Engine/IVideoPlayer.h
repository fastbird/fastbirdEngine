#pragma once

namespace fastbird{
	class IVideoPlayer{
	public:
		typedef std::function<void(IVideoPlayer*)> VideoNotifierFunc;
		virtual bool PlayVideo(const char* path) = 0;
		virtual void StopVideo() = 0;

		virtual void RegisterVideoNotifier(VideoNotifierFunc func) = 0;
		
		// Update called internally.
		virtual void Update(float dt) = 0;

		virtual bool IsFinish() const = 0;
	};
}