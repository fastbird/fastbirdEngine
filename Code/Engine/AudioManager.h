#pragma once
namespace fastbird{
	class AudioManager{
		ALCdevice* mDevice;
		ALCcontext* mContext;

	public:
		AudioManager();
		~AudioManager();
		bool Init();
		void Deinit();

	private:
		static LPALGETSOURCEDVSOFT alGetSourcedvSOFT;

	};
}