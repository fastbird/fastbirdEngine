#include <Engine/StdAfx.h>
#include <Engine/CloudManager.h>
#include <Engine/VolumetricCloud.h>

namespace fastbird
{
	bool gQuitCloudThread = false;
	SyncEvent* gConsumed = 0;
	void CloudUpdateThread(CloudManager* cm);
	

	//-----------------------------------------------------------------------
	CloudManager::CloudManager()
		:mReadyNextData(false)
	{
		gConsumed = CreateSyncEvent();
	}

	//-----------------------------------------------------------------------
	CloudManager::~CloudManager()
	{
		CleanCloud();
		DeleteSyncEvent(gConsumed);
	}

	//-----------------------------------------------------------------------
	bool CloudManager::InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds)
	{
		CleanCloud();

		for (unsigned i = 0; i < numCloud; ++i)
		{
			mClouds.push_back(FB_NEW(VolumetricCloud));
			VolumetricCloud* pC = mClouds.back();
			pC->Setup(clouds[i]);
		}

		mThread = std::thread(CloudUpdateThread, this);
		return true;
	}

	//-----------------------------------------------------------------------
	void CloudManager::CleanCloud()
	{
		gQuitCloudThread = true;
		if (mThread.joinable())
		{
			gConsumed->Trigger();
			mThread.join();
		}
			
		gQuitCloudThread = false;

		FB_FOREACH(it, mClouds)
		{
			FB_SAFE_DEL(*it);
		}
	}

	//-----------------------------------------------------------------------
	void CloudManager::Update()
	{
		if (mReadyNextData)
		{
			mReadyNextData = false;
			FB_FOREACH(it, mClouds)
			{
				(*it)->PrepareRender();
			}
			gConsumed->Trigger();
		}
	}

	//-----------------------------------------------------------------------
	VolumetricCloud* CloudManager::GetCloud(size_t idx) const
	{
		assert(idx < mClouds.size());
		return mClouds[idx];
	}


	//-----------------------------------------------------------------------
	// Cloud Update Thread
	void CloudUpdateThread(CloudManager* cm)
	{
		size_t numClouds = cm->GetNumClouds();
		while (!gQuitCloudThread)
		{
			{
				Profiler profile("CloudUpdate");

				float time = gpTimer->GetTime();
				for (size_t i = 0; i < numClouds; ++i)
				{
					cm->GetCloud(i)->AdvanceTime(time, 1);
				}
				cm->Ready();
			}
			gConsumed->Wait();
		}
	}

}