#include <EngineApp/StdAfx.h>
#include <EngineApp/UI.h>
#include <EngineApp/FleetUI.h>

using namespace fastbird;

//---------------------------------------------------------------------------
UIs::UIs()
: pFleetUI(0)
{
	memset(mUIHolder, 0, sizeof(mUIHolder));
}

//---------------------------------------------------------------------------
UIs::~UIs()
{
	for each(auto ui in mUIHolder)
	{
		FB_SAFE_DEL(ui);
	}
}


void UIs::OnEnterPlayState()
{
}

//---------------------------------------------------------------------------
void UIs::Update(float dt)
{
	std::vector<int>::iterator it = mReloadUIs.begin(), itEnd = mReloadUIs.end();
	for (; it != itEnd; ++it)
	{
		bool visible = mUIHolder[*it]->IsVisible();
		mUIHolder[*it]->Deinitialize();
		mUIHolder[*it]->Initialize();
		mUIHolder[*it]->OnReload();
		mUIHolder[*it]->SetVisible(visible);
	}

	mReloadUIs.clear();

	for each(auto ui in mUIHolder)
	{
		if (ui)
			ui->Update(dt);
	}
}


//---------------------------------------------------------------------------
void UIs::OnBackgroundClicked()
{
}

//---------------------------------------------------------------------------
void UIs::OnUIFileChanged(const char* file)
{
	for (int i = 0; i < UIS_COUNT; ++i)
	{
		if (!mUIHolder[i])
			continue;
		if (mUIHolder[i]->IsFile(file))
		{
			mReloadUIs.push_back(i);
			return;
		}
	}
}

void UIs::AddExclusives(UIS target, UIS close)
{
	assert(std::find(mExclusives[target].begin(), mExclusives[target].end(), close) == mExclusives[target].end());
	mExclusives[target].push_back(close);
	assert(std::find(mExclusives[close].begin(), mExclusives[close].end(), target) == mExclusives[close].end());
	mExclusives[close].push_back(target);
}

//---------------------------------------------------------------------------
void UIs::SetVisible(bool visible, UIS ui)
{
	bool useHistory = true;
	if (visible)
	{
		auto it = mExclusives.Find(ui);
		if (it != mExclusives.end())
		{
			for each (auto exclUI in it->second)
			{
				SetVisible(false, exclUI);
			}
		}
	}

	switch(ui)
	{
	case UIS_FLEET_UI:
		if (!pFleetUI)
		{
			mUIHolder[UIS_FLEET_UI] = pFleetUI = FB_NEW(FleetUI);
		}
		pFleetUI->SetVisible(visible);
		useHistory = true;
		break;

	default:
		fastbird::Error("UIs::SetVisible: Unkown type of ui");
		assert(0);
	}

	if (useHistory)
	{
		if (visible)
		{
			mVisibleHistory.push_back(ui);
		}
		else
		{
			mVisibleHistory.erase(std::remove(mVisibleHistory.begin(), mVisibleHistory.end(), ui), mVisibleHistory.end());

		}
	}
}

void UIs::OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	if (pKeyboard->IsValid())
	{
		if (pKeyboard->IsKeyPressed(VK_ESCAPE))
		{
			if (!mVisibleHistory.empty())
			{
				auto it = mVisibleHistory.end() - 1;
				SetVisible(false, *it);
				pKeyboard->Invalidate();
			}
		}
	}
	if (pFleetUI)
		pFleetUI->OnInputFromHandler(pMouse, pKeyboard);
}

//---------------------------------------------------------------------------
bool UIs::GetVisible(UIS ui) const
{
	assert(ui >= 0 && ui < UIS_COUNT);
	if (!mUIHolder[ui])
	{
		return false;
	}

	return mUIHolder[ui]->IsVisible();
}

//------------------------------------------------------------------
//------------------------------------------------------------------
CUI::CUI()
	: mVisible(true)
	, mInitialized(false)
{
}
void CUI::SetVisible(bool visible) 
{
	if (!mInitialized && visible)
		Initialize();
	mVisible = visible;
	if (visible && GetRootWnd())
		IUIManager::GetUIManager().SetFocusUI(GetRootWnd());
}