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
#include "UIManager.h"
#include "HorizontalGauge.h"
#include "UIObject.h"
#include "FBRenderer/TextureAtlas.h"

namespace fb
{

	HorizontalGaugePtr HorizontalGauge::Create(){
		HorizontalGaugePtr p(new HorizontalGauge, [](HorizontalGauge* obj){ delete obj; });
		p->mSelfPtr = p;
		return p;
	}

	HorizontalGauge::HorizontalGauge()
		:mPercentage(0), mMaximum(1.f)
		, mBlink(false)
		, mBlinkSpeed(3.f)
		, mBlinkTime(0)
		, mVertical(false)
		, mHorizontalFlip(false)
		, mMaterialUsingImage(false)
		, mDrag(false), mDragging(false)
		, mGaugeColorEmpty(0, 0, 0, 1)
	{
		mUIObject = UIObject::Create(GetRenderTargetSize(), this);
		mUIObject->SetMaterial("EssentialEngineData/materials/UIHorizontalGauge.material");
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

		auto mat = mUIObject->GetMaterial();
		mGaugeColor = Color(1, 1, 1, 1);
		mBlinkColor = Color(1, 1, 0, 1);
		mGaugeBorderColor = Color(0.5f, 0.5f, 0.5f, 0.5f);
		mat->SetShaderParameter(1, mGaugeColor.GetVec4());
		mat->SetShaderParameter(2, mBlinkColor.GetVec4());
		mat->SetAmbientColor(mGaugeBorderColor.GetVec4());

		// x is lerp.		
		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);		
		SetPercentage(0);
	}

	HorizontalGauge::~HorizontalGauge()
	{
	}

	void HorizontalGauge::GatherVisit(std::vector<UIObject*>& v)
	{
		v.push_back(mUIObject.get());
		__super::GatherVisit(v);
	}
	void HorizontalGauge::OnStartUpdate(float elapsedTime)
	{
		__super::OnStartUpdate(elapsedTime);
		if (mBlink && mBlinkTime > 0)
		{
			auto mat = mUIObject->GetMaterial();
			auto v = mat->GetShaderParameter(3);
			v.y = sin(gpTimer->GetTime()*mBlinkSpeed)*.5f + .5f;
			mat->SetShaderParameter(3, v);
			mBlinkTime -= elapsedTime;
			if (mBlinkTime <= 0){
				Blink(false);				
			}
		}
	}

	void HorizontalGauge::OnMouseHover(IInputInjectorPtr injector, bool propergated) {
		__super::OnMouseHover(injector, propergated);
		if (mDrag) {
			SetCursor(WinBase::sCursorWE);
		}
	}

	void HorizontalGauge::OnMouseDrag(IInputInjectorPtr injector) {		
		__super::OnMouseDrag(injector);
		if (!mDrag)
			return;
		Vec2I delta = injector->GetAbsDeltaXY();
		if (delta.x != 0) {
			auto percent = GetPercentage();
			auto maximum = GetFinalSize().x;
			auto cur = maximum * percent;
			cur += delta.x;
			percent = cur / (float)maximum;
			SetPercentage(percent);
		}
	}

	/*bool HorizontalGauge::OnInputFromHandler(IInputInjectorPtr injector) {
		if (mDragging && IsIn(injector)) {
			Vec2I delta = injector->GetAbsDeltaXY();
			if (delta.x != 0) {
				auto dragPart = mDragPart.lock();
				auto startX = GetFinalPos().x;
				auto endX = startX + GetFinalSize().x;
				Vec2I mousePos = injector->GetMousePos();
				auto destX = mousePos.x - startX;
				destX = std::min(startX + 1, destX);
				destX = std::max(endX - 1, destX);
				auto posRelative = destX - startX;
				if (dragPart) {
					dragPart->ChangePosX(posRelative);
				}
				SetPercentage(posRelative / GetFinalSize().x);
			}
			return true;
		}
		return __super::OnInputFromHandler(injector);
	}*/

	bool HorizontalGauge::UsingEmptyColor() {
		return mGaugeColorEmpty.r() + mGaugeColorEmpty.g() + mGaugeColorEmpty.b() + mGaugeColorEmpty.a() != 0.f;
	}

	void HorizontalGauge::SetPercentage(float p)
	{
		mPercentage = std::min(mMaximum, p);
		mPercentage = std::max(0.f, mPercentage);

		auto dragPart = mDragPart.lock();
		if (dragPart) {
			dragPart->ChangeNPosX(mPercentage);
		}
		OnEvent(UIEvents::EVENT_GAGUE_CHANGED);
		
		auto mat = mUIObject->GetMaterial();
		// x : percent		
		// y : sin
		// z : ratio
		Vec4 val = mat->GetShaderParameter(3);
		val.x = mPercentage / mMaximum;				
		mat->SetShaderParameter(3, val);

		if (UsingEmptyColor())
		{
			Color c= Lerp(mGaugeColorEmpty, mGaugeColor, p);
			mat->SetShaderParameter(1, c.GetVec4());
		}
		else {			
			mat->SetShaderParameter(1, mGaugeColor.GetVec4());
		}
	}

	void HorizontalGauge::SetMaximum(float m)
	{
		mMaximum = m;
		auto mat = mUIObject->GetMaterial();
		
		Vec4 val = mat->GetShaderParameter(3);
		val.x = mPercentage / mMaximum;		
		mat->SetShaderParameter(3, val);
	}

	void HorizontalGauge::Blink(bool blink)
	{
		if (mBlink == blink)
			return;

		mBlink = blink;
		if (!blink){
			auto mat = mUIObject->GetMaterial();
			mat->SetShaderParameter(2, Vec4(0, 0, 0, 0));
			mBlinkTime = 0;
		}
		else{
			mBlinkTime = FLT_MAX;
		}
	}

	void HorizontalGauge::Blink(bool blink, float time){

		if (mBlink == blink)
			return;

		mBlink = blink;
		mBlinkTime = time;

		if (!blink)
		{
			auto mat = mUIObject->GetMaterial();
			mat->SetShaderParameter(2, Vec4(0, 0, 0, 0));
			mBlinkTime = 0;
		}

	}

	void HorizontalGauge::SetGaugeColor(const Color& color)
	{
		mGaugeColor = color;
		SetPercentage(mPercentage);
	}

	const Color& HorizontalGauge::GetGaugeColor() const
	{
		return mGaugeColor;
	}

	void HorizontalGauge::SetGaugeColorEmpty(const Color& color)
	{
		if (mGaugeBorderColor.r() != mGaugeBorderColor.r()) {
			int a = 0;
			a++;
		}
		if (mGaugeBorderColor.g() != mGaugeBorderColor.g()) {
			int a = 0;
			a++;
		}
		mGaugeColorEmpty = color;
		SetPercentage(mPercentage);
	}

	void HorizontalGauge::SetBlinkColor(const Color& color)
	{
		mBlinkColor = color;
		auto mat = mUIObject->GetMaterial();
		mat->SetShaderParameter(2, color.GetVec4());
	}

	const Color& HorizontalGauge::GetBlinkColor() const
	{
		return mBlinkColor;
	}

	bool HorizontalGauge::SetProperty(UIProperty::Enum prop, const char* val)
	{

		switch (prop)
		{
		case UIProperty::GAUGE_MAX:
		{
			SetMaximum(StringConverter::ParseReal(val));
			return true;
		}
		case UIProperty::GAUGE_CUR:
		{
			SetPercentage(StringConverter::ParseReal(val));
			return true;
		}
		case UIProperty::GAUGE_COLOR:
		{
			SetGaugeColor(StringMathConverter::ParseColor(val));
			return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
			SetGaugeColorEmpty(StringMathConverter::ParseColor(val));			
			return true;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
			SetBlinkColor(StringMathConverter::ParseColor(val));
			return true;
		}

		case UIProperty::GAUGE_BLINK_SPEED:
		{
			mBlinkSpeed = StringConverter::ParseReal(val);
			return true;
		}

		case UIProperty::GAUGE_BORDER_COLOR:
		{
			mGaugeBorderColor = StringMathConverter::ParseColor(val);
			if (mUIObject)
			{
				auto mat = mUIObject->GetMaterial();
				if (mat)
				{
					mat->SetAmbientColor(mGaugeBorderColor.GetVec4());
				}
			}
			return true;
		}

		case UIProperty::Gauge_DRAG: {
			SetDrag(StringConverter::ParseBool(val));
			
			return true;
		}

		case UIProperty::REGION_FILLED:
		{
			mRegionFilled = val;
			SetTextureAtlasRegion(prop, val);
			return true;
		}
		case UIProperty::REGION_NOT_FILLED:
		{
			mRegionNotFilled = val;
			SetTextureAtlasRegion(prop, val);
			return true;
		}

		case UIProperty::IMAGE_HFLIP:
		{
			mHorizontalFlip = StringConverter::ParseBool(val);
			return true;
		}

		case UIProperty::TEXTUREATLAS:
		{
			mTextureAtlasFile = val;
			return true;
		}
		break;

		case UIProperty::GAUGE_VERTICAL:
		{
			mVertical = StringConverter::ParseBool(val);
			if (mVertical)
				mUIObject->GetMaterial()->AddShaderDefine("_VERTICAL", "1");
			else
				mUIObject->GetMaterial()->RemoveShaderDefine("_VERTICAL");
			return true;
		}
		}

		return __super::SetProperty(prop, val);
	}

	bool HorizontalGauge::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
	{
		switch (prop)
		{
		case UIProperty::GAUGE_MAX:
		{
			if (notDefaultOnly)
			{
				if (mMaximum == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mMaximum);
			strcpy_s(val, bufsize, data.c_str());			
			return true;
		}
		case UIProperty::GAUGE_CUR:
		{
			if (notDefaultOnly)
			{
				if (mPercentage == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mPercentage);
			strcpy_s(val, bufsize, data.c_str());
			return true;
			
		}
		case UIProperty::GAUGE_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mGaugeColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringMathConverter::ToString(mGaugeColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
			if (UsingEmptyColor())
			{
				auto data = StringMathConverter::ToString(mGaugeColorEmpty);
				strcpy_s(val, bufsize, data.c_str());
				return true;
			}	
			strcpy_s(val, bufsize, "0 0 0 0");
			return true;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mBlinkColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringMathConverter::ToString(mBlinkColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::GAUGE_BLINK_SPEED:
		{
			if (notDefaultOnly)
			{
				if (mBlinkSpeed == UIProperty::GetDefaultValueFloat(prop))
					return false;
			}
			auto data = StringConverter::ToString(mBlinkSpeed);
			strcpy_s(val, bufsize, data.c_str());
			return true;			
		}

		case UIProperty::GAUGE_BORDER_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mGaugeBorderColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringMathConverter::ToString(mGaugeBorderColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::Gauge_DRAG: {
			if (notDefaultOnly) {
				if (mDrag == UIProperty::GetDefaultValueBool(prop)) {
					return false;
				}
			}
			strcpy_s(val, bufsize, StringConverter::ToString(mDrag).c_str());
			return true;
		}

		case UIProperty::GAUGE_VERTICAL:
		{
			if (notDefaultOnly)
			{
				if (mVertical == UIProperty::GetDefaultValueBool(prop))
					return false;
			}
			auto data = StringConverter::ToString(mVertical);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		case UIProperty::TEXTUREATLAS:
		{
			if (notDefaultOnly)
			{
				if (mTextureAtlasFile.empty())
					return false;
			}
			strcpy_s(val, bufsize, mTextureAtlasFile.c_str());
			return true;
		}
		break;

		case UIProperty::REGION_FILLED:
		{
			if (notDefaultOnly)
			{
				if (mRegionFilled.empty())
					return false;
			}
			strcpy_s(val, bufsize, mRegionFilled.c_str());
			return true;
		}
		case UIProperty::REGION_NOT_FILLED:
		{
			if (notDefaultOnly)
			{
				if (mRegionNotFilled.empty())
					return false;
			}
			strcpy_s(val, bufsize, mRegionNotFilled.c_str());
			return true;
		}
		break;

		case UIProperty::IMAGE_HFLIP:
		{
			if (notDefaultOnly)
			{
				if (mHorizontalFlip == UIProperty::GetDefaultValueBool(prop))
					return false;
			}
			auto data = StringConverter::ToString(mHorizontalFlip);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}

		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}

	void HorizontalGauge::SetTextureAtlasRegion(UIProperty::Enum prop, const char* region)
	{
		if (mTextureAtlasFile.empty())
		{
			mTextureAtlasFile = "Data/textures/gameui.xml";
		}
		int index = 0;
		if (prop == UIProperty::REGION_FILLED)
		{
			index = 0;
		}
		else if (prop == UIProperty::REGION_NOT_FILLED)
		{
			index = 1;
		}
		else
		{
			assert(0);
		}
		if (index == 0 && strlen(region) == 0 && mMaterialUsingImage) {
			mMaterialUsingImage = false;
			ChangeMaterial("EssentialEngineData/materials/UIHorizontalGauge.material");			
			mRegionFilled.clear();
			mRegionNotFilled.clear();
			return;
		}

		mTextureAtlas = Renderer::GetInstance().GetTextureAtlas(mTextureAtlasFile.c_str());
		if (mTextureAtlas)
		{
			if (!mMaterialUsingImage)
			{
				mMaterialUsingImage = true;
				ChangeMaterial("EssentialEngineData/materials/UIGaugeImage.material");				
			}
			mTextures[index] = mTextureAtlas->GetTexture();
			mAtlasRegions[index] = mTextureAtlas->GetRegion(region);
			if (!mAtlasRegions[index])
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Cannot find the region %s in the atlas %s", 
					region, mTextureAtlasFile.c_str()).c_str());
			}
			SAMPLER_DESC sdesc;
			sdesc.SetFilter(TEXTURE_FILTER_MIN_MAG_MIP_POINT);			
			auto mat = mUIObject->GetMaterial();
			mat->SetTexture(mTextures[index], SHADER_TYPE_PS, index, sdesc);
			if (mAtlasRegions[index])
			{
				Vec2 texcoords[4];
				mAtlasRegions[index]->GetQuadUV(texcoords);
				if (mHorizontalFlip)
				{
					std::swap(texcoords[0], texcoords[2]);
					std::swap(texcoords[1], texcoords[3]);
				}
				if (index == 0)
					mUIObject->SetTexCoord(texcoords, 4);
				else if (index == 1)
					mUIObject->SetTexCoord(texcoords, 4, 1);
				float minY = texcoords[1].y;
				float maxY = texcoords[2].y;
				auto v = mat->GetShaderParameter(3);
				v.z = minY;
				v.w = maxY;
				mat->SetShaderParameter(3, v);
				DWORD colors[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
				mUIObject->SetColors(colors, 4);				
			}
		}
	}

	void HorizontalGauge::ChangeMaterial(const char* materialPath) {
		if (!mUIObject || !mUIObject->GetMaterial()) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid timing.");
			return;
		}

		Material::Parameters parameters = mUIObject->GetMaterial()->GetShaderParameters();
		mUIObject->SetMaterial(materialPath);
		auto newMat = mUIObject->GetMaterial();
		if (newMat) {
			for (auto& it : parameters) {
				newMat->SetShaderParameter(it.first, it.second);
			}
		}
	}

	void HorizontalGauge::SetDrag(bool drag) {
		mDrag = drag;
		auto dragPart = mDragPart.lock();
		if (mDrag && !dragPart) {
			dragPart = AddChild(mPercentage, 0.5f, 0.01f, 0.9f, ComponentType::StaticText);
			dragPart->SetProperty(UIProperty::USE_NPOSX, "true");
			dragPart->SetProperty(UIProperty::USE_NPOSY, "true");
			dragPart->SetProperty(UIProperty::NO_BACKGROUND, "false");			
			dragPart->SetProperty(UIProperty::BACK_COLOR, "1, 1, 0, 1");
			dragPart->SetProperty(UIProperty::ALIGNH, "center");
			dragPart->SetProperty(UIProperty::ALIGNV, "middle");
			dragPart->ChangeSizeX(2);
			if (GetVisible()) {
				dragPart->SetVisible(true);
			}
			mDragPart = dragPart;
		}
		else if (!mDrag && dragPart) {
			RemoveChild(dragPart);
		}
	}
}