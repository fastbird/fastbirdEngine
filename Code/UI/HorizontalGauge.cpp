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
		, mBlinkTime(0)
	{
		mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
		mUIObject->SetMaterial("es/Materials/UIHorizontalGauge.material");
		mUIObject->mOwnerUI = this;
		mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

		IMaterial* mat = mUIObject->GetMaterial();
		mGaugeColor = Color(1, 1, 1, 1);
		mBlinkColor = Color(1, 1, 0, 1);
		mGaugeBorderColor = Color(0.5f, 0.5f, 0.5f, 0.5f);
		mat->SetMaterialParameters(1, mGaugeColor.GetVec4());
		mat->SetMaterialParameters(2, mBlinkColor.GetVec4());
		mat->SetAmbientColor(mGaugeBorderColor.GetVec4());

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
		__super::OnStartUpdate(elapsedTime);
		if (mBlink && mBlinkTime > 0)
		{
			IMaterial* mat = mUIObject->GetMaterial();
			mat->SetMaterialParameters(3, Vec4(sin(gFBEnv->pTimer->GetTime()*mBlinkSpeed)*.5f + .5f, 0, 0, 0));
			mBlinkTime -= elapsedTime;
			if (mBlinkTime <= 0){
				Blink(false);
				
			}
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
		if (!blink){
			IMaterial* mat = mUIObject->GetMaterial();
			mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
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
			IMaterial* mat = mUIObject->GetMaterial();
			mat->SetMaterialParameters(3, Vec4(0, 0, 0, 0));
			mBlinkTime = 0;
		}

	}

	void HorizontalGauge::SetGaugeColor(const Color& color)
	{
		mGaugeColor = color;
		IMaterial* mat = mUIObject->GetMaterial();
		mat->SetMaterialParameters(1, color.GetVec4());
	}

	const Color& HorizontalGauge::GetGaugeColor() const
	{
		return mGaugeColor;
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
									  SetMaximum(StringConverter::parseReal(val));
									  return true;
		}
		case UIProperty::GAUGE_CUR:
		{
									  SetPercentage(StringConverter::parseReal(val));
									  return true;
		}
		case UIProperty::GAUGE_COLOR:
		{
										SetGaugeColor(StringConverter::parseColor(val));
										return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
											  SetGaugeColorEmpty(StringConverter::parseColor(val));
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

		case UIProperty::GAUGE_BORDER_COLOR:
		{
											   mGaugeBorderColor = StringConverter::parseColor(val);
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
			auto data = StringConverter::toString(mMaximum);
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
			auto data = StringConverter::toString(mPercentage);
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
			auto data = StringConverter::toString(mGaugeColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		case UIProperty::GAUGE_COLOR_EMPTY:
		{
			if (mGaugeColorEmptySet)
			{
				auto data = StringConverter::toString(mGaugeColorEmpty);
				strcpy_s(val, bufsize, data.c_str());
				return true;
			}
			return false;
		}

		case UIProperty::GAUGE_BLINK_COLOR:
		{
			if (notDefaultOnly)
			{
				if (mBlinkColor == UIProperty::GetDefaultValueVec4(prop))
					return false;
			}
			auto data = StringConverter::toString(mBlinkColor);
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
			auto data = StringConverter::toString(mBlinkSpeed);
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
			auto data = StringConverter::toString(mGaugeBorderColor);
			strcpy_s(val, bufsize, data.c_str());
			return true;
		}
		}

		return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
	}


	
}