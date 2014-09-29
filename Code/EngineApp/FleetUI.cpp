#include <EngineApp/StdAfx.h>
#include <EngineApp/FleetUI.h>
#include <UI/Wnd.h>
#include <UI/Button.h>
#include <UI/IUIManager.h>
#include <UI/ImageBox.h>
#include <UI/StaticText.h>
#include <UI/CheckBox.h>
#include <UI/DropDown.h>

using namespace fastbird;

extern IMeshObject* gMeshObject;
extern UIs* gUI;

FleetUI::FleetUI()
: mWnd(0)
, mRenderToTexture(0)
, mRenderTargetCamDist(50)
, mLightDir(-1.0, 0.01f, 0.3f)
, mPatrol(0)
, mPopulationPolicy(0)
{
	mLaunchExceptions[0] = 0;
	mLaunchExceptions[1] = 0;

	mLightDir.Normalize();
}

FleetUI::~FleetUI()
{
	if (mInitialized)
		Deinitialize();
}

bool FleetUI::Initialize()
{
	IUIManager& um = IUIManager::GetUIManager();
	mUIFilename = "data/ui/FleetUI.ui";
	std::vector<IWinBase*> wnds;
	bool success = um.ParseUI(mUIFilename.c_str(), wnds, mUIName);
	assert(success);
	if (success)
	{
		mInitialized = true;

		mWnd = (Wnd*)wnds[0];
		assert(mWnd);

		mShipImage = (ImageBox*)mWnd->GetChild("ShipImage", true); assert(mShipImage);
		mNumModules[0] = (StaticText*)mWnd->GetChild("NumModules", true); assert(mNumModules[0]);
		mNumModules[1] = (StaticText*)mWnd->GetChild("NumResi", true); assert(mNumModules[1]);
		mNumModules[2] = (StaticText*)mWnd->GetChild("NumHulls", true); assert(mNumModules[2]);
		mNumModules[3] = (StaticText*)mWnd->GetChild("NumShields", true); assert(mNumModules[3]);
		mNumModules[4] = (StaticText*)mWnd->GetChild("NumCombats", true); assert(mNumModules[4]);
		mNumModules[5] = (StaticText*)mWnd->GetChild("NumPowers", true); assert(mNumModules[5]);
		
		mNumShips[0] = (StaticText*)mWnd->GetChild("NumShip", true); assert(mNumShips[0]);
		mNumShips[1] = (StaticText*)mWnd->GetChild("NumFighters", true); assert(mNumShips[1]);
		mNumShips[2] = (StaticText*)mWnd->GetChild("NumBombers", true); assert(mNumShips[2]);
		mNumShips[3] = (StaticText*)mWnd->GetChild("NumCorvettes", true); assert(mNumShips[3]);

		mPatrolOptions = (DropDown*)mWnd->GetChild("Patrols", true); assert(mPatrolOptions);
		mPatrolOptions->RegisterEventFunc(IEventHandler::EVENT_DROP_DOWN_SELECTED,
			std::bind(&FleetUI::OnPatrolOptionChanged, this, std::placeholders::_1));
		
		mLaunchExceptionDD[0] = (DropDown*)mWnd->GetChild("Exception1", true); assert(mLaunchExceptionDD[0]);
		mLaunchExceptionDD[0]->RegisterEventFunc(IEventHandler::EVENT_DROP_DOWN_SELECTED,
			std::bind(&FleetUI::OnException1Changed, this, std::placeholders::_1));
		mLaunchExceptionDD[1] = (DropDown*)mWnd->GetChild("Exception2", true); assert(mLaunchExceptionDD[1]);
		mLaunchExceptionDD[1]->RegisterEventFunc(IEventHandler::EVENT_DROP_DOWN_SELECTED,
			std::bind(&FleetUI::OnException2Changed, this, std::placeholders::_1));
		
		mPopulation[0] = (StaticText*)mWnd->GetChild("Population", true); assert(mPopulation[0]);
		mPopulation[1] = (StaticText*)mWnd->GetChild("Children", true); assert(mPopulation[1]);
		mPopulation[2] = (StaticText*)mWnd->GetChild("Adults", true); assert(mPopulation[2]);

		mPopPolicyDD = (DropDown*)mWnd->GetChild("PopPolicy", true); assert(mPopPolicyDD);
		mPopPolicyDD->RegisterEventFunc(IEventHandler::EVENT_DROP_DOWN_SELECTED,
			std::bind(&FleetUI::OnPopulationPolicyChanged, this, std::placeholders::_1));

		mAutoRepairCB = (CheckBox*)mWnd->GetChild("autoRepair", true); assert(mAutoRepairCB);
		mRepair = (Button*)mWnd->GetChild("repair", true); assert(mRepair);
		mRepair->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&FleetUI::OnRepair, this, std::placeholders::_1));
		mCancelBtn = (Button*)mWnd->GetChild("Cancel", true); assert(mCancelBtn);
		mCancelBtn->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&FleetUI::OnCancelBtn, this, std::placeholders::_1));
		mOKBtn = (Button*)mWnd->GetChild("OKBtn", true); assert(mOKBtn);
		mOKBtn->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
			std::bind(&FleetUI::OnOKBtn, this, std::placeholders::_1));

		PreparePortrait();
	}

	return true;
}
void FleetUI::Deinitialize()
{
	IUIManager& um = IUIManager::GetUIManager();
	um.DeleteWindow(mWnd);
	mWnd = 0;
	mInitialized = false;
	gEnv->pRenderer->DeleteRenderToTexture(mRenderToTexture);
	mRenderToTexture = 0;
}
void FleetUI::SetVisible(bool visible)
{
	__super::SetVisible(visible);
	if (mWnd)
		mWnd->SetVisible(visible);

	if (mRenderToTexture)
		mRenderToTexture->SetEnable(visible);

	if (visible)
	{
		mPatrolOptions->SetSelectedIndex(mPatrol);
		mLaunchExceptionDD[0]->SetSelectedIndex(mLaunchExceptions[0]);
		mLaunchExceptionDD[1]->SetSelectedIndex(mLaunchExceptions[1]);
		mAutoRepairCB->SetCheck(mAutoRepair);
		mPopPolicyDD->SetSelectedIndex(mPopulationPolicy);
		RefreshAll();
	}

}
void FleetUI::Update(float dt)
{
	if (!mInitialized)
		return;

	if (mVisible)
	{
		
	}
}

