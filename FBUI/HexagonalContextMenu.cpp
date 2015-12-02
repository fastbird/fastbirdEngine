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

#include "StdAfx.h"
#include "HexagonalContextMenu.h"
#include "StaticText.h"
#include "ImageBox.h"
#include "UIObject.h"

namespace fb
{
	HexagonalContextMenuPtr HexagonalContextMenu::Create(){
		HexagonalContextMenuPtr p(new HexagonalContextMenu, [](HexagonalContextMenu* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	HexagonalContextMenu::HexagonalContextMenu()
		: mUpdateMaterialParams(true)
		, mMouseInHexaIdx(-1)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize());
		mUIObject->SetMaterial("EssentialEngineData/materials/UIHexagonal.material");
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

		RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
			std::bind(&HexagonalContextMenu::OnMouseHover, this, std::placeholders::_1));
		RegisterEventFunc(UIEvents::EVENT_MOUSE_OUT,
			std::bind(&HexagonalContextMenu::OnMouseOut, this, std::placeholders::_1));

		for (int i = 0; i < 6; i++)
		{
			mHexaEnabled[i] = false;			
		}

		mHexaOrigins[0] = Vec2(0.0f, 0.61f);
		mHexaOrigins[1] = Vec2(0.52f, 0.3f);
		mHexaOrigins[2] = Vec2(0.52f, -0.3f);
		mHexaOrigins[3] = Vec2(0.0f, -0.61f);
		mHexaOrigins[4] = Vec2(-0.52f, -0.3f);
		mHexaOrigins[5] = Vec2(-0.52f, 0.3f);
		
		

		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	}

	HexagonalContextMenu::~HexagonalContextMenu()
	{
	}

	void HexagonalContextMenu::GatherVisit(std::vector<UIObject*>& v)
	{
		if (!mVisibility.IsVisible())
			return;
		/*__super::GatherVisitAlpha(v);*/
		v.push_back(mUIObject.get());
		__super::GatherVisit(v);
	}

	void HexagonalContextMenu::SetNPos(const fb::Vec2& pos) // normalized pos (0.0~1.0)
	{
		__super::SetNPos(pos);
		mUpdateMaterialParams = true;
	}

	void HexagonalContextMenu::SetHexaEnabled(unsigned index, unsigned cmdID /*= -1*/)
	{
		assert(index < 6);
		mHexaEnabled[index] = true;
		mUpdateMaterialParams = true;

		mCmdID[index] = cmdID;
	}

	void HexagonalContextMenu::DisableHexa(unsigned index)
	{
		assert(index < 6);
		mHexaEnabled[index] = false;
		mUpdateMaterialParams = true;

	}

	void HexagonalContextMenu::SetHexaText(unsigned index, const wchar_t* text)
	{
		assert(index < 6);
		if (!mHexaEnabled[index])
			return;
		StaticTextPtr staticText = mHexaStaticTexts[index].lock();
		if (!staticText)
		{
			staticText = std::static_pointer_cast<StaticText>(AddChild(
				mHexaOrigins[index].x*.5f + .5f, mHexaOrigins[index].y*-.5f + .5f, 0.16f, 0.16f, ComponentType::StaticText));
			mHexaStaticTexts[index] = staticText;
			staticText->SetRuntimeChild(true);
		}
		staticText->SetVisible(true);
		staticText->SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
		staticText->SetProperty(UIProperty::TEXT_ALIGN, "center");
		staticText->SetProperty(UIProperty::TEXT_COLOR, "0.86667, 1.0, 0.1843, 1");
		staticText->SetProperty(UIProperty::USE_SCISSOR, "false");
		staticText->SetText(text);
		staticText->DisableEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
	}

	void HexagonalContextMenu::SetHexaImageIcon(unsigned index, const char* atlas, const char* region)
	{
		auto image = std::static_pointer_cast<ImageBox>(AddChild(mHexaOrigins[index].x*.5f + .5f, mHexaOrigins[index].y*-.5f + .5f, 0.2f, 0.2f, ComponentType::ImageBox));
		mHexaImages[index] = image;
		image->SetRuntimeChild(true);
		image->SetProperty(UIProperty::USE_SCISSOR, "false");
	}

