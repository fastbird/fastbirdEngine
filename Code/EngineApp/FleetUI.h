#pragma once
#include <EngineApp/UI.h>

namespace fastbird
{
	class Wnd;
	class Button;
	class CheckBox;
	class ImageBox;
	class StaticText;
	class DropDown;
	class IWinBase;
	class IRenderToTexture;
}

class FleetUI : public CUI
{
public:
	FleetUI();
	virtual ~FleetUI();

	virtual bool Initialize();
	virtual void Deinitialize();
	virtual void SetVisible(bool visible);
	virtual void Update(float dt);
	virtual fastbird::IWinBase* GetRootWnd() {
		return (fastbird::IWinBase*)mWnd;
	}
	void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);

	void OnRepair(void* arg);
	void OnAutoRepair(void* arg);
	void OnOKBtn(void* arg);
	void OnCancelBtn(void* arg);

	void OnPatrolOptionChanged(void* arg);
	void OnException1Changed(void* arg);
	void OnException2Changed(void* arg);
	void OnPopulationPolicyChanged(void* arg);

	void RefreshAll();
	void RefreshModules();
	void RefreshShips();
	void RefreshExceptionCond();
	void RefreshPopulation();

	void PreparePortrait();

private:
	fastbird::Wnd* mWnd;
	fastbird::ImageBox* mShipImage;
	fastbird::StaticText* mNumModules[6];
	fastbird::StaticText* mNumShips[4];
	fastbird::DropDown* mPatrolOptions;
	fastbird::DropDown* mLaunchExceptionDD[2];

	fastbird::StaticText* mPopulation[3];
	fastbird::DropDown* mPopPolicyDD;

	fastbird::CheckBox* mAutoRepairCB;

	fastbird::Button* mRepair;
	fastbird::Button* mOKBtn;
	fastbird::Button* mCancelBtn;

	fastbird::IRenderToTexture* mRenderToTexture;
	float mRenderTargetCamDist;
	fastbird::Vec3 mLightDir;

	// data
	size_t mPatrol;
	size_t mLaunchExceptions[2];
	size_t mPopulationPolicy;
	bool mAutoRepair;
};