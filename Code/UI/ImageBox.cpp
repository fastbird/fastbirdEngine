#include <UI/StdAfx.h>
#include <UI/ImageBox.h>

namespace fastbird
{

ImageBox::ImageBox()
	: mTextureAtlas(0)
	, mAtlasRegion(0)
	, mUseHighlight(false)
{
	mUIObject = IUIObject::CreateUIObject(false);
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

void ImageBox::SetTexture(const char* file)
{
	mImageFile = file;
	ITexture* pTexture = gEnv->pRenderer->CreateTexture(file);
	SetTexture(pTexture);
}

void ImageBox::SetTexture(ITexture* pTexture)
{
	SAMPLER_DESC sd;
	sd.AddressU = TEXTURE_ADDRESS_BORDER;
	sd.AddressV = TEXTURE_ADDRESS_BORDER;
	sd.AddressW = TEXTURE_ADDRESS_BORDER;
	mUIObject->GetMaterial()->SetTexture(pTexture, BINDING_SHADER_PS, 0, sd);
	const RECT& r = mUIObject->GetRegion();
	float width = (float)(r.right - r.left);
	float height = (float)(r.bottom - r.top);
	float imgRatio = width / height;
	const RECT& uiRect = mUIObject->GetRegion();
	float uiRatio = (uiRect.right - uiRect.left) /
		(float)(uiRect.bottom - uiRect.top);
	if (uiRatio == imgRatio)
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
		float halfu = (imgRatio - uiRatio)* .5f;
		Vec2 texcoords[4] = {
			Vec2(0.5f - halfu, 1.f),
			Vec2(0.5f - halfu, 0.f),
			Vec2(0.5f + halfu, 1.f),
			Vec2(0.5f + halfu, 0.f)
		};
		mUIObject->SetTexCoord(texcoords, 4);
	}
}

void ImageBox::SetTextureAtlasRegion(const char* atlas, const char* region)
{
	mTextureAtlas = gEnv->pRenderer->GetTextureAtlas(atlas);
	if (mTextureAtlas)
	{
		mTexture = mTextureAtlas->mTexture->Clone();
		mAtlasRegion = mTextureAtlas->GetRegion(region);
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
	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

void ImageBox::Highlight(bool enable)
{
	if (!mUseHighlight)
		return;

	if (enable)
	{
		mUIObject->GetMaterial()->SetEmissiveColor(1, 1, 1, 1);
	}
	else
	{
		mUIObject->GetMaterial()->SetEmissiveColor(0, 0, 0, 0);
	}
}

void ImageBox::OnMouseHover(void* arg)
{
	SetCursor(WinBase::mCursorOver);
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
									 mImageFile = val;
									 return true;
	}
		break;

	case UIProperty::REGION:
	{
							   if (!mImageFile.empty())
							   {
								   SetTextureAtlasRegion(mImageFile.c_str(), val);
							   }
							   return true;
	}
		break;
	}

	return __super::SetProperty(prop, val);
}

}