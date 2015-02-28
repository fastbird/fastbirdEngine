#include <UI/StdAfx.h>
#include <UI/VerticalGauge.h>
#include <Engine/IMaterial.h>

namespace fastbird
{

	VerticalGauge::VerticalGauge()
		:mPercentage(0), mMaximum(1.f)
		, mBlink(false)
		, mBlinkSpeed(3.f)
		, mTextureAtlas(0)
		, mMaterialUsingImage(false)
		, mHorizontalFlip(false)
{
	for (int i = 0; i < 2; i++)
	{
		mAtlasRegions[i] = 0;
		mTextures[i] = 0;
	}

	mUIObject = IUIObject::CreateUIObject(false, GetRenderTargetSize());
	mUIObject->SetMaterial("es/Materials/UIVerticalGauge.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	IMaterial* mat = mUIObject->GetMaterial();
	mGaugeColor = Color(1, 1, 1, 1);
	mBlinkColor = Color(1, 0, 0, 1);
	mat->SetMaterialParameters(1, mGaugeColor.GetVec4());
	mat->SetMaterialParameters(2, mBlinkColor.GetVec4());
	// x is lerp.
	mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
	Vec2 texcoords[4] = {
		Vec2(0.f, 1.f),
		Vec2(0.f, 0.f),
		Vec2(1.f, 1.f),
		Vec2(1.f, 0.f)
	};
	mUIObject->SetTexCoord(texcoords, 4);
}

VerticalGauge::~VerticalGauge()
{
}

void VerticalGauge::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);
}
void VerticalGauge::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);
	if (mBlink)
	{
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(3, Vec4(sin(gEnv->pTimer->GetTime()*mBlinkSpeed)*.5f + .5f, 0, 0, 0));
	}
}

void VerticalGauge::SetPercentage(float p)
{
	mPercentage = p;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(0, Vec4(mPercentage, mMaximum, 0, 0));
}

void VerticalGauge::SetMaximum(float m)
{
	mMaximum = m;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(0, Vec4(mPercentage, m, 0, 0));
}

void VerticalGauge::Blink(bool blink)
{
	mBlink = blink;
	if (!blink)
	{
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
	}
}

void VerticalGauge::SetGaugeColor(const Color& color)
{
	mGaugeColor = color;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(1, color.GetVec4());
}

void VerticalGauge::SetBlinkColor(const Color& color)
{
	mBlinkColor = color;
	IMaterial* mat = mUIObject->GetMaterial();
	mat->SetMaterialParameters(2, color.GetVec4());
}

bool VerticalGauge::SetProperty(UIProperty::Enum prop, const char* val)
{

	switch (prop)
	{
	case UIProperty::GAUGE_COLOR:
	{
									SetGaugeColor(StringConverter::parseColor(val));
									return true;
	}

	case UIProperty::GAUGE_BLINK_COLOR:
	{
										  SetBlinkColor(StringConverter::parseColor(val));

										  return true;
	}

	case UIProperty::GAUGE_BLINK_SPEED:
	{
										  mBlinkSpeed = StringConverter::parseReal(val);
										  return true;
	}
	case UIProperty::GAUGE_MAX:
	{
								  SetMaximum(StringConverter::parseReal(val));
								  return true;
	}
	case UIProperty::GAUGE_CUR:
	{
								  SetPercentage(StringConverter::parseReal(val));
								  return true;
	}
		
	case UIProperty::TEXTUREATLAS:
	{
									 mTextureAtlasFile = val;
									 return true;
	}
		break;

	case UIProperty::REGION_FILLED:
	case UIProperty::REGION_NOT_FILLED:
	{
										  SetTextureAtlasRegion(prop, val);
										  return true;
	}
		break;

	case UIProperty::IMAGE_HFLIP:
	{
									mHorizontalFlip = StringConverter::parseBool(val);
									return true;
	}

	case UIProperty::GAUGE_BORDER_COLOR:
	{
										   Color gaugeBorderC = StringConverter::parseColor(val);
										   if (mUIObject)
										   {
											   auto mat = mUIObject->GetMaterial();
											   if (mat)
											   {
												   mat->SetAmbientColor(gaugeBorderC.GetVec4());
											   }
										   }
										   return true;
	}
	}

	return __super::SetProperty(prop, val);
}

void VerticalGauge::SetTextureAtlasRegion(UIProperty::Enum prop, const char* region)
{
	if (mTextureAtlasFile.empty())
	{
		mTextureAtlasFile = "data/textures/gameui.xml";
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

	mTextureAtlas = gEnv->pRenderer->GetTextureAtlas(mTextureAtlasFile.c_str());
	if (mTextureAtlas)
	{
		if (!mMaterialUsingImage)
		{
			mMaterialUsingImage = true;
			mUIObject->SetMaterial("es/Materials/UIVerticalGaugeImage.material");
			mUIObject->GetMaterial()->SetMaterialParameters(0, Vec4(mPercentage, mMaximum, 0, 0));
		}
		mTextures[index] = mTextureAtlas->mTexture->Clone();
		mAtlasRegions[index] = mTextureAtlas->GetRegion(region);
		if (!mAtlasRegions[index])
		{
			Error("Cannot find the region %s in the atlas %s", region, mTextureAtlasFile.c_str());
		}
		SAMPLER_DESC sdesc;
		sdesc.Filter = TEXTURE_FILTER_MIN_MAG_MIP_POINT;
		mUIObject->GetMaterial()->SetTexture(mTextures[index], BINDING_SHADER_PS, index, sdesc);
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
			mUIObject->GetMaterial()->SetMaterialParameters(1, Vec4(minY, maxY, 0, 0));
			DWORD colors[4] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
			mUIObject->SetColors(colors, 4);
		}
	}
}


}