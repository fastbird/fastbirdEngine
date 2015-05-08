#include <UI/StdAfx.h>
#include <UI/ImageBox.h>
#include <UI/IUIManager.h>
namespace fastbird
{

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
		, mTexture(0)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->SetMaterial("es/Materials/UIImageBox.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER, 
		std::bind(&ImageBox::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_OUT, 
		std::bind(&ImageBox::OnMouseOut, this, std::placeholders::_1));
}

ImageBox::~ImageBox()
{
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
	const RECT& uiRect = mUIObject->GetRegion();
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
	if (mPlayingTime > mSecPerFrame)
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

void ImageBox::SetTexture(const char* file)
{
	if (!file)
		return;
	if (mImageFile == file)
		return;
	mImageFile = file ? file : "";
	if (mImageFile.empty())
	{
		mImageFile = "data/textures/empty_transparent.dds";
	}
	ITexture* pTexture = gFBEnv->pRenderer->CreateTexture(mImageFile.c_str());
	SetTexture(pTexture);
	gFBEnv->pUIManager->DirtyRenderList();
}

void ImageBox::SetTexture(ITexture* pTexture)
{
	mImageFile.clear();
	mTexture = pTexture;
	SAMPLER_DESC sd;
	sd.AddressU = TEXTURE_ADDRESS_BORDER;
	sd.AddressV = TEXTURE_ADDRESS_BORDER;
	sd.AddressW = TEXTURE_ADDRESS_BORDER;
	mUIObject->GetMaterial()->SetTexture(pTexture, BINDING_SHADER_PS, 0, sd);
	if (pTexture)
		CalcUV(pTexture->GetSize());
}

void ImageBox::SetTextureAtlasRegion(const char* atlas, const char* region)
{
	mTextureAtlas = gFBEnv->pRenderer->GetTextureAtlas(atlas);
	if (mTextureAtlas)
	{
		// need to set to material. matarial will hold its reference counter
		mTexture = mTextureAtlas->mTexture->Clone();
		if (mImageFixedSize)
		{
			DrawAsFixedSize();
		}
		mAtlasRegion = mTextureAtlas->GetRegion(region);
		if (!mAtlasRegion)
		{
			Error("Cannot find the region %s in the atlas %s", region, atlas);
		}
		SAMPLER_DESC sdesc;
		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		mUIObject->GetMaterial()->SetTexture(mTexture, 
			BINDING_SHADER_PS, 0, sdesc);
		if (mAtlasRegion)
		{
			Vec2 texcoords[4];
			mAtlasRegion->GetQuadUV(texcoords);
			mUIObject->SetTexCoord(texcoords, 4);
			DWORD colors[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
			mUIObject->SetColors(colors, 4);
		}
	}
}

void ImageBox::SetTextureAtlasRegions(const char* atlas, const std::vector<std::string>& data)
{
	mTextureAtlas = gFBEnv->pRenderer->GetTextureAtlas(atlas);
	if (mTextureAtlas)
	{
		// need to set to material. matarial will hold its reference counter
		mTexture = mTextureAtlas->mTexture->Clone();
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
		mUIObject->GetMaterial()->SetTexture(mTexture,
			BINDING_SHADER_PS, 0, sdesc);

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

void ImageBox::ChangeRegion(TextureAtlasRegion* region)
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
	assert(mTextureAtlas);
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

void ImageBox::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;

	if (!mTexture && mImageFile.empty())
		return;
	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

void ImageBox::OnSizeChanged()
{
	__super::OnSizeChanged();
	if (!mAtlasRegion && mAtlasRegions.empty())
	{
		auto texture = mUIObject->GetMaterial()->GetTexture(BINDING_SHADER_PS, 0);
		if (texture)
			CalcUV(texture->GetSize());
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

void ImageBox::OnMouseHover(void* arg)
{
}

void ImageBox::OnMouseOut(void* arg)
{
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
							   if (mTextureAtlasFile.empty())
							   {
								   mTextureAtlasFile = "data/textures/gameui.xml";
							   }
							   SetTextureAtlasRegion(mTextureAtlasFile.c_str(), val);
							   return true;
	}
		break;

	case UIProperty::REGIONS:
	{
								mAnimation = true;
								auto useNumberData = Split(val, ":");
								if (useNumberData.size() >= 2)
								{
									auto fromtoData = Split(useNumberData[1], ",");
									fromtoData[0] = StripBoth(fromtoData[0].c_str());
									fromtoData[1] = StripBoth(fromtoData[1].c_str());
									unsigned from = StringConverter::parseUnsignedInt(fromtoData[0].c_str());
									unsigned to = StringConverter::parseUnsignedInt(fromtoData[1].c_str());
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
							mSecPerFrame = 1.0f / StringConverter::parseReal(val);
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
										 SetKeepImageRatio(StringConverter::parseBool(val, true));
										 return true;
	}
	case UIProperty::FRAME_IMAGE:
	{
									if (!mFrameImage)
									{
										mFrameImage = CreateImageBox();
									}
									if_assert_pass(!mTextureAtlasFile.empty())
									{
										mFrameImage->SetTextureAtlasRegion(mTextureAtlasFile.c_str(), val);
										if (strlen(val) == 0)
										{
											mFrameImage->SetVisible(false);
										}
										else
										{
											mFrameImage->SetVisible(true);
										}
									}
									return true;
	}

	case UIProperty::IMAGE_COLOR_OVERLAY:
	{
											Color color = Color(val);
											if (mUIObject)
											{
												mUIObject->GetMaterial()->SetDiffuseColor(color.GetVec4());
											}
											return true;

	}

	case UIProperty::IMAGE_FIXED_SIZE:
	{
										 mImageFixedSize = StringConverter::parseBool(val);										 
										 if (mTexture || mAtlasRegion)
										 {
											 DrawAsFixedSize();
										 }
										 
										 return true;

	}

	}

	return __super::SetProperty(prop, val);
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

ImageBox* ImageBox::CreateImageBox()
{
	auto image = (ImageBox*)AddChild(0, 0, 1.0f, 1.0f, ComponentType::ImageBox);
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
		mat->ApplyShaderDefines();
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
			mat->SetMaterialParameters(0, Vec4(center.x, center.y, 0, 0));
		}

	}
	else
	{
		assert(0 && "Not Implemented");
	}
}

void ImageBox::DrawAsFixedSizeCenteredAt(const Vec2& wnpos)
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
		assert(0 && "You didn't set the texture");
		return;
	}
	Vec2 size = Vec2(isize) / Vec2(GetRenderTargetSize());
	SetWNSize(size);
	SetWNPos(wnpos);
	SetAlign(ALIGNH::CENTER, ALIGNV::MIDDLE);
	
}

void ImageBox::DrawAsFixedSizeAtCenter()
{
	DrawAsFixedSize();
	SetNPos(Vec2(0.5f, 0.5f));
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
	Vec2 size = Vec2(isize) / Vec2(GetRenderTargetSize());
	SetWNSize(size);
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
	mUIObject->GetMaterial()->ApplyShaderDefines();
}

void ImageBox::SetAmbientColor(const Vec4& color)
{
	mUIObject->GetMaterial()->SetAmbientColor(color);
}

void ImageBox::SetSpecularColor(const Vec4& color)
{
	mUIObject->GetMaterial()->SetSpecularColor(color);
}



}