	void HexagonalContextMenu::ClearHexa()
	{
		for (int i = 0; i < 6; i++)
		{
			mHexaEnabled[i] = false;
			auto staticText = mHexaStaticTexts[i].lock();
			if (staticText)
			{
				staticText->SetVisible(false);
			}
			auto image = mHexaImages[i].lock();
			if (image)
			{
				image->SetVisible(false);
			}
			
			mCmdID[i] = -1;		
		}
		mUpdateMaterialParams = true;
	}

	void HexagonalContextMenu::OnMouseHover(void* arg)
	{
		SetCursor(WinBase::sCursorOver);
	}
	void HexagonalContextMenu::OnMouseOut(void* arg)
	{

	}

	void HexagonalContextMenu::UpdateMaterialParameters()
	{
		if (mUpdateMaterialParams)
		{
			mUpdateMaterialParams = false;
			auto mat = mUIObject->GetMaterial();
			assert(mat);
			const auto& finalSize = GetFinalSize();
			const auto& finalPos = GetFinalPos();
			auto wnPos = finalPos / Vec2(GetRenderTargetSize());
			auto wnSize = finalSize / Vec2(GetRenderTargetSize());
			Vec4 param[3];
			param[0] = Vec4(mHexaEnabled[0] ? 1.0f : 0.0f, mHexaEnabled[1] ? 1.0f : 0.0f,
				mHexaEnabled[2] ? 1.0f : 0.0f, mHexaEnabled[3] ? 1.0f : 0.0f);
			param[1] = Vec4(mHexaEnabled[4] ? 1.0f : 0.0f, mHexaEnabled[5] ? 1.0f : 0.0f,
				wnSize.x, wnSize.y);
			param[2] = Vec4(wnPos.x, wnPos.y, 0.0f, 0.0f);
			for (int i = 1; i < 4; ++i)
				mat->SetMaterialParameter(i, param[i-1]);
		}
	}

	bool HexagonalContextMenu::IsIn(IInputInjectorPtr injector)
	{
		bool isIn = __super::IsIn(injector);

		mMouseInHexaIdx = -1;
		if (isIn)
		{
			const auto& finalSize = GetFinalSize();
			const auto& finalPos = GetFinalPos();
			auto wnPos = finalPos / Vec2(GetRenderTargetSize());
			auto wnSize = finalSize / Vec2(GetRenderTargetSize());
			Vec2 localMousePos = (Vec2(injector->GetMouseNPos()) - wnPos) / wnSize;
			localMousePos = localMousePos*2.0f - 1.0f;
			localMousePos.y = -localMousePos.y;
			

			for (int i = 0; i<6; ++i)
			{
				if (!mHexaEnabled[i])
					continue;
				// world means just entire Hexagonal Area.
				Vec2 worldMouse = mHexaOrigins[i] - localMousePos;
				worldMouse = Abs(worldMouse);
				float m = std::max(worldMouse.x + worldMouse.y*0.57735f, worldMouse.y*1.1547f) - 0.2f;

				if (m <= 0.1)
				{
					mMouseInHexaIdx = i;
					break;
				}
			}
			return mMouseInHexaIdx != -1;
		}
		return false;
		
	}

	//-----------------------------------------------------------------------
	void HexagonalContextMenu::SetCmdID(unsigned idx, unsigned id)
	{
		if (idx < 6)
		{
			mCmdID[idx] = id;
		}
		else
		{
			assert(0);
		}
	}
	
	//-----------------------------------------------------------------------
	unsigned HexagonalContextMenu::GetCurCmdID()
	{
		if (mMouseInHexaIdx>= 0 && mMouseInHexaIdx < 6)
		{
			return mCmdID[mMouseInHexaIdx];
		}

		return -1;
	}
}