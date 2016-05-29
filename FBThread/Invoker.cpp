#include "stdafx.h"
#include "Invoker.h"
#include "AsyncObjects.h"
using namespace fb;

class Invoker::Impl {
public:
	std::vector<std::function<void()>> mFuncsAtStart;
	std::vector<std::function<void()>> mFuncsAtEnd;
	FB_CRITICAL_SECTION mLockStart;
	FB_CRITICAL_SECTION mLockEnd;
	std::atomic<bool> mPreparedQuit{ false };

	void InvokeAtStart(std::function<void()>&& func)
	{
		ENTER_CRITICAL_SECTION lock(mLockStart);
		if (mPreparedQuit)
			return;
		mFuncsAtStart.push_back(func);
	}

	void InvokeAtEnd(std::function<void()>&& func)
	{
		ENTER_CRITICAL_SECTION lock(mLockEnd);
		if (mPreparedQuit)
			return;
		mFuncsAtEnd.push_back(func);
	}

	void Start()
	{
		std::vector<std::function<void()>> funtions;
		{
			ENTER_CRITICAL_SECTION lock(mLockStart);
			funtions.swap(mFuncsAtStart);
		}
		for (auto& f : funtions) {
			f();
		}
	}

	void End()
	{
		std::vector<std::function<void()>> funtions;
		{
			ENTER_CRITICAL_SECTION lock(mLockEnd);
			funtions.swap(mFuncsAtEnd);
		}
		for (auto& f : funtions) {
			f();
		}		
	}
	void PrepareQuit()
	{
		mPreparedQuit = true; 
		{
			ENTER_CRITICAL_SECTION lock(mLockStart);
			mFuncsAtStart.clear();
		}
		{
			ENTER_CRITICAL_SECTION lock(mLockEnd);
			mFuncsAtEnd.clear();
		}
	}
};

static Invoker* sInvokerRaw = 0;
InvokerWeakPtr sInvoker;

InvokerPtr Invoker::Create() {
	InvokerPtr p(new Invoker(), [](Invoker* obj) {delete obj; });
	if (sInvoker.expired()) {
		sInvoker = p;
		sInvokerRaw = p.get();
	}
	return p;
}


Invoker& Invoker::GetInstance() {
	return *sInvokerRaw;
}

bool Invoker::HasInstance() {
	return !sInvoker.expired();
}

Invoker::Invoker()
	: mImpl(new Impl) {

}

Invoker::~Invoker() = default;

/// invode 'func' at the beginning of update loop
void Invoker::InvokeAtStart(std::function<void()>&& func) {
	mImpl->InvokeAtStart(std::move(func));
}
/// invode 'func' at the end of update loop
void Invoker::InvokeAtEnd(std::function<void()>&& func) {
	mImpl->InvokeAtEnd(std::move(func));
}

/// If you use EngineFacade don't need to call this function manually
void Invoker::Start() {
	mImpl->Start();
}
/// If you use EngineFacade don't need to call this function manually
void Invoker::End() {
	mImpl->End();
}

void Invoker::PrepareQuit() {
	mImpl->PrepareQuit();
}