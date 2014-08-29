#pragma once

namespace fastbird
{
	class VolumetricCloud;
	struct CloudProperties;
	class CloudManager
	{
	public:
		CloudManager();
		~CloudManager();

		struct Cloud
		{
			Vec3 mSize;
			Vec3 mPos;
			unsigned mParticleID;
		};
		bool InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds);
		void CleanCloud();
		void Update();
		size_t GetNumClouds() const { return mClouds.size(); }
		VolumetricCloud* GetCloud(size_t idx) const;
		void Ready() { mReadyNextData = true; }

	private:
		bool mReadyNextData;
		std::vector<VolumetricCloud*> mClouds;
		std::thread mThread;
	};
}