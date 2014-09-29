#include <UI/StdAfx.h>
#include <UI/HorizontalGauge.h>
#include <Engine/IMaterial.h>

namespace fastbird
{

	HorizontalGauge::HorizontalGauge()
		:mPercentage(0), mMaximum(1.f)
		, mBlink(false)
		, mBlinkSpeed(3.f)
		, mGaugeColorEmptySet(false)
	{
		mUIObject = IUIObject::CreateUIObject(false);
		mUIObject->SetMaterial("es/Materials/UIHorizontalGauge.material");
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
		mat->SetMaterialParameters(4, Vec4(mPercentage, mMaximum, 0, 0));
	}

	HorizontalGauge::~HorizontalGauge()
	{
	}

	void HorizontalGauge::GatherVisit(std::vector<IUIObject*>& v)
	{
		v.push_back(mUIObject);
		__super::GatherVisit(v);
	}
	void HorizontalGauge::OnStartUpdate(float elapsedTime)
	{
		if (mBlink)
		{
			IMaterial* mat = mUIObject->GetMaterial();
			mat->SetMaterialParameters(3, Vec4(sin(gEnv->pTimer->GetTime()*mBlinkSpeed)*.5f + .5f, 0, 0, 0));
		}
	}

	void HorizontalGauge::SetPercentage(float p)
	{
		mPercentage = p;
		IMaterial* mat = mUIObject->GetMaterial();
		// x : ratio
		// y : percent
		// z : maximum
		Vec4 val = mat->GetMaterialParameters(4);
		val.x = p;
		val.y = mMaximum;
		mat->SetMaterialParameters(4, val);

		if (mGaugeColorEmptySet)
		{
			IMaterial* mat = mUIObject->GetMaterial();
			Color c= Lerp(mGaugeColorEmpty, mGaugeColor, p);
			mat->SetMaterialParameters(1, c.GetVec4());
		}
	}

	void HorizontalGauge::SetMaximum(float m)
	{
		mMaximum = m;
		IMaterial* mat = mUIObject->GetMaterial();
		
		Vec4 val = mat->GetMaterialParameters(4);
		val.y = mMaximum;
		mat->SetMaterialParameters(4, val);
	}

	void HorizontalGauge::Blink(bool blink)
	{
		if (mBlink == blink)
			return;

		mBlink = blink;
		if (!blink)
		{
			IMaterial* mat = mUIObject->GetMaterial();
			mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
		}
	}

	void HorizontalGauge::SetGaugeColor(const Color& color)
	{
		mGaugeColor = color;
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(1, color.GetVec4());
	}

	void HorizontalGauge::SetGaugeColorEmpty(const Color& color)
	{
		mGaugeColorEmpty = color;
		mGaugeColorEmptySet = true;
	}

	void HorizontalGauge::SetBlinkColor(const Color& color)
	{
		mBlinkColor = color;
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(2, color.GetVec4());
	}

	void HorizontalGauge::OnSizeChanged()
	{
		__super::OnSizeChanged();
		/*float screenratio = gEnv->pRenderer->GetWidth() / (float)gEnv->pRenderer->GetHeight();
		Vec4 ratio(mWNSize.x*screenratio / mWNSize.y, 0, 0, 0);
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(4, ratio);*/
	}

	bool HorizontalGauge::SetProperty(UIProperty::Enum prop, const char* val)
	{
		if (prop == UIProperty::GAUGE_COLOR)
		{
			SetGaugeColor(StringConverter::parseColor(val));
			return true;
		}

		if (prop == UIProperty::GAUGE_COLOR_EMPTY)
		{
			SetGaugeColorEmpty(StringConverter::parseColor(val));
			return true;
		}

		if (prop == UIProperty::GAUGE_BLINK_COLOR)
		{
			SetBlinkColor(StringConverter::parseColor(val));

			return true;
		}

		if (prop == UIProperty::GAUGE_BLINK_SPEED)
		{
			mBlinkSpeed = StringConverter::parseReal(val);
			return true;
		}

		return __super::SetProperty(prop, val);
	}


	
}