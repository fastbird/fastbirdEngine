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
#include "ImageBox.h"
#include "UIManager.h"
#include "UIObject.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/Camera.h"
#include "FBRenderer/TextureAtlas.h"
#include "FBRenderer/Texture.h"
namespace fb
{

ImageBoxPtr ImageBox::Create(){
	ImageBoxPtr p(new ImageBox, [](ImageBox* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

ImageBox::ImageBox()
	: mTextureAtlas(0)
	, mAtlasRegion(0)
	, mUseHighlight(false)
	, mKeepImageRatio(true)
	, mFrameImage(0)
	, mAnimation(false)
	, mSecPerFrame(0)
	, mPlayingTime(0)
	, mCurFrame(0), mImageFixedSize(false)
	, mTexture(0), mColorOveraySet(false)
	, mRenderTarget(0)
	, mImageRot(false)
	, mLinearSampler(false)
{
	mUIObject = UIObject::Create(GetRenderTargetSize());
	mUIObject->SetMaterial("EssentialEngineData/materials/UIImageBox.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ImageBox::~ImageBox()
{
}

void ImageBox::OnCreated()
{
	SetProperty(UIProperty::TEXTUREATLAS, "data/textures/gameui.xml");
}

void ImageBox::OnResolutionChanged(HWindowId hwndId){
	__super::OnResolutionChanged(hwndId);
	OnAnySizeChanged();
}

void ImageBox::CalcUV(const Vec2I& textureSize)
{
	if (mImageFixedSize)
	{
		DrawAsFixedSize();
		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
		return;
	}

	float width = (float)textureSize.x;
	float height = (float)textureSize.y;
	float imgRatio = width / height;
	const Rect& uiRect = mUIObject->GetRegion();
	float uiRatio = (uiRect.right - uiRect.left) /
		(float)(uiRect.bottom - uiRect.top);
	if (uiRatio == imgRatio || !mKeepImageRatio)
	{
		Vec2 texcoords[4] = {
			Vec2(0.f, 1.f),
			Vec2(0.f, 0.f),
			Vec2(1.f, 1.f),
			Vec2(1.f, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	}
	else
	{
		float halfu = (uiRatio / imgRatio) * .5f;
		Vec2 texcoords[4] = {
			Vec2(0.5f - halfu, 1.f),
			Vec2(0.5f - halfu, 0.f),
			Vec2(0.5f + halfu, 1.f),
			Vec2(0.5f + halfu, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	}
}

void ImageBox::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);
	if (!mAnimation || !mVisibility.IsVisible())
		return;

	mPlayingTime += elapsedTime;
	if (!mAtlasRegions.empty() && mPlayingTime > mSecPerFrame)
	{
		mCurFrame++;
		if (mCurFrame >= mAtlasRegions.size())
			mCurFrame = 0;
		Vec2 texcoords[4];
		mAtlasRegions[mCurFrame]->GetQuadUV(texcoords);
		mUIObject->SetTexCoord(texcoords, 4);
		mPlayingTime -= mSecPerFrame;
	}
}

bool ImageBox::OnInputFromHandler(IInputInjectorPtr injector){
	auto ret = __super::OnInputFromHandler(injector);
	if (ret && mRenderTarget && injector->IsValid(InputDevice::Mouse)){
		mRenderTarget->ConsumeInput(injector);
	}
	return ret;
}

void ImageBox::SetTexture(const char* file)
{
	if (!file)
		return;
	if (mImageFile == file)
		return;

	mImageFile = file ? file : "";

	if (mImageFile.empty()){
		SetTexture(TexturePtr());
	}
	else{
		TexturePtr pTexture = Renderer::GetInstance().CreateTexture(mImageFile.c_str());
		SetTexture(pTexture);
	}
	
	UIManager::GetInstance().DirtyRenderList(GetHwndId());
}

void ImageBox::SetTexture(TexturePtr pTexture)
{
	//mImageFile.clear();
	if (!pTexture || strlen(pTexture->GetFilePath())==0){
		mImageFile.clear();
	}
	mTexture = pTexture;
	SAMPLER_DESC sd;
	sd.AddressU = TEXTURE_ADDRESS_BORDER;
	sd.AddressV = TEXTURE_ADDRESS_BORDER;
	sd.AddressW = TEXTURE_ADDRESS_BORDER;
	mUIObject->GetMaterial()->SetTexture(pTexture, BINDING_SHADER_PS, 0, sd);
	if (pTexture)
		CalcUV(pTexture->GetSize());
}

void ImageBox::SetRenderTargetTexture(RenderTargetPtr rt){
	mRenderTarget = rt;
	SetTexture(mRenderTarget->GetRenderTargetTexture());
}

const Vec2I& ImageBox::SetTextureAtlasRegion(const char* atlas, const char* region)
{
	mTextureAtlas = Renderer::GetInstance().GetTextureAtlas(atlas);
	if (mTextureAtlas)
	{
		if (mImageFixedSize)
		{
			DrawAsFixedSize();
		}
		SAMPLER_DESC sdesc;
		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		TriggerRedraw();

		mAtlasRegion = mTextureAtlas->GetRegion(region);
		if (!mAtlasRegion)
		{
			mTexture = 0;
			mUIObject->GetMaterial()->SetTexture(TexturePtr(), BINDING_SHADER_PS, 0, sdesc);
			mUIObject->ClearTexCoord();
			Error("Cannot find the region %s in the atlas %s", region, atlas);
			return Vec2I::ZERO;
		}
		// need to set to material. matarial will hold its reference counter
		mTexture = mTextureAtlas->GetTexture();
		mUIObject->GetMaterial()->SetTexture(mTexture, BINDING_SHADER_PS, 0, sdesc);
		Vec2 texcoords[4];
		mAtlasRegion->GetQuadUV(texcoords);
		mUIObject->SetTexCoord(texcoords, 4);
		DWORD colors[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
		mUIObject->SetColors(colors, 4);
		mAtlasRegions.clear();
		return mAtlasRegion->GetSize();		
	}
	return Vec2I::ZERO;
}

void ImageBox::SetTextureAtlasRegions(const char* atlas, const std::vector<std::string>& data)
{
	mTextureAtlas = Renderer::GetInstance().GetTextureAtlas(atlas);
	if (mTextureAtlas)
	{
		mTexture = mTextureAtlas->GetTexture();
		if (mImageFixedSize)
		{
			DrawAsFixedSize();
		}
		for (const auto& region : data)
		{
			mAtlasRegions.push_back(mTextureAtlas->GetRegion(region.c_str()));
		}
		SAMPLER_DESC sdesc;
		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		mUIObject->GetMaterial()->SetTexture(mTexture, BINDING_SHADER_PS, 0, sdesc);

		if (!mAtlasRegions.empty())
		{
			Vec2 texcoords[4];
			mAtlasRegions[0]->GetQuadUV(texcoords);
			mUIObject->SetTexCoord(texcoords, 4);
			DWORD colors[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
			mUIObject->SetColors(colors, 4);
		}
	}
}

void ImageBox::ChangeRegion(TextureAtlasRegionPtr region)
{
	mAtlasRegion = region;
	if (mAtlasRegion)
	{
		Vec2 texcoords[4];
		mAtlasRegion->GetQuadUV(texcoords);
		mUIObject->SetTexCoord(texcoords, 4);
	}	
}

void ImageBox::ChangeRegion(const char* region)
{
	if (!mTextureAtlas)
		mTextureAtlas = Renderer::GetInstance().GetTextureAtlas(mTextureAtlasFile.c_str());

	if (mTextureAtlas)
	{
		mAtlasRegion = mTextureAtlas->GetRegion(region);
		if (mAtlasRegion)
		{
			Vec2 texcoords[4];
			mAtlasRegion->GetQuadUV(texcoords);
			mUIObject->SetTexCoord(texcoords, 4);
		}
	}

}

void ImageBox::GatherVisit(std::vector<UIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;

	if (!mTexture && mImageFile.empty())
		return;
	v.push_back(mUIObject.get());
	__super::GatherVisit(v);
}

void ImageBox::OnSizeChanged()
{
	__super::OnSizeChanged();
	OnAnySizeChanged();
}

void ImageBox::OnParentSizeChanged(){
	__super::OnParentSizeChanged();
	OnAnySizeChanged();
}

void ImageBox::OnAnySizeChanged(){
	if (!mAtlasRegion && mAtlasRegions.empty())
	{
		auto texture = mUIObject->GetMaterial()->GetTexture(BINDING_SHADER_PS, 0);
		if (texture)
			CalcUV(texture->GetSize());
	}
	else{
		if (mAtlasRegion)
		{
			const auto& imagesize = mAtlasRegion->GetSize();
			/*if (imagesize.x > mSize.x * 2 || imagesize.y > mSize.y * 2){
				Vec2 start((float)mAtlasRegion->mStart.x, (float)mAtlasRegion->mStart.y);
				auto xsize = imagesize.x > mSize.x * 2 ? mSize.x : imagesize.x;
				auto ysize = imagesize.y > mSize.y * 2 ? mSize.y : imagesize.y;
				Vec2 end(start.x + xsize, start.y + ysize);
				auto uvEnd = end / mTextureAtlas->mTexture->GetSize();
				Vec2 texcoords[4];
				texcoords[0] = Vec2(mAtlasRegion->mUVStart.x, uvEnd.y);
				texcoords[1] = mAtlasRegion->mUVStart;
				texcoords[2] = uvEnd;
				texcoords[3] = Vec2(uvEnd.x, mAtlasRegion->mUVStart.y);
				mUIObject->SetTexCoord(texcoords, 4);
			}
			else{*/
				Vec2 texcoords[4];
				mAtlasRegion->GetQuadUV(texcoords);
				mUIObject->SetTexCoord(texcoords, 4);
			//}

		}
	}
}

void ImageBox::Highlight(bool enable)
{
	if (!mUseHighlight)
		return;

	if (enable)
	{
		mUIObject->GetMaterial()->SetEmissiveColor(0.15f, 0.95f, 0.9f, 1);
	}
	else
	{
		mUIObject->GetMaterial()->SetEmissiveColor(0, 0, 0, 0);
	}
}

bool ImageBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::TEXTUREATLAS:
	{
									 mTextureAtlasFile = val;
									 return true;
	}
		break;

	case UIProperty::REGION:
	{
		mStrRegion = val;
		if (mTextureAtlasFile.empty()){
			mTextureAtlasFile = "data/textures/gameui.xml";
		}
		SetTextureAtlasRegion(mTextureAtlasFile.c_str(), val);
		return true;
	}
		break;

	case UIProperty::REGIONS:
	{
		mStrRegions = val;
		if (mTextureAtlasFile.empty()){
			mTextureAtlasFile = "data/textures/gameui.xml";
		}
		mAnimation = true;
		auto useNumberData = Split(val, ":");
		if (useNumberData.size() >= 2)
		{
			auto fromtoData = Split(useNumberData[1], ",");
			fromtoData[0] = StripBoth(fromtoData[0].c_str());
			fromtoData[1] = StripBoth(fromtoData[1].c_str());
			unsigned from = StringConverter::ParseUnsignedInt(fromtoData[0].c_str());
			unsigned to = StringConverter::ParseUnsignedInt(fromtoData[1].c_str());
			assert(to > from);
			std::vector<std::string> data;
			char buf[256];
			for (unsigned i = from; i <= to; i++)
			{
				sprintf_s(buf, "%s%u", useNumberData[0].c_str(), i);
				data.push_back(buf);
			}
			SetTextureAtlasRegions(mTextureAtlasFile.c_str(), data);
		}
		else
		{
			auto data = Split(val, ",");
			for (auto& str : data)
			{
				str = StripBoth(str.c_str());
			}
			SetTextureAtlasRegions(mTextureAtlasFile.c_str(), data);
		}
		if (!mAtlasRegions.empty())
		{
			Vec2 texcoords[4];
			mAtlasRegions[mCurFrame]->GetQuadUV(texcoords);
			mUIObject->SetTexCoord(texcoords, 4);
		}
								
		return true;

	}
		break;

	case UIProperty::FPS:
	{
							mSecPerFrame = 1.0f / StringConverter::ParseReal(val);
							return true;
	}
		break;

	case UIProperty::TEXTURE_FILE:
	{
									 SetTexture(val);
									 return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
										 SetKeepImageRatio(StringConverter::ParseBool(val, true));
										 return true;
	}
	case UIProperty::FRAME_IMAGE:
	{
		mStrFrameImage = val;
									if (!mFrameImage)
									{
										mFrameImage = CreateChildImageBox();
									}
									mFrameImage->SetTextureAtlasRegion(mTextureAtlasFile.c_str(), val);
									if (strlen(val) == 0)
									{
										mFrameImage->SetVisible(false);
									}
									else
									{
										mFrameImage->SetVisible(true);
									}
									return true;
	}

	case UIProperty::IMAGE_COLOR_OVERLAY:
	{
		mColorOveraySet = true;
											Color color = Color(val);
											if (mUIObject)
											{
												mUIObject->GetMaterial()->SetDiffuseColor(color.GetVec4());
											}
											return true;

	}

	case UIProperty::IMAGE_FIXED_SIZE:
	{
										 mImageFixedSize = StringConverter::ParseBool(val);										 
										 if (mTexture || mAtlasRegion)
										 {
											 DrawAsFixedSize();
										 }
										 
										 return true;

	}

	case UIProperty::IMAGE_ROTATE:
	{
		SetUVRot(mImageRot);		
		return true;

	}

	case UIProperty::IMAGE_LINEAR_SAMPLER:
	{
		mLinearSampler = StringConverter::ParseBool(val);
		if (mUIObject){
			mUIObject->EnableLinearSampler(mLinearSampler);
		}
		return true;
	}

	}

	return __super::SetProperty(prop, val);
}

bool ImageBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::TEXTUREATLAS:
	{
		if (notDefaultOnly){
			if (mTextureAtlasFile.empty())
				return false;
		}
		strcpy_s(val, bufsize, mTextureAtlasFile.c_str());
		return true;
	}

	case UIProperty::REGION:
	{
		if (notDefaultOnly){
			if (mStrRegion.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrRegion.c_str());
		return true;
	}

	case UIProperty::REGIONS:
	{
		if (notDefaultOnly){
			if (mStrRegions.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrRegions.c_str());
		return true;
	}
	break;

	case UIProperty::FPS:
	{
		if (notDefaultOnly)	{
			if (mSecPerFrame == 0.f)
				return false;

		}
		auto data = StringConverter::ToString(mSecPerFrame==0.f ? 0.f : 1.f / mSecPerFrame);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	break;

	case UIProperty::TEXTURE_FILE:
	{
		if (notDefaultOnly)
		{
			if (mImageFile.empty())
				return false;
		}
		strcpy_s(val, bufsize, mImageFile.c_str());
		return true;
	}

	case UIProperty::KEEP_IMAGE_RATIO:
	{
		if (notDefaultOnly)
		{
			if (mKeepImageRatio == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::ToString(mKeepImageRatio);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::FRAME_IMAGE:
	{
		if (notDefaultOnly)
		{
			if (mStrFrameImage.empty())
				return false;
		}
		strcpy_s(val, bufsize, mStrFrameImage.c_str());
		return true;
	}

	case UIProperty::IMAGE_COLOR_OVERLAY:
	{
		if (notDefaultOnly)
		{
			if (!mColorOveraySet)
				return false;
		}
		if (!mUIObject)
			return false;
		auto data = StringMathConverter::ToString(mUIObject->GetMaterial()->GetDiffuseColor());
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::IMAGE_FIXED_SIZE:
	{
		if (notDefaultOnly)
		{
			if (mImageFixedSize == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::ToString(mImageFixedSize);
		strcpy_s(val, bufsize, data.c_str());
		return true;

	}
	case UIProperty::IMAGE_ROTATE:
	{
		if (notDefaultOnly)
		{
			if (mImageRot == UIProperty::GetDefaultValueBool(prop))
				return false;
		}

		auto data = StringConverter::ToString(mImageRot);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	case UIProperty::IMAGE_LINEAR_SAMPLER:
	{
		if (notDefaultOnly){
			if (mLinearSampler == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy_s(val, bufsize, StringConverter::ToString(mLinearSampler).c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void ImageBox::SetVisibleInternal(bool visible){
	__super::SetVisibleInternal(visible);
	if (mRenderTarget){
		mRenderTarget->SetEnable(visible);
	}
}

void ImageBox::SetKeepImageRatio(bool keep)
{
	mKeepImageRatio = keep;
	if (!mAtlasRegion)
	{
		auto texture = mUIObject->GetMaterial()->GetTexture(BINDING_SHADER_PS, 0);
		if (texture)
			CalcUV(texture->GetSize());
	}
}

ImageBoxPtr ImageBox::CreateChildImageBox()
{
	auto image = std::static_pointer_cast<ImageBox>(AddChild(0, 0, 1.0f, 1.0f, ComponentType::ImageBox));	
	image->SetRuntimeChild(true);
	return image;
}

bool ImageBox::IsAnimated() const
{
	return !mAtlasRegions.empty();
}

void ImageBox::SetUVRot(bool set)
{
	auto mat = mUIObject->GetMaterial();
	if (mat)
	{
		if (set)
		{
			mat->AddShaderDefine("_UV_ROT", "1");
		}
		else
		{
			mat->RemoveShaderDefine("_UV_ROT");
		}
		//mat->ApplyShaderDefines();
	}
}

void ImageBox::SetCenterUVMatParam()
{
	if (mAtlasRegion)
	{
		Vec2 uv[4];
		mAtlasRegion->GetQuadUV(uv);
		auto center = (uv[1] + uv[2]) * .5f;
		auto mat = mUIObject->GetMaterial();
		if_assert_pass(mat)
		{
			mat->SetMaterialParameter(0, Vec4(center.x, center.y, 0, 0));
		}

	}
	else
	{
		assert(0 && "Not Implemented");
	}
}

void ImageBox::DrawAsFixedSizeCenteredAt(const Vec2I& wpos)
{
	Vec2I isize(0, 0);
	if (mAtlasRegion)
	{	
		isize = mAtlasRegion->GetSize();
	}
	else if (!mAtlasRegions.empty())
	{
		isize = mAtlasRegions[mCurFrame]->GetSize();
	}
	else if (mTexture)
	{
		isize = mTexture->GetSize();		
	}
	else
	{
		Log("ImageBox %s : DrawAsFixedSizeCenteredAt, you didn't set the texture", mName.c_str());
		return;
	}
	ChangeSize(isize);
	ChangeWPos(wpos);
	SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
	
}

void ImageBox::DrawAsFixedSizeAtCenter()
{
	DrawAsFixedSize();
	ChangeNPos(Vec2(0.5f, 0.5f));
	SetUseAbsPos(false);
	SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
}

void ImageBox::DrawAsFixedSize()
{
	Vec2I isize(0, 0);
	if (mAtlasRegion)
	{
		isize = mAtlasRegion->GetSize();
	}
	else if (!mAtlasRegions.empty())
	{
		isize = mAtlasRegions[0]->GetSize();
	}
	else if (mTexture)
	{
		isize = mTexture->GetSize();
	}
	else
	{
		return;
	}
	ChangeSize(isize);
}

void ImageBox::SetDesaturate(bool desat)
{
	if (desat)
	{
		mUIObject->GetMaterial()->AddShaderDefine("_DESATURATE", "1");
	}
	else
	{
		mUIObject->GetMaterial()->RemoveShaderDefine("_DESATURATE");
	}
	//mUIObject->GetMaterial()->ApplyShaderDefines();
}

void ImageBox::SetAmbientColor(const Vec4& color)
{
	mUIObject->GetMaterial()->SetAmbientColor(color);
}

void ImageBox::SetSpecularColor(const Vec4& color)
{
	mUIObject->GetMaterial()->SetSpecularColor(color);
}

void ImageBox::SetAlphaTextureAutoGenerated(bool set){
	auto curMat = mUIObject->GetMaterial();
	if (set){		
		mUIObject->SetMaterial("EssentialEngineData/materials/UIImageBox_SeperatedAlpha.material");
		auto newMat = mUIObject->GetMaterial();
		if (newMat){		
			newMat->CopyMaterialParamFrom(curMat);
			newMat->CopyMaterialConstFrom(curMat);
			newMat->CopyTexturesFrom(curMat);
			const Vec2I& finalSize = GetFinalSize();
			bool callmeLater = false;
			auto texture = UIManager::GetInstance().GetBorderAlphaInfoTexture(finalSize, callmeLater);
			if (texture && newMat){
				newMat->SetTexture(texture, BINDING_SHADER_PS, 1);
			}

			auto& defines = curMat->GetShaderDefines();
			bool defineChanged = false;
			for (auto it : defines){
				if (it.name == "_DESATURATE"){
					defineChanged = newMat->AddShaderDefine("_DESATURATE", "1") || defineChanged;
				}
				else if (it.name == "_UV_ROT"){
					defineChanged = newMat->AddShaderDefine("_UV_ROT", "1") || defineChanged;

				}
			}
			/*if (defineChanged)
				newMat->ApplyShaderDefines();*/

			Vec2 texcoords[4] = {
				Vec2(0.f, 1.f),
				Vec2(0.f, 0.f),
				Vec2(1.f, 1.f),
				Vec2(1.f, 0.f)
			};
			mUIObject->SetTexCoord(texcoords, 4, 1);
		}
	}
	else{
		mUIObject->SetMaterial("EssentialEngineData/materials/UIImageBox.material");
		auto newMat = mUIObject->GetMaterial();
		if (newMat){
			newMat->CopyMaterialParamFrom(curMat);
			newMat->CopyMaterialConstFrom(curMat);
			newMat->CopyTexturesFrom(curMat);

			auto& defines = curMat->GetShaderDefines();
			for (auto it : defines){
				if (it.name == "_DESATURATE"){
					newMat->AddShaderDefine("_DESATURATE", "1");
				}
				else if (it.name == "_UV_ROT"){
					newMat->AddShaderDefine("_UV_ROT", "1");

				}
			}
			//newMat->ApplyShaderDefines();
			mUIObject->ClearTexCoord(1);
		}
	}
}


void ImageBox::SetBorderAlpha(bool use){
	SetAlphaTextureAutoGenerated(use);
}
}


