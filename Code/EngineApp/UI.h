#pragma once

namespace fastbird
{
	class IWinBase;
}
enum UIS
{
	UIS_FLEET_UI,

	UIS_COUNT,
};

class CUI;
class FleetUI;


using namespace fastbird;
//-----------------------------------------------------------------------------
struct UIs
{
	UIs();
	~UIs();

	void SetVisible(bool visible, UIS ui);
	bool GetVisible(UIS ui) const;
	void Update(float dt);

	void OnUIFileChanged(const char* file);
	void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);
	void AddExclusives(UIS target, UIS close);
	
	void OnEnterPlayState();
	void OnBackgroundClicked();

	FleetUI* pFleetUI;

	CUI* mUIHolder[UIS_COUNT];

private:
	std::vector<int> mReloadUIs;
	std::vector<UIS> mVisibleHistory;
	fastbird::VectorMap<UIS, std::vector<UIS>> mExclusives;

};


//-----------------------------------------------------------------------------
class CUI
{
public:
	CUI();
	virtual ~CUI(){}

	virtual bool Initialize() = 0;
	virtual void Deinitialize() = 0;
	virtual bool IsInitialized() { return mInitialized; }
	virtual void SetVisible(bool visible);
	virtual bool IsVisible() const { return mVisible; }
	virtual void Update(float dt){}
	virtual const char* GetUIName() const { return mUIName.c_str(); }
	virtual const char* GetUIFilename() const { return mUIFilename.c_str(); }
	virtual bool IsFile(const char* file) const
	{
		return stricmp(file, mUIFilename.c_str()) == 0;
	}
	virtual void OnGameBackgroundClicked() {}
	virtual void OnReload() {}
	virtual IWinBase* GetRootWnd(){ return 0; }

protected:
	bool mVisible;
	bool mInitialized;
	std::string mUIName;
	std::string mUIFilename;

};