#pragma once

#include <UI/Container.h>
namespace fastbird
{
class ImageBox;
class StaticText;
class HexagonalContextMenu : public Container
{
public:
	HexagonalContextMenu();
	~HexagonalContextMenu();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Hexagonal; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void SetNPos(const fastbird::Vec2& pos); // normalized pos (0.0~1.0)
	virtual bool IsIn(const Vec2& mouseNormpos);

	//own
	// index : 0~5
	virtual void SetHexaEnabled(unsigned index, unsigned cmdID=-1);
	virtual void DisableHexa(unsigned idx);
	virtual void SetHexaText(unsigned index, const wchar_t* text);
	virtual void SetHexaImageIcon(unsigned index, const char* atlas, const char* region);
	virtual void ClearHexa();
	virtual void UpdateMaterialParameters();
	virtual int GetCurHexaIdx() const { return mMouseInHexaIdx; }
	virtual void SetCmdID(unsigned idx, unsigned id);
	virtual unsigned GetCmdID(unsigned idx) { return mCmdID[idx]; }
	virtual unsigned GetCurCmdID();

	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);

private:
	Vec2 mHexaOrigins[6];
	bool mHexaEnabled[6];
	ImageBox* mHexaImages[6];
	StaticText* mHexaStaticTexts[6];
	unsigned mCmdID[6];
	bool mUpdateMaterialParams;
	int mMouseInHexaIdx;
};
}