void FleetUI::OnRepair(void* arg)
{
}

void FleetUI::OnAutoRepair(void* arg)
{
	CheckBox* c = (CheckBox*)arg;
	mAutoRepair = c->GetCheck();
}

void FleetUI::OnCancelBtn(void* arg)
{
	gUI->SetVisible(false, UIS_FLEET_UI);
}

void FleetUI::OnOKBtn(void* arg)
{
	gUI->SetVisible(false, UIS_FLEET_UI);
}

void FleetUI::PreparePortrait()
{
	if (!mRenderToTexture)
	{
		mRenderToTexture = gEnv->pRenderer->CreateRenderToTexture(true);
		mRenderToTexture->SetEnable(false);
		
		const Vec2I& size = mShipImage->GetSize();
		mRenderToTexture->SetColorTextureDesc(size.x, size.y, PIXEL_FORMAT_R8G8B8A8_UNORM, true, false, false);
		mRenderToTexture->SetDepthStencilDesc(size.x, size.y, PIXEL_FORMAT_D32_FLOAT, false, false);
		ICamera* cam = mRenderToTexture->GetCamera();
		cam->SetNearFar(0.01f, 100.0f);
		float theta = cam->GetFOV()*.5f;
		float tanTheta = tan(theta);
		
		IMeshObject* pMeshObject = gMeshObject;
		assert(pMeshObject);
		mRenderTargetCamDist = pMeshObject->GetBoundingVolume()->GetRadius() / tanTheta + 0.01f;
		mRenderTargetCamDist *= 1.05f;
		cam->SetPos(gMeshObject->GetPos() + mLightDir*mRenderTargetCamDist);
		cam->SetDir(-mLightDir);
		cam->SetTarget(pMeshObject);
		cam->SetEnalbeInput(true);
		cam->SetInitialDistToTarget(mRenderTargetCamDist);
		mRenderToTexture->GetLight()->SetDiffuse(Vec3(1, 1, 1));
		mRenderToTexture->GetLight()->SetPosition(mLightDir);
		mRenderToTexture->GetScene()->AttachObject(pMeshObject);		
		mShipImage->SetTexture(mRenderToTexture->GetRenderTargetTexture());
	}
}

