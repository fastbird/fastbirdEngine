#pragma once
#include <string>
#include <chrono>
#include <functional>
#include "Types.h"
namespace fb {
	class ProfilerSimple {
		const char* mName;
		INT64 mStartTick;
		std::function<void(const char*, INT64)> mCallbackFunction;
		mutable TIME_PRECISION mPrevDT;

	public:
		ProfilerSimple(const char* name, std::function<void(const char*, INT64)> callback = {})
			: mName(name)
			, mCallbackFunction(callback)
		{
			using namespace std::chrono;
			mStartTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
		}

		~ProfilerSimple()
		{
			if (mCallbackFunction)
				mCallbackFunction(mName, GetDTMilliSecs());
		}

		TIME_PRECISION GetDT() const {
			using namespace std::chrono;
			TIME_PRECISION dt = 
				(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() - mStartTick) / (TIME_PRECISION)std::milli::den;
			mPrevDT = (dt + mPrevDT) * .5f;
			return mPrevDT;
		}

		INT64 GetDTMilliSecs() const {
			using namespace std::chrono;
			return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() - mStartTick;
		}

		const char* GetName() const {
			return mName;
		}

		void Reset() {
			using namespace std::chrono;
			mStartTick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
		}
	};
}
