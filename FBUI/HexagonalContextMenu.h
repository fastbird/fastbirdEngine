/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once

#include "Container.h"
namespace fb
{
FB_DECLARE_SMART_PTR(StaticText);
FB_DECLARE_SMART_PTR(ImageBox);
FB_DECLARE_SMART_PTR(HexagonalContextMenu);
class FB_DLL_UI HexagonalContextMenu : public Container
{
protected:
	HexagonalContextMenu();
	~HexagonalContextMenu();


public:
	static HexagonalContextMenuPtr Create();	

	// IWinBase
	ComponentType::Enum GetType() const { return ComponentType::Hexagonal; }
	void GatherVisit(std::vector<UIObject*>& v);
	void SetNPos(const fb::Vec2& pos); // normalized pos (0.0~1.0)
	bool IsIn(IInputInjectorPtr injector);

	//own
	// index : 0~5
	void SetHexaEnabled(unsigned index, unsigned cmdID=-1);
	void DisableHexa(unsigned idx);
	void SetHexaText(unsigned index, const wchar_t* text);
	void SetHexaImageIcon(unsigned index, const char* atlas, const char* region);
	void ClearHexa();
	void UpdateMaterialParameters();
	int GetCurHexaIdx() const { return mMouseInHexaIdx; }
	void SetCmdID(unsigned idx, unsigned id);
	unsigned GetCmdID(unsigned idx) { return mCmdID[idx]; }
	unsigned GetCurCmdID();

	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);

private:
	Vec2 mHexaOrigins[6];
	bool mHexaEnabled[6];
	ImageBoxWeakPtr mHexaImages[6];
	StaticTextWeakPtr mHexaStaticTexts[6];
	unsigned mCmdID[6];
	bool mUpdateMaterialParams;
	int mMouseInHexaIdx;
};
}