#pragma once

namespace fastbird{
	class IVideoPlayer{
	public:
		virtual ~IVideoPlayer(){}
		typedef std::function<void(IVideoPlayer*)> VideoNotifierFunc;
		virtual bool PlayVideo(const char* path) = 0;
		virtual void StopVideo() = 0;

		virtual void RegisterVideoNotifier(VideoNotifierFunc func) = 0;
		virtual void SetDurationAfterFinish(float time) = 0;
		// Update called internally.
		virtual void Update(float dt) = 0;

		virtual bool IsFinish() const = 0;
	};
}