void FleetUI::OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	if (!mVisible || !pMouse || !pMouse->IsValid())
		return;

	bool isin = mShipImage->IsIn(pMouse);
	if (isin)
	{
		mRenderToTexture->OnInputFromHandler(pMouse, pKeyboard);
	}
}

void FleetUI::OnPatrolOptionChanged(void* arg)
{
	size_t index = mPatrolOptions->GetSelectedIndex();
	mPatrol = index;
}
void FleetUI::OnException1Changed(void* arg)
{
	size_t index = mLaunchExceptionDD[0]->GetSelectedIndex();
	mLaunchExceptions[0] = index;
}
void FleetUI::OnException2Changed(void* arg)
{
	size_t index = mLaunchExceptionDD[1]->GetSelectedIndex();
	mLaunchExceptions[1] = index;
}
void FleetUI::OnPopulationPolicyChanged(void* arg)
{
	size_t index = mPopPolicyDD->GetSelectedIndex();
	mPopulationPolicy = index;
}

void FleetUI::RefreshAll()
{
	RefreshModules();
	RefreshShips();
	RefreshExceptionCond();
	RefreshPopulation();
}

void FleetUI::RefreshModules()
{
	DWORD num = 20;
	WCHAR buf[256];
	swprintf_s(buf, L"%u", num);
	mNumModules[0]->SetText(buf);
	
	for (int i = 0; i < 5; i++)
	{
		num = Random(1, 5);
		swprintf_s(buf, L"%u", num);
		mNumModules[i+1]->SetText(buf);
	}	

	// render target camera
	ICamera* cam = mRenderToTexture->GetCamera();
	float theta = cam->GetFOV()*.5f;
	float tanTheta = tan(theta);
	IMeshObject* pMeshObject = gMeshObject;
	assert(pMeshObject);
	mRenderTargetCamDist = pMeshObject->GetBoundingVolume()->GetRadius() / tanTheta + 0.01f;
	mRenderTargetCamDist *= 1.05f;
	cam->SetPos(pMeshObject->GetPos() + mLightDir*mRenderTargetCamDist);
	cam->SetDir(-mLightDir);
}

void FleetUI::RefreshShips()
{
	DWORD num = 10;
	WCHAR buf[256];
	swprintf_s(buf, L"%u", num);
	mNumShips[0]->SetText(buf);
	num = 7;
	swprintf_s(buf, L"%u", num);
	mNumShips[1]->SetText(buf);
	num = 3;
	swprintf_s(buf, L"%u", num);
	mNumShips[2]->SetText(buf);
}

void FleetUI::RefreshExceptionCond()
{
}

void FleetUI::RefreshPopulation()
{
	
	DWORD maxPop = 80000;
	DWORD curPop = 40000;
	WCHAR buf[256];
	swprintf_s(buf, L"%u / %u", curPop, maxPop);
	mPopulation[0]->SetText(buf);

	DWORD children = 15000;
	swprintf_s(buf, L"%u", children);
	mPopulation[1]->SetText(buf);

	DWORD adults = 35000;
	swprintf_s(buf, L"%u", adults);
	mPopulation[2]->SetText(buf);